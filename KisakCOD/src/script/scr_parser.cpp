#include "scr_parser.h"
#include <qcommon/mem_track.h>
#include <universal/assertive.h>
#include "scr_main.h"
#include <universal/com_memory.h>


scrParserGlob_t scrParserGlob;
scrParserPub_t scrParserPub;

void __cdecl TRACK_scr_parser()
{
    track_static_alloc_internal(&scrParserGlob, 52, "scrParserGlob", 7);
    track_static_alloc_internal(&scrParserPub, 16, "scrParserPub", 7);
}

void __cdecl Scr_InitOpcodeLookup()
{
    if (scrParserGlob.opcodeLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 75, 0, "%s", "!scrParserGlob.opcodeLookup");
    if (scrParserGlob.sourcePosLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 76, 0, "%s", "!scrParserGlob.sourcePosLookup");
    if (scrParserPub.sourceBufferLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 77, 0, "%s", "!scrParserPub.sourceBufferLookup");
    if (scrVarPub.developer)
    {
        scrParserGlob.delayedSourceIndex = -1;
        scrParserGlob.opcodeLookupMaxLen = 0x40000;
        scrParserGlob.opcodeLookupLen = 0;
        scrParserGlob.opcodeLookup = (OpcodeLookup *)Hunk_AllocDebugMem(6291456, "Scr_InitOpcodeLookup");
        memset((unsigned __int8 *)scrParserGlob.opcodeLookup, 0, 24 * scrParserGlob.opcodeLookupMaxLen);
        scrParserGlob.sourcePosLookupMaxLen = 0x40000;
        scrParserGlob.sourcePosLookupLen = 0;
        scrParserGlob.sourcePosLookup = (SourceLookup *)Hunk_AllocDebugMem(0x200000, "Scr_InitOpcodeLookup");
        scrParserGlob.currentCodePos = 0;
        scrParserGlob.currentSourcePosCount = 0;
        scrParserGlob.sourceBufferLookupMaxLen = 256;
        scrParserPub.sourceBufferLookupLen = 0;
        scrParserPub.sourceBufferLookup = (SourceBufferInfo *)Hunk_AllocDebugMem(11264, "Scr_InitOpcodeLookup");
    }
}

void __cdecl Scr_ShutdownOpcodeLookup()
{
    unsigned int i; // [esp+0h] [ebp-4h]
    unsigned int ia; // [esp+0h] [ebp-4h]

    if (scrParserGlob.opcodeLookup)
    {
        Hunk_FreeDebugMem(scrParserGlob.opcodeLookup);
        scrParserGlob.opcodeLookup = 0;
    }
    if (scrParserGlob.sourcePosLookup)
    {
        Hunk_FreeDebugMem(scrParserGlob.sourcePosLookup);
        scrParserGlob.sourcePosLookup = 0;
    }
    if (scrParserPub.sourceBufferLookup)
    {
        for (i = 0; i < scrParserPub.sourceBufferLookupLen; ++i)
            Hunk_FreeDebugMem(scrParserPub.sourceBufferLookup[i].buf);
        Hunk_FreeDebugMem(scrParserPub.sourceBufferLookup);
        scrParserPub.sourceBufferLookup = 0;
    }
    if (scrParserGlob.saveSourceBufferLookup)
    {
        for (ia = 0; ia < scrParserGlob.saveSourceBufferLookupLen; ++ia)
        {
            if (scrParserGlob.saveSourceBufferLookup[ia].sourceBuf)
                Hunk_FreeDebugMem(scrParserGlob.saveSourceBufferLookup[ia].sourceBuf);
        }
        Hunk_FreeDebugMem(scrParserGlob.saveSourceBufferLookup);
        scrParserGlob.saveSourceBufferLookup = 0;
    }
}

void __cdecl AddOpcodePos(unsigned int sourcePos, int type)
{
    unsigned int sourcePosLookupIndex; // [esp+0h] [ebp-14h]
    OpcodeLookup *opcodeLookup; // [esp+4h] [ebp-10h]
    SourceLookup *sourcePosLookup; // [esp+8h] [ebp-Ch]
    unsigned __int8 *newSourcePosLookup; // [esp+Ch] [ebp-8h]
    unsigned __int8 *newOpcodeLookup; // [esp+10h] [ebp-4h]

    if (scrVarPub.developer)
    {
        if (scrCompilePub.developer_statement == 2)
        {
            if (scrVarPub.developer_script)
                MyAssertHandler(".\\script\\scr_parser.cpp", 152, 0, "%s", "!scrVarPub.developer_script");
        }
        else if (scrCompilePub.developer_statement != 3)
        {
            if (!scrCompilePub.allowedBreakpoint)
                type &= ~1u;
            if (!scrParserGlob.opcodeLookup)
                MyAssertHandler(".\\script\\scr_parser.cpp", 164, 0, "%s", "scrParserGlob.opcodeLookup");
            if (!scrParserGlob.opcodeLookupMaxLen)
                MyAssertHandler(".\\script\\scr_parser.cpp", 165, 0, "%s", "scrParserGlob.opcodeLookupMaxLen");
            if (!scrParserGlob.sourcePosLookup)
                MyAssertHandler(".\\script\\scr_parser.cpp", 166, 0, "%s", "scrParserGlob.sourcePosLookup");
            if (!scrCompilePub.opcodePos)
                MyAssertHandler(".\\script\\scr_parser.cpp", 167, 0, "%s", "scrCompilePub.opcodePos");
            if (scrParserGlob.opcodeLookupLen >= scrParserGlob.opcodeLookupMaxLen)
            {
                scrParserGlob.opcodeLookupMaxLen *= 2;
                if (scrParserGlob.opcodeLookupLen >= scrParserGlob.opcodeLookupMaxLen)
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        172,
                        0,
                        "%s",
                        "scrParserGlob.opcodeLookupLen < scrParserGlob.opcodeLookupMaxLen");
                newOpcodeLookup = (unsigned __int8 *)Hunk_AllocDebugMem(24 * scrParserGlob.opcodeLookupMaxLen);
                memcpy(newOpcodeLookup, (unsigned __int8 *)scrParserGlob.opcodeLookup, 24 * scrParserGlob.opcodeLookupLen);
                Hunk_FreeDebugMem();
                scrParserGlob.opcodeLookup = (OpcodeLookup *)newOpcodeLookup;
            }
            if (scrParserGlob.sourcePosLookupLen >= scrParserGlob.sourcePosLookupMaxLen)
            {
                scrParserGlob.sourcePosLookupMaxLen *= 2;
                if (scrParserGlob.sourcePosLookupLen >= scrParserGlob.sourcePosLookupMaxLen)
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        182,
                        0,
                        "%s",
                        "scrParserGlob.sourcePosLookupLen < scrParserGlob.sourcePosLookupMaxLen");
                newSourcePosLookup = (unsigned __int8 *)Hunk_AllocDebugMem(8 * scrParserGlob.sourcePosLookupMaxLen);
                memcpy(
                    newSourcePosLookup,
                    (unsigned __int8 *)scrParserGlob.sourcePosLookup,
                    8 * scrParserGlob.sourcePosLookupLen);
                Hunk_FreeDebugMem();
                scrParserGlob.sourcePosLookup = (SourceLookup *)newSourcePosLookup;
            }
            if (scrParserGlob.currentCodePos == scrCompilePub.opcodePos)
            {
                if (!scrParserGlob.currentSourcePosCount)
                    MyAssertHandler(".\\script\\scr_parser.cpp", 191, 0, "%s", "scrParserGlob.currentSourcePosCount");
                opcodeLookup = &scrParserGlob.opcodeLookup[--scrParserGlob.opcodeLookupLen];
                if (scrParserGlob.currentSourcePosCount + opcodeLookup->sourcePosIndex != scrParserGlob.sourcePosLookupLen)
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        197,
                        0,
                        "%s",
                        "opcodeLookup->sourcePosIndex + scrParserGlob.currentSourcePosCount == scrParserGlob.sourcePosLookupLen");
                if (opcodeLookup->codePos != (const char *)scrParserGlob.currentCodePos)
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        198,
                        0,
                        "%s",
                        "opcodeLookup->codePos == (char *)scrParserGlob.currentCodePos");
            }
            else
            {
                scrParserGlob.currentSourcePosCount = 0;
                scrParserGlob.currentCodePos = scrCompilePub.opcodePos;
                opcodeLookup = &scrParserGlob.opcodeLookup[scrParserGlob.opcodeLookupLen];
                opcodeLookup->sourcePosIndex = scrParserGlob.sourcePosLookupLen;
                opcodeLookup->codePos = (const char *)scrParserGlob.currentCodePos;
            }
            sourcePosLookupIndex = scrParserGlob.currentSourcePosCount + opcodeLookup->sourcePosIndex;
            sourcePosLookup = &scrParserGlob.sourcePosLookup[sourcePosLookupIndex];
            sourcePosLookup->sourcePos = sourcePos;
            if (sourcePos == -1)
            {
                if (scrParserGlob.delayedSourceIndex != -1)
                    MyAssertHandler(".\\script\\scr_parser.cpp", 217, 0, "%s", "scrParserGlob.delayedSourceIndex == -1");
                if ((type & 1) == 0)
                    MyAssertHandler(".\\script\\scr_parser.cpp", 218, 0, "%s", "type & SOURCE_TYPE_BREAKPOINT");
                scrParserGlob.delayedSourceIndex = sourcePosLookupIndex;
            }
            else if (sourcePos == -2)
            {
                scrParserGlob.threadStartSourceIndex = sourcePosLookupIndex;
            }
            else if (scrParserGlob.delayedSourceIndex >= 0 && (type & 1) != 0)
            {
                scrParserGlob.sourcePosLookup[scrParserGlob.delayedSourceIndex].sourcePos = sourcePos;
                scrParserGlob.delayedSourceIndex = -1;
            }
            sourcePosLookup->type |= type;
            opcodeLookup->sourcePosCount = ++scrParserGlob.currentSourcePosCount;
            ++scrParserGlob.opcodeLookupLen;
            ++scrParserGlob.sourcePosLookupLen;
        }
    }
}

void __cdecl RemoveOpcodePos()
{
    OpcodeLookup *opcodeLookup; // [esp+0h] [ebp-4h]

    if (scrVarPub.developer)
    {
        if (scrCompilePub.developer_statement == 2)
        {
            if (scrVarPub.developer_script)
                MyAssertHandler(".\\script\\scr_parser.cpp", 252, 0, "%s", "!scrVarPub.developer_script");
        }
        else
        {
            if (!scrParserGlob.opcodeLookup)
                MyAssertHandler(".\\script\\scr_parser.cpp", 256, 0, "%s", "scrParserGlob.opcodeLookup");
            if (!scrParserGlob.opcodeLookupMaxLen)
                MyAssertHandler(".\\script\\scr_parser.cpp", 257, 0, "%s", "scrParserGlob.opcodeLookupMaxLen");
            if (!scrParserGlob.sourcePosLookup)
                MyAssertHandler(".\\script\\scr_parser.cpp", 258, 0, "%s", "scrParserGlob.sourcePosLookup");
            if (!scrCompilePub.opcodePos)
                MyAssertHandler(".\\script\\scr_parser.cpp", 259, 0, "%s", "scrCompilePub.opcodePos");
            if (!scrParserGlob.sourcePosLookupLen)
                MyAssertHandler(".\\script\\scr_parser.cpp", 261, 0, "%s", "scrParserGlob.sourcePosLookupLen");
            --scrParserGlob.sourcePosLookupLen;
            if (!scrParserGlob.opcodeLookupLen)
                MyAssertHandler(".\\script\\scr_parser.cpp", 264, 0, "%s", "scrParserGlob.opcodeLookupLen");
            --scrParserGlob.opcodeLookupLen;
            if (!scrParserGlob.currentSourcePosCount)
                MyAssertHandler(".\\script\\scr_parser.cpp", 267, 0, "%s", "scrParserGlob.currentSourcePosCount");
            --scrParserGlob.currentSourcePosCount;
            opcodeLookup = &scrParserGlob.opcodeLookup[scrParserGlob.opcodeLookupLen];
            if (scrParserGlob.currentCodePos != scrCompilePub.opcodePos)
                MyAssertHandler(
                    ".\\script\\scr_parser.cpp",
                    272,
                    0,
                    "%s",
                    "scrParserGlob.currentCodePos == scrCompilePub.opcodePos");
            if (scrParserGlob.currentSourcePosCount + opcodeLookup->sourcePosIndex != scrParserGlob.sourcePosLookupLen)
                MyAssertHandler(
                    ".\\script\\scr_parser.cpp",
                    273,
                    0,
                    "%s",
                    "opcodeLookup->sourcePosIndex + scrParserGlob.currentSourcePosCount == scrParserGlob.sourcePosLookupLen");
            if (opcodeLookup->codePos != (const char *)scrParserGlob.currentCodePos)
                MyAssertHandler(
                    ".\\script\\scr_parser.cpp",
                    274,
                    0,
                    "%s",
                    "opcodeLookup->codePos == (char *)scrParserGlob.currentCodePos");
            if (!scrParserGlob.currentSourcePosCount)
                scrParserGlob.currentCodePos = 0;
            opcodeLookup->sourcePosCount = scrParserGlob.currentSourcePosCount;
        }
    }
}

void __cdecl AddThreadStartOpcodePos(unsigned int sourcePos)
{
    SourceLookup *sourcePosLookup; // [esp+0h] [ebp-4h]

    if (scrVarPub.developer)
    {
        if (scrCompilePub.developer_statement == 2)
        {
            if (scrVarPub.developer_script)
                MyAssertHandler(".\\script\\scr_parser.cpp", 292, 0, "%s", "!scrVarPub.developer_script");
        }
        else
        {
            if (scrParserGlob.threadStartSourceIndex < 0)
                MyAssertHandler(".\\script\\scr_parser.cpp", 296, 0, "%s", "scrParserGlob.threadStartSourceIndex >= 0");
            sourcePosLookup = &scrParserGlob.sourcePosLookup[scrParserGlob.threadStartSourceIndex];
            sourcePosLookup->sourcePos = sourcePos;
            if (sourcePosLookup->type)
                MyAssertHandler(".\\script\\scr_parser.cpp", 299, 0, "%s", "!sourcePosLookup->type");
            sourcePosLookup->type = 4;
            scrParserGlob.threadStartSourceIndex = -1;
        }
    }
}

const char *__cdecl Scr_GetOpcodePosOfType(
    unsigned int bufferIndex,
    unsigned int startSourcePos,
    unsigned int endSourcePos,
    int type,
    unsigned int *sourcePos)
{
    const char *codePos; // [esp+0h] [ebp-34h]
    SourceBufferInfo *v7; // [esp+4h] [ebp-30h]
    unsigned int j; // [esp+8h] [ebp-2Ch]
    signed int sourcePosCount; // [esp+10h] [ebp-24h]
    unsigned int firstSourcePos; // [esp+14h] [ebp-20h]
    int k; // [esp+18h] [ebp-1Ch]
    SourceLookup *sourcePosLookup; // [esp+1Ch] [ebp-18h]
    SourceBufferInfo *sourceBufData; // [esp+20h] [ebp-14h]
    const char *startBufferCodePos; // [esp+24h] [ebp-10h]
    const char *firstOpcodePos; // [esp+28h] [ebp-Ch]
    const char *opcodePos; // [esp+2Ch] [ebp-8h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_parser.cpp", 320, 0, "%s", "!Sys_IsRemoteDebugClient()");
    sourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex];
    if (bufferIndex + 1 >= scrParserPub.sourceBufferLookupLen)
        v7 = 0;
    else
        v7 = &scrParserPub.sourceBufferLookup[bufferIndex + 1];
    startBufferCodePos = sourceBufData->codePos;
    if (sourceBufData->codePos)
    {
        firstOpcodePos = 0;
        firstSourcePos = 0;
        if (v7)
            codePos = v7->codePos;
        else
            codePos = 0;
        for (j = 0; j < scrParserGlob.opcodeLookupLen; ++j)
        {
            sourcePosCount = scrParserGlob.opcodeLookup[j].sourcePosCount;
            for (k = 0; k < sourcePosCount; ++k)
            {
                opcodePos = scrParserGlob.opcodeLookup[j].codePos;
                if (opcodePos >= startBufferCodePos)
                {
                    if (opcodePos <= codePos - 1)
                    {
                        sourcePosLookup = &scrParserGlob.sourcePosLookup[k + scrParserGlob.opcodeLookup[j].sourcePosIndex];
                        if ((type & sourcePosLookup->type) == type
                            && sourcePosLookup->sourcePos >= startSourcePos
                            && sourcePosLookup->sourcePos < endSourcePos
                            && (!firstOpcodePos || scrParserGlob.opcodeLookup[j].codePos < firstOpcodePos))
                        {
                            firstOpcodePos = scrParserGlob.opcodeLookup[j].codePos;
                            firstSourcePos = sourcePosLookup->sourcePos;
                        }
                    }
                    else if (!v7)
                    {
                        MyAssertHandler(".\\script\\scr_parser.cpp", 347, 0, "%s", "nextSourceBufData");
                    }
                }
            }
        }
        *sourcePos = firstSourcePos;
        return firstOpcodePos;
    }
    else
    {
        *sourcePos = 0;
        return 0;
    }
}

unsigned int __cdecl Scr_GetClosestSourcePosOfType(unsigned int bufferIndex, unsigned int sourcePos, int type)
{
    const char *codePos; // [esp+0h] [ebp-30h]
    SourceBufferInfo *v5; // [esp+4h] [ebp-2Ch]
    unsigned int j; // [esp+8h] [ebp-28h]
    signed int sourcePosCount; // [esp+10h] [ebp-20h]
    int k; // [esp+14h] [ebp-1Ch]
    SourceLookup *sourcePosLookup; // [esp+18h] [ebp-18h]
    SourceBufferInfo *sourceBufData; // [esp+1Ch] [ebp-14h]
    const char *startBufferCodePos; // [esp+20h] [ebp-10h]
    unsigned int bestSourcePos; // [esp+24h] [ebp-Ch]
    const char *opcodePos; // [esp+28h] [ebp-8h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_parser.cpp", 385, 0, "%s", "!Sys_IsRemoteDebugClient()");
    bestSourcePos = 0;
    sourceBufData = &scrParserPub.sourceBufferLookup[bufferIndex];
    if (bufferIndex + 1 >= scrParserPub.sourceBufferLookupLen)
        v5 = 0;
    else
        v5 = &scrParserPub.sourceBufferLookup[bufferIndex + 1];
    startBufferCodePos = sourceBufData->codePos;
    if (!sourceBufData->codePos)
        return 0;
    if (v5)
        codePos = v5->codePos;
    else
        codePos = 0;
    for (j = 0; j < scrParserGlob.opcodeLookupLen; ++j)
    {
        sourcePosCount = scrParserGlob.opcodeLookup[j].sourcePosCount;
        for (k = 0; k < sourcePosCount; ++k)
        {
            opcodePos = scrParserGlob.opcodeLookup[j].codePos;
            if (opcodePos >= startBufferCodePos)
            {
                if (opcodePos <= codePos - 1)
                {
                    sourcePosLookup = &scrParserGlob.sourcePosLookup[k + scrParserGlob.opcodeLookup[j].sourcePosIndex];
                    if ((type & sourcePosLookup->type) == type
                        && sourcePosLookup->sourcePos >= bestSourcePos
                        && sourcePosLookup->sourcePos <= sourcePos)
                    {
                        bestSourcePos = sourcePosLookup->sourcePos;
                    }
                }
                else if (!v5)
                {
                    MyAssertHandler(".\\script\\scr_parser.cpp", 407, 0, "%s", "nextSourceBufData");
                }
            }
        }
    }
    return bestSourcePos;
}

unsigned int __cdecl Scr_GetPrevSourcePos(const char *codePos, unsigned int index)
{
    return scrParserGlob.sourcePosLookup[index + Scr_GetPrevSourcePosOpcodeLookup(codePos)->sourcePosIndex].sourcePos;
}

OpcodeLookup *__cdecl Scr_GetPrevSourcePosOpcodeLookup(const char *codePos)
{
    unsigned int low; // [esp+0h] [ebp-Ch]
    unsigned int middle; // [esp+4h] [ebp-8h]
    unsigned int high; // [esp+8h] [ebp-4h]

    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 430, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    if (!scrParserGlob.opcodeLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 431, 0, "%s", "scrParserGlob.opcodeLookup");
    low = 0;
    high = scrParserGlob.opcodeLookupLen - 1;
    while (low <= high)
    {
        middle = (high + low) >> 1;
        if (codePos < scrParserGlob.opcodeLookup[middle].codePos)
        {
            high = middle - 1;
        }
        else
        {
            low = middle + 1;
            if (middle + 1 == scrParserGlob.opcodeLookupLen || codePos < scrParserGlob.opcodeLookup[low].codePos)
                return &scrParserGlob.opcodeLookup[middle];
        }
    }
    if (!alwaysfails)
        MyAssertHandler(".\\script\\scr_parser.cpp", 450, 0, "unreachable");
    return 0;
}

unsigned int __cdecl Scr_GetLineNum(unsigned int bufferIndex, unsigned int sourcePos)
{
    const char *startLine; // [esp+0h] [ebp-8h] BYREF
    int col; // [esp+4h] [ebp-4h] BYREF

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 557, 0, "%s", "scrVarPub.developer");
    return Scr_GetLineNumInternal(scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, sourcePos, &startLine, &col);
}

unsigned int __cdecl Scr_GetLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine, int *col)
{
    unsigned int lineNum; // [esp+0h] [ebp-4h]

    if (!buf)
        MyAssertHandler(".\\script\\scr_parser.cpp", 530, 0, "%s", "buf");
    *startLine = buf;
    lineNum = 0;
    while (sourcePos)
    {
        if (!*buf)
        {
            *startLine = buf + 1;
            ++lineNum;
        }
        ++buf;
        --sourcePos;
    }
    *col = buf - *startLine;
    return lineNum;
}

unsigned int __cdecl Scr_GetFunctionLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine)
{
    unsigned int lineNum; // [esp+0h] [ebp-Ch]
    unsigned int functionLine; // [esp+4h] [ebp-8h]
    const char *functionStartLine; // [esp+8h] [ebp-4h]

    if (!buf)
        MyAssertHandler(".\\script\\scr_parser.cpp", 497, 0, "%s", "buf");
    *startLine = buf;
    lineNum = 0;
    functionLine = 0;
    functionStartLine = buf;
    while (sourcePos)
    {
        if (!*buf)
        {
            if (buf[1] == 123)
            {
                functionLine = lineNum;
                functionStartLine = *startLine;
            }
            *startLine = buf + 1;
            ++lineNum;
        }
        ++buf;
        --sourcePos;
    }
    *startLine = functionStartLine;
    return functionLine;
}

int __cdecl Scr_GetSourcePosOfType(const char *codePos, int type, Scr_SourcePos_t *pos)
{
    OpcodeLookup *SourcePosOpcodeLookup; // [esp+0h] [ebp-Ch]
    unsigned int index; // [esp+8h] [ebp-4h]

    if (type)
        SourcePosOpcodeLookup = Scr_GetSourcePosOpcodeLookup(codePos);
    else
        SourcePosOpcodeLookup = Scr_GetPrevSourcePosOpcodeLookup(codePos);
    if (SourcePosOpcodeLookup)
    {
        for (index = 0; index < SourcePosOpcodeLookup->sourcePosCount; ++index)
        {
            if ((type & scrParserGlob.sourcePosLookup[index + SourcePosOpcodeLookup->sourcePosIndex].type) == type)
            {
                pos->sourcePos = scrParserGlob.sourcePosLookup[index + SourcePosOpcodeLookup->sourcePosIndex].sourcePos;
                pos->bufferIndex = Scr_GetSourceBuffer(codePos);
                pos->lineNum = Scr_GetLineNum(pos->bufferIndex, pos->sourcePos);
                return 1;
            }
        }
    }
    pos->sourcePos = 0;
    pos->bufferIndex = 0;
    pos->lineNum = 0;
    return 0;
}

OpcodeLookup *__cdecl Scr_GetSourcePosOpcodeLookup(const char *codePos)
{
    int low; // [esp+0h] [ebp-Ch]
    int middle; // [esp+4h] [ebp-8h]
    int high; // [esp+8h] [ebp-4h]

    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 461, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    if (!scrParserGlob.opcodeLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 462, 0, "%s", "scrParserGlob.opcodeLookup");
    low = 0;
    high = scrParserGlob.opcodeLookupLen - 1;
    while (low <= high)
    {
        middle = (high + low) / 2;
        if (codePos < scrParserGlob.opcodeLookup[middle].codePos)
        {
            high = middle - 1;
        }
        else
        {
            if (codePos == scrParserGlob.opcodeLookup[middle].codePos)
                return &scrParserGlob.opcodeLookup[middle];
            low = middle + 1;
        }
    }
    return 0;
}

void __cdecl Scr_AddSourceBufferInternal(
    const char *extFilename,
    const char *codePos,
    char *sourceBuf,
    int len,
    bool doEolFixup,
    bool archive)
{
    SourceBufferInfo *NewSourceBuffer; // eax
    const char *v7; // [esp+0h] [ebp-48h]
    char v8; // [esp+7h] [ebp-41h]
    char *v9; // [esp+Ch] [ebp-3Ch]
    const char *v10; // [esp+10h] [ebp-38h]
    unsigned int v11; // [esp+14h] [ebp-34h]
    const char *source; // [esp+2Ch] [ebp-1Ch]
    char c; // [esp+33h] [ebp-15h]
    char ca; // [esp+33h] [ebp-15h]
    char *buf; // [esp+34h] [ebp-14h]
    char *dest; // [esp+40h] [ebp-8h]
    int i; // [esp+44h] [ebp-4h]
    int ia; // [esp+44h] [ebp-4h]

    if (scrParserPub.sourceBufferLookup)
    {
        v11 = strlen(extFilename);
        buf = (char *)Hunk_AllocDebugMem(v11 + 1 + len + 2);
        v10 = extFilename;
        v9 = buf;
        do
        {
            v8 = *v10;
            *v9++ = *v10++;
        } while (v8);
        if (sourceBuf)
            v7 = &buf[v11 + 1];
        else
            v7 = 0;
        source = sourceBuf;
        dest = (char *)v7;
        if (doEolFixup)
        {
            for (i = 0; i <= len; ++i)
            {
                c = *source++;
                if (c == 10 || c == 13 && *source != 10)
                    *dest = 0;
                else
                    *dest = c;
                ++dest;
            }
        }
        else
        {
            for (ia = 0; ia <= len; ++ia)
            {
                ca = *source++;
                *dest++ = ca;
            }
        }
        NewSourceBuffer = Scr_GetNewSourceBuffer();
        NewSourceBuffer->codePos = codePos;
        NewSourceBuffer->buf = buf;
        NewSourceBuffer->sourceBuf = v7;
        NewSourceBuffer->len = len;
        NewSourceBuffer->sortedIndex = -1;
        NewSourceBuffer->archive = archive;
        if (v7)
            scrParserPub.sourceBuf = v7;
    }
    else
    {
        scrParserPub.sourceBuf = 0;
    }
}

SourceBufferInfo *__cdecl Scr_GetNewSourceBuffer()
{
    char *newSourceBufferInfo; // [esp+4h] [ebp-4h]

    if (!scrParserPub.sourceBufferLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 611, 0, "%s", "scrParserPub.sourceBufferLookup");
    if (!scrParserGlob.sourceBufferLookupMaxLen)
        MyAssertHandler(".\\script\\scr_parser.cpp", 612, 0, "%s", "scrParserGlob.sourceBufferLookupMaxLen");
    if (scrParserPub.sourceBufferLookupLen >= scrParserGlob.sourceBufferLookupMaxLen)
    {
        scrParserGlob.sourceBufferLookupMaxLen *= 2;
        if (scrParserPub.sourceBufferLookupLen >= scrParserGlob.sourceBufferLookupMaxLen)
            MyAssertHandler(
                ".\\script\\scr_parser.cpp",
                617,
                0,
                "%s",
                "scrParserPub.sourceBufferLookupLen < scrParserGlob.sourceBufferLookupMaxLen");
        newSourceBufferInfo = (char *)Hunk_AllocDebugMem(44 * scrParserGlob.sourceBufferLookupMaxLen);
        Com_Memcpy(newSourceBufferInfo, (char *)scrParserPub.sourceBufferLookup, 44 * scrParserPub.sourceBufferLookupLen);
        Hunk_FreeDebugMem();
        scrParserPub.sourceBufferLookup = (SourceBufferInfo *)newSourceBufferInfo;
    }
    return &scrParserPub.sourceBufferLookup[scrParserPub.sourceBufferLookupLen++];
}

char *__cdecl Scr_AddSourceBuffer(const char *filename, char *extFilename, const char *codePos, bool archive)
{
    char v5; // [esp+3h] [ebp-1Dh]
    const char *source; // [esp+4h] [ebp-1Ch]
    char c; // [esp+Bh] [ebp-15h]
    SaveSourceBufferInfo *saveSourceBuffer; // [esp+Ch] [ebp-14h]
    int len; // [esp+10h] [ebp-10h]
    char *dest; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    char *sourceBuf; // [esp+1Ch] [ebp-4h]

    if (!archive || !scrParserGlob.saveSourceBufferLookup)
        return Scr_ReadFile(filename, extFilename, codePos, archive);
    if (!scrParserGlob.saveSourceBufferLookupLen)
        MyAssertHandler(".\\script\\scr_parser.cpp", 861, 0, "%s", "scrParserGlob.saveSourceBufferLookupLen > 0");
    saveSourceBuffer = &scrParserGlob.saveSourceBufferLookup[--scrParserGlob.saveSourceBufferLookupLen];
    len = saveSourceBuffer->len;
    if (len < -1)
        MyAssertHandler(".\\script\\scr_parser.cpp", 865, 0, "%s", "len >= -1");
    if (len >= 0)
    {
        sourceBuf = (char *)Hunk_AllocateTempMemoryHigh(len + 1, "Scr_AddSourceBuffer1");
        source = saveSourceBuffer->sourceBuf;
        dest = sourceBuf;
        for (i = 0; i < len; ++i)
        {
            c = *source++;
            if (c)
                v5 = c;
            else
                v5 = 10;
            *dest++ = v5;
        }
        *dest = 0;
        if (saveSourceBuffer->sourceBuf)
            Hunk_FreeDebugMem();
        Scr_AddSourceBufferInternal(extFilename, codePos, sourceBuf, len, 1, archive);
    }
    else
    {
        sourceBuf = 0;
        Scr_AddSourceBufferInternal(extFilename, codePos, 0, len, 1, archive);
    }
    return sourceBuf;
}

char *__cdecl Scr_ReadFile(const char *filename, char *extFilename, const char *codePos, bool archive)
{
    int file; // [esp+24h] [ebp-4h] BYREF

    if (*(_BYTE *)fs_gameDirVar->current.integer)
    {
        if ((FS_FOpenFileRead(extFilename, &file) & 0x80000000) != 0)
        {
            return Scr_ReadFile_FastFile(filename, extFilename, codePos, archive);
        }
        else
        {
            FS_FCloseFile(file);
            return Scr_ReadFile_LoadObj(filename, extFilename, codePos, archive);
        }
    }
    else if (useFastFile->current.enabled)
    {
        return Scr_ReadFile_FastFile(filename, extFilename, codePos, archive);
    }
    else
    {
        return Scr_ReadFile_LoadObj(filename, extFilename, codePos, archive);
    }
}

char *__cdecl Scr_ReadFile_FastFile(const char *filename, const char *extFilename, const char *codePos, bool archive)
{
    RawFile *rawfile; // [esp+14h] [ebp-Ch]
    char *sourceBuf; // [esp+1Ch] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, extFilename).rawfile;
    if (rawfile)
    {
        sourceBuf = (char *)rawfile->buffer;
        Scr_AddSourceBufferInternal(extFilename, codePos, sourceBuf, strlen(sourceBuf), 1, archive);
        return sourceBuf;
    }
    else
    {
        Scr_AddSourceBufferInternal(extFilename, codePos, 0, -1, 1, archive);
        return 0;
    }
}

unsigned int __cdecl Scr_GetSourcePos(
    unsigned int bufferIndex,
    unsigned int sourcePos,
    char *outBuf,
    unsigned int outBufLen)
{
    unsigned int lineNum; // [esp+4h] [ebp-40Ch]
    char line[1024]; // [esp+8h] [ebp-408h] BYREF
    int col; // [esp+40Ch] [ebp-4h] BYREF

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1010, 0, "%s", "scrVarPub.developer");
    lineNum = Scr_GetLineInfo(scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, sourcePos, &col, line);
    if (scrParserGlob.saveSourceBufferLookup)
        Com_sprintf(
            outBuf,
            outBufLen,
            "%s // %s%s, line %d",
            line,
            scrParserPub.sourceBufferLookup[bufferIndex].buf,
            " (savegame)",
            lineNum + 1);
    else
        Com_sprintf(
            outBuf,
            outBufLen,
            "%s // %s%s, line %d",
            line,
            scrParserPub.sourceBufferLookup[bufferIndex].buf,
            "",
            lineNum + 1);
    return lineNum;
}

unsigned int __cdecl Scr_GetLineInfo(const char *buf, unsigned int sourcePos, int *col, char *line)
{
    const char *startLine; // [esp+0h] [ebp-8h] BYREF
    unsigned int lineNum; // [esp+4h] [ebp-4h]

    lineNum = Scr_GetLineNumInternal(buf, sourcePos, &startLine, col);
    Scr_CopyFormattedLine(line, startLine);
    return lineNum;
}

void __cdecl Scr_CopyFormattedLine(char *line, const char *rawLine)
{
    char v2; // [esp+3h] [ebp-1Dh]
    int len; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]

    len = strlen(rawLine);
    if (len >= 1024)
        len = 1023;
    for (i = 0; i < len; ++i)
    {
        if (rawLine[i] == 9)
            v2 = 32;
        else
            v2 = rawLine[i];
        line[i] = v2;
    }
    if (line[len - 1] == 13)
        line[len - 1] = 0;
    line[len] = 0;
}

unsigned int __cdecl Scr_GetSourceBuffer(const char *codePos)
{
    unsigned int bufferIndex; // [esp+0h] [ebp-4h]

    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1022, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    if (!scrParserPub.sourceBufferLookupLen)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1023, 0, "%s", "scrParserPub.sourceBufferLookupLen > 0");
    for (bufferIndex = scrParserPub.sourceBufferLookupLen - 1;
        bufferIndex
        && (!scrParserPub.sourceBufferLookup[bufferIndex].codePos
            || scrParserPub.sourceBufferLookup[bufferIndex].codePos > codePos);
        --bufferIndex)
    {
        ;
    }
    return bufferIndex;
}

void __cdecl Scr_PrintPrevCodePos(int channel, char *codePos, unsigned int index)
{
    char *v3; // eax
    unsigned int PrevSourcePos; // eax
    char *v5; // eax
    unsigned int bufferIndex; // [esp+0h] [ebp-4h]

    if (!codePos)
    {
        Com_PrintMessage(channel, "<frozen thread>\n", 0);
        return;
    }
    if (codePos == &g_EndPos)
    {
        Com_PrintMessage(channel, "<removed thread>\n", 0);
    }
    else
    {
        if (scrVarPub.developer)
        {
            if (scrVarPub.programBuffer && Scr_IsInOpcodeMemory(codePos))
            {
                bufferIndex = Scr_GetSourceBuffer(codePos - 1);
                PrevSourcePos = Scr_GetPrevSourcePos(codePos - 1, index);
                Scr_PrintSourcePos(
                    channel,
                    scrParserPub.sourceBufferLookup[bufferIndex].buf,
                    scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf,
                    PrevSourcePos);
                return;
            }
        }
        else if (Scr_IsInOpcodeMemory(codePos - 1))
        {
            v3 = va("@ %d\n", codePos - scrVarPub.programBuffer);
            Com_PrintMessage(channel, v3, 0);
            return;
        }
        v5 = va("%s\n\n", codePos);
        Com_PrintMessage(channel, v5, 0);
    }
}

void __cdecl Scr_PrintSourcePos(int channel, const char *filename, const char *buf, unsigned int sourcePos)
{
    char *v4; // eax
    char *v5; // eax
    const char *v6; // [esp+0h] [ebp-418h]
    unsigned int lineNum; // [esp+4h] [ebp-414h]
    char line[1028]; // [esp+8h] [ebp-410h] BYREF
    int i; // [esp+410h] [ebp-8h]
    int col; // [esp+414h] [ebp-4h] BYREF

    if (!filename)
        MyAssertHandler(".\\script\\scr_parser.cpp", 956, 0, "%s", "filename");
    lineNum = Scr_GetLineInfo(buf, sourcePos, &col, line);
    if (scrParserGlob.saveSourceBufferLookup)
        v6 = " (savegame)";
    else
        v6 = "";
    v4 = va("(file '%s'%s, line %d)\n", filename, v6, lineNum + 1);
    Com_PrintMessage(channel, v4, 0);
    v5 = va("%s\n", line);
    Com_PrintMessage(channel, v5, 0);
    for (i = 0; i < col; ++i)
        Com_PrintMessage(channel, " ", 0);
    Com_PrintMessage(channel, "*\n", 0);
}

char *__cdecl Scr_PrevCodePosFileName(char *codePos)
{
    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1096, 0, "%s", "scrVarPub.developer");
    if (!codePos)
        return "<frozen thread>";
    if (codePos == &g_EndPos)
        return "<removed thread>";
    if (scrVarPub.programBuffer && Scr_IsInOpcodeMemory(codePos))
        return scrParserPub.sourceBufferLookup[Scr_GetSourceBuffer(codePos - 1)].buf;
    return codePos;
}

const char *__cdecl Scr_PrevCodePosFunctionName(char *codePos)
{
    unsigned int PrevSourcePos; // eax
    unsigned int bufferIndex; // [esp+0h] [ebp-8h]
    const char *startLine; // [esp+4h] [ebp-4h] BYREF

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1120, 0, "%s", "scrVarPub.developer");
    if (!codePos)
        return "<frozen thread>";
    if (codePos == &g_EndPos)
        return "<removed thread>";
    if (!scrVarPub.programBuffer || !Scr_IsInOpcodeMemory(codePos))
        return aUnk;
    bufferIndex = Scr_GetSourceBuffer(codePos - 1);
    PrevSourcePos = Scr_GetPrevSourcePos(codePos - 1, 0);
    Scr_GetFunctionLineNumInternal(scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, PrevSourcePos, &startLine);
    return startLine;
}

bool __cdecl Scr_PrevCodePosFileNameMatches(char *codePos, const char *fileName)
{
    char *codePosFileName; // [esp+0h] [ebp-4h]

    if (!fileName)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1149, 0, "%s", "fileName");
    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1150, 0, "%s", "scrVarPub.developer");
    codePosFileName = Scr_PrevCodePosFileName(codePos);
    return codePosFileName && I_stristr(codePosFileName, fileName) != 0;
}

void __cdecl Scr_PrintPrevCodePosSpreadSheet(int channel, char *codePos, bool summary, bool functionSummary)
{
    unsigned int PrevSourcePos; // eax
    char *v5; // eax
    unsigned int bufferIndex; // [esp+0h] [ebp-4h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1165, 0, "%s", "scrVarPub.developer");
    if (codePos)
    {
        if (codePos == &g_EndPos)
        {
            Com_PrintMessage(channel, "<removed thread>\n", 0);
        }
        else if (scrVarPub.programBuffer && Scr_IsInOpcodeMemory(codePos))
        {
            bufferIndex = Scr_GetSourceBuffer(codePos - 1);
            if (summary)
            {
                Scr_GetPrevSourcePos(codePos - 1, 0);
                Scr_PrintSourcePosSummary(channel, scrParserPub.sourceBufferLookup[bufferIndex].buf);
            }
            else
            {
                PrevSourcePos = Scr_GetPrevSourcePos(codePos - 1, 0);
                if (functionSummary)
                    Scr_PrintFunctionPosSpreadSheet(
                        channel,
                        scrParserPub.sourceBufferLookup[bufferIndex].buf,
                        scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf,
                        PrevSourcePos);
                else
                    Scr_PrintSourcePosSpreadSheet(
                        channel,
                        scrParserPub.sourceBufferLookup[bufferIndex].buf,
                        scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf,
                        PrevSourcePos);
            }
        }
        else
        {
            v5 = va("%s\n", codePos);
            Com_PrintMessage(channel, v5, 0);
        }
    }
    else
    {
        Com_PrintMessage(channel, "<frozen thread>\n", 0);
    }
}

void __cdecl Scr_PrintSourcePosSpreadSheet(int channel, const char *filename, const char *buf, unsigned int sourcePos)
{
    char *v4; // eax
    const char *v5; // [esp+0h] [ebp-410h]
    unsigned int lineNum; // [esp+4h] [ebp-40Ch]
    char line[1024]; // [esp+8h] [ebp-408h] BYREF
    int col; // [esp+40Ch] [ebp-4h] BYREF

    if (!filename)
        MyAssertHandler(".\\script\\scr_parser.cpp", 975, 0, "%s", "filename");
    lineNum = Scr_GetLineInfo(buf, sourcePos, &col, line);
    if (scrParserGlob.saveSourceBufferLookup)
        v5 = "(savegame)";
    else
        v5 = "";
    v4 = va("%s%s\t%d\t%s\t%d\n", filename, v5, lineNum + 1, line, col);
    Com_PrintMessage(channel, v4, 0);
}

void __cdecl Scr_PrintFunctionPosSpreadSheet(
    int channel,
    const char *filename,
    const char *buf,
    unsigned int sourcePos)
{
    char *v4; // eax
    const char *v5; // [esp+0h] [ebp-410h]
    unsigned int lineNum; // [esp+4h] [ebp-40Ch]
    char line[1028]; // [esp+8h] [ebp-408h] BYREF

    if (!filename)
        MyAssertHandler(".\\script\\scr_parser.cpp", 988, 0, "%s", "filename");
    lineNum = Scr_GetFunctionInfo(buf, sourcePos, line);
    if (scrParserGlob.saveSourceBufferLookup)
        v5 = "(savegame)";
    else
        v5 = "";
    v4 = va("%s%s\t%d\t%s\n", filename, v5, lineNum + 1, line);
    Com_PrintMessage(channel, v4, 0);
}

unsigned int __cdecl Scr_GetFunctionInfo(const char *buf, unsigned int sourcePos, char *line)
{
    const char *startLine; // [esp+0h] [ebp-8h] BYREF
    unsigned int lineNum; // [esp+4h] [ebp-4h]

    lineNum = Scr_GetFunctionLineNumInternal(buf, sourcePos, &startLine);
    Scr_CopyFormattedLine(line, startLine);
    return lineNum;
}

void __cdecl Scr_PrintSourcePosSummary(int channel, const char *filename)
{
    char *v2; // eax
    const char *v3; // [esp+0h] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\script\\scr_parser.cpp", 998, 0, "%s", "filename");
    if (scrParserGlob.saveSourceBufferLookup)
        v3 = "(savegame)";
    else
        v3 = "";
    v2 = va("%s%s\t\n", filename, v3);
    Com_PrintMessage(channel, v2, 0);
}

void __cdecl Scr_GetCodePos(const char *codePos, unsigned int index, char *outBuf, unsigned int outBufLen)
{
    Scr_SourcePos_t pos; // [esp+0h] [ebp-Ch] BYREF

    if (scrVarPub.developer)
    {
        Scr_GetSourcePosOfType(codePos, 4, &pos);
        Scr_GetSourcePos(pos.bufferIndex, pos.sourcePos, outBuf, outBufLen);
    }
    else
    {
        if (!Scr_IsInOpcodeMemory(codePos))
            MyAssertHandler(".\\script\\scr_parser.cpp", 1201, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
        Com_sprintf(outBuf, outBufLen, "@ %d", codePos - scrVarPub.programBuffer);
    }
}

void __cdecl Scr_GetFileAndLine(const char *codePos, char **filename, int *linenum)
{
    unsigned int bufferIndex; // [esp+0h] [ebp-Ch]
    OpcodeLookup *opcodeLookup; // [esp+4h] [ebp-8h]
    unsigned int sourcePos; // [esp+8h] [ebp-4h]

    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1216, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    opcodeLookup = Scr_GetPrevSourcePosOpcodeLookup(codePos);
    if (opcodeLookup)
    {
        sourcePos = scrParserGlob.sourcePosLookup[opcodeLookup->sourcePosIndex].sourcePos;
        bufferIndex = Scr_GetSourceBuffer(codePos);
        *linenum = Scr_GetLineNum(bufferIndex, sourcePos) + 1;
        *filename = scrParserPub.sourceBufferLookup[bufferIndex].buf;
    }
    else
    {
        *linenum = 0;
        *filename = (char *)"";
    }
}

void __cdecl Scr_AddProfileTime(const char *codePos, int time, int builtInTime)
{
    unsigned int low; // [esp+0h] [ebp-Ch]
    unsigned int middle; // [esp+4h] [ebp-8h]
    unsigned int high; // [esp+8h] [ebp-4h]

    if (time < 0)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1240, 0, "%s\n\t(time) = %i", "(time >= 0)", time);
    if (builtInTime > time)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1241, 0, "builtInTime <= time\n\t%i, %i", builtInTime, time);
    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1242, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    if (!scrParserGlob.opcodeLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1243, 0, "%s", "scrParserGlob.opcodeLookup");
    low = 0;
    high = scrParserGlob.opcodeLookupLen - 1;
    while (low <= high)
    {
        middle = (high + low) >> 1;
        if (codePos <= scrParserGlob.opcodeLookup[middle].codePos)
        {
            high = middle - 1;
        }
        else
        {
            low = middle + 1;
            if (middle + 1 >= scrParserGlob.opcodeLookupLen)
                MyAssertHandler(".\\script\\scr_parser.cpp", 1253, 0, "%s", "low < scrParserGlob.opcodeLookupLen");
            if (codePos <= scrParserGlob.opcodeLookup[low].codePos)
            {
                scrParserGlob.opcodeLookup[middle].profileTime += time;
                scrParserGlob.opcodeLookup[middle].profileBuiltInTime += builtInTime;
                ++scrParserGlob.opcodeLookup[middle].profileUsage;
                return;
            }
        }
    }
    if (!alwaysfails)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1268, 0, "unreachable");
}

void __cdecl Scr_CalcScriptFileProfile()
{
    const char *codePos; // ecx
    SourceBufferInfo *v1; // [esp+10Ch] [ebp-101Ch]
    char *pos; // [esp+110h] [ebp-1018h]
    int profileTime; // [esp+114h] [ebp-1014h]
    const GfxStaticModelDrawInst *v4[1025]; // [esp+118h] [ebp-1010h] BYREF
    ProfileScript *Script; // [esp+111Ch] [ebp-Ch]
    unsigned int i; // [esp+1120h] [ebp-8h]
    int profileBuiltInTime; // [esp+1124h] [ebp-4h]

    v4[1024] = (const GfxStaticModelDrawInst *)1024;
    if (scrVarPub.developer)
    {
        Script = //Profile_GetScript();
            Script->srcTotal = 0;
        for (i = 0; i < scrParserGlob.opcodeLookupLen; ++i)
        {
            if (scrParserGlob.opcodeLookup[i].profileUsage)
            {
                profileTime = scrParserGlob.opcodeLookup[i].profileTime;
                profileBuiltInTime = scrParserGlob.opcodeLookup[i].profileBuiltInTime;
                scrParserGlob.opcodeLookup[i].profileUsage = 0;
                scrParserGlob.opcodeLookup[i].profileTime = 0;
                scrParserGlob.opcodeLookup[i].profileBuiltInTime = 0;
                codePos = scrParserGlob.opcodeLookup[i].codePos;
                pos = (char *)(codePos + 1);
                if (codePos == (const char *)-1 || pos == &g_EndPos)
                    MyAssertHandler(".\\script\\scr_parser.cpp", 1313, 0, "%s", "codePos && codePos != &g_EndPos");
                if (!scrVarPub.programBuffer || !Scr_IsInOpcodeMemory(pos))
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        1314,
                        0,
                        "%s",
                        "scrVarPub.programBuffer && Scr_IsInOpcodeMemory( codePos )");
                v1 = &scrParserPub.sourceBufferLookup[Scr_GetSourceBuffer(pos - 1)];
                v1->time = profileTime;
                v1->avgTime = (profileTime + 4 * v1->avgTime) / 5;
                if (profileTime > v1->maxTime)
                    v1->maxTime = profileTime;
                v1->totalTime = (double)profileTime * *((float *)Sys_GetValue(0) + 20782) + v1->totalTime;
                v1->totalBuiltIn = (double)profileBuiltInTime * *((float *)Sys_GetValue(0) + 20782) + v1->totalBuiltIn;
                Script->srcTotal += profileTime;
            }
            else if (scrParserGlob.opcodeLookup[i].profileTime)
            {
                MyAssertHandler(".\\script\\scr_parser.cpp", 1302, 0, "%s", "!scrParserGlob.opcodeLookup[i].profileTime");
            }
        }
        Script->srcAvgTime = (Script->srcTotal + 4 * Script->srcAvgTime) / 5;
        if (Script->srcTotal > Script->srcMaxTime)
            Script->srcMaxTime = Script->srcTotal;
        if (scrParserPub.sourceBufferLookupLen >= 0x400)
            MyAssertHandler(
                ".\\script\\scr_parser.cpp",
                1334,
                0,
                "%s",
                "scrParserPub.sourceBufferLookupLen < MAX_SCRIPT_FILES");
        for (i = 0; i < scrParserPub.sourceBufferLookupLen; ++i)
            v4[i] = (const GfxStaticModelDrawInst *)i;
        std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
            v4,
            &v4[scrParserPub.sourceBufferLookupLen],
            (signed int)(4 * scrParserPub.sourceBufferLookupLen) >> 2,
            (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))Scr_CompareScriptSourceProfileTimes);
        memcpy(Script->scriptSrcBufferIndex, v4, sizeof(Script->scriptSrcBufferIndex));
    }
}

bool __cdecl Scr_CompareScriptSourceProfileTimes(int index1, int index2)
{
    return scrParserPub.sourceBufferLookup[index2].totalTime < (double)scrParserPub.sourceBufferLookup[index1].totalTime;
}

void __cdecl Scr_CalcAnimscriptProfile(int *total, int *totalNonBuiltIn)
{
    const char *v2; // edx
    SourceBufferInfo *srcBuffer; // [esp+4h] [ebp-14h]
    char *codePos; // [esp+8h] [ebp-10h]
    int profileTime; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+10h] [ebp-8h]
    int profileBuiltInTime; // [esp+14h] [ebp-4h]

    *total = 0;
    *totalNonBuiltIn = 0;
    if (scrVarPub.developer)
    {
        for (i = 0; i < scrParserGlob.opcodeLookupLen; ++i)
        {
            if (scrParserGlob.opcodeLookup[i].profileUsage)
            {
                profileTime = scrParserGlob.opcodeLookup[i].profileTime;
                profileBuiltInTime = scrParserGlob.opcodeLookup[i].profileBuiltInTime;
                scrParserGlob.opcodeLookup[i].profileUsage = 0;
                scrParserGlob.opcodeLookup[i].profileTime = 0;
                scrParserGlob.opcodeLookup[i].profileBuiltInTime = 0;
                v2 = scrParserGlob.opcodeLookup[i].codePos;
                codePos = (char *)(v2 + 1);
                if (v2 == (const char *)-1 || codePos == &g_EndPos)
                    MyAssertHandler(".\\script\\scr_parser.cpp", 1371, 0, "%s", "codePos && codePos != &g_EndPos");
                if (!scrVarPub.programBuffer || !Scr_IsInOpcodeMemory(codePos))
                    MyAssertHandler(
                        ".\\script\\scr_parser.cpp",
                        1372,
                        0,
                        "%s",
                        "scrVarPub.programBuffer && Scr_IsInOpcodeMemory( codePos )");
                srcBuffer = &scrParserPub.sourceBufferLookup[Scr_GetSourceBuffer(codePos - 1)];
                if (!strncmp(srcBuffer->buf, "animscript", 0xAu))
                {
                    *total += profileTime;
                    *totalNonBuiltIn += profileTime - profileBuiltInTime;
                }
            }
            else if (scrParserGlob.opcodeLookup[i].profileTime)
            {
                MyAssertHandler(".\\script\\scr_parser.cpp", 1360, 0, "%s", "!scrParserGlob.opcodeLookup[i].profileTime");
            }
        }
    }
}

char __cdecl Scr_PrintProfileTimes(float minTime)
{
    OpcodeLookup *v2; // ecx
    const char **v3; // edx
    int v4; // [esp+34h] [ebp-A0h]
    unsigned int v5; // [esp+38h] [ebp-9Ch]
    float v6; // [esp+3Ch] [ebp-98h]
    float v7; // [esp+44h] [ebp-90h]
    float v8; // [esp+4Ch] [ebp-88h]
    int profileCount; // [esp+ACh] [ebp-28h]
    OpcodeLookup *opcodeLookup; // [esp+B0h] [ebp-24h]
    ProfileScript *profile; // [esp+B4h] [ebp-20h]
    char *name; // [esp+B8h] [ebp-1Ch]
    int time; // [esp+BCh] [ebp-18h]
    unsigned int maxNameLength; // [esp+C0h] [ebp-14h]
    unsigned int i; // [esp+C8h] [ebp-Ch]
    unsigned int ia; // [esp+C8h] [ebp-Ch]
    unsigned int ib; // [esp+C8h] [ebp-Ch]
    unsigned int ic; // [esp+C8h] [ebp-Ch]
    char *sortedOpcodeLookup; // [esp+CCh] [ebp-8h]
    int profileIndex; // [esp+D0h] [ebp-4h]
    int profileIndexa; // [esp+D0h] [ebp-4h]
    int profileIndexb; // [esp+D0h] [ebp-4h]
    int profileIndexc; // [esp+D0h] [ebp-4h]

    if (minTime <= 0.0)
        goto LABEL_10;
    time = 0;
    for (i = 0; i < scrParserGlob.opcodeLookupLen; time += scrParserGlob.opcodeLookup[i++].profileTime)
        ;
    if (minTime <= (double)time * *((float *)Sys_GetValue(0) + 20782))
    {
    LABEL_10:
        profileCount = 0;
        for (ib = 0; ib < scrParserGlob.opcodeLookupLen; ++ib)
        {
            if (scrParserGlob.opcodeLookup[ib].profileUsage)
            {
                ++profileCount;
            }
            else if (scrParserGlob.opcodeLookup[ib].profileTime)
            {
                MyAssertHandler(".\\script\\scr_parser.cpp", 1469, 0, "%s", "!scrParserGlob.opcodeLookup[i].profileTime");
            }
        }
        sortedOpcodeLookup = Z_VirtualAlloc(24 * profileCount, "Scr_PrintProfileTimes", 0);
        profileIndex = 0;
        for (ic = 0; ic < scrParserGlob.opcodeLookupLen; ++ic)
        {
            if (scrParserGlob.opcodeLookup[ic].profileUsage)
            {
                v2 = &scrParserGlob.opcodeLookup[ic];
                v3 = (const char **)&sortedOpcodeLookup[24 * profileIndex];
                *v3 = v2->codePos;
                v3[1] = (const char *)v2->sourcePosIndex;
                v3[2] = (const char *)v2->sourcePosCount;
                v3[3] = (const char *)v2->profileTime;
                v3[4] = (const char *)v2->profileBuiltInTime;
                v3[5] = (const char *)v2->profileUsage;
                ++profileIndex;
                scrParserGlob.opcodeLookup[ic].profileUsage = 0;
                scrParserGlob.opcodeLookup[ic].profileTime = 0;
                scrParserGlob.opcodeLookup[ic].profileBuiltInTime = 0;
            }
            else if (scrParserGlob.opcodeLookup[ic].profileTime)
            {
                MyAssertHandler(".\\script\\scr_parser.cpp", 1482, 0, "%s", "!scrParserGlob.opcodeLookup[i].profileTime");
            }
        }
        if (profileIndex != profileCount)
            MyAssertHandler(".\\script\\scr_parser.cpp", 1492, 0, "%s", "profileIndex == profileCount");
        std::_Sort<MapProfileHotSpot *, int, bool(__cdecl *)(MapProfileHotSpot const &, MapProfileHotSpot const &)>(
            (MapProfileHotSpot *)sortedOpcodeLookup,
            (MapProfileHotSpot *)&sortedOpcodeLookup[24 * profileCount],
            24 * profileCount / 24,
            (bool(__cdecl *)(const MapProfileHotSpot *, const MapProfileHotSpot *))Scr_CompareProfileTimes);
        Com_Printf(23, "\n");
        profile = //Profile_GetScript();
            maxNameLength = 0;
        for (profileIndexa = 0; profileIndexa < 40; ++profileIndexa)
        {
            v4 = (int)&profile->profileScriptNames[profileIndexa][1];
            v5 = (unsigned int)&profile->profileScriptNames[profileIndexa][strlen(profile->profileScriptNames[profileIndexa])
                + 1];
            if (v5 - v4 > maxNameLength)
                maxNameLength = v5 - v4;
        }
        for (profileIndexb = 0; profileIndexb < 40; ++profileIndexb)
        {
            name = profile->profileScriptNames[profileIndexb];
            if (*name)
            {
                strlen(name);
                v8 = *((float *)Sys_GetValue(0) + 20782);
                Com_Printf(23, "%-*s %6.2f\n", maxNameLength, name, (double)profile->write[profileIndexb].totalTime * v8);
            }
        }
        Com_Printf(23, "\n");
        for (profileIndexc = 0; profileIndexc < profileCount; ++profileIndexc)
        {
            opcodeLookup = (OpcodeLookup *)&sortedOpcodeLookup[24 * profileIndexc];
            v7 = *((float *)Sys_GetValue(0) + 20782);
            v6 = *((float *)Sys_GetValue(0) + 20782);
            Com_Printf(
                23,
                "time: %f, builtin: %f, usage: %d\n",
                (double)opcodeLookup->profileTime * v6,
                (double)opcodeLookup->profileBuiltInTime * v7,
                opcodeLookup->profileUsage);
            Scr_PrintPrevCodePos(23, (char *)opcodeLookup->codePos + 1, 1u);
        }
        Com_Printf(23, "\n");
        Z_VirtualFree(sortedOpcodeLookup);
        return 1;
    }
    else
    {
        for (ia = 0; ia < scrParserGlob.opcodeLookupLen; ++ia)
        {
            scrParserGlob.opcodeLookup[ia].profileUsage = 0;
            scrParserGlob.opcodeLookup[ia].profileTime = 0;
            scrParserGlob.opcodeLookup[ia].profileBuiltInTime = 0;
        }
        return 0;
    }
}

bool __cdecl Scr_CompareProfileTimes(const OpcodeLookup *opcodeLookup1, const OpcodeLookup *opcodeLookup2)
{
    return opcodeLookup1->profileTime < opcodeLookup2->profileTime;
}

void CompileError(unsigned int sourcePos, const char *msg, ...)
{
    char line[1024]; // [esp+4h] [ebp-808h] BYREF
    char text[1024]; // [esp+404h] [ebp-408h] BYREF
    int col; // [esp+808h] [ebp-4h] BYREF
    va_list va; // [esp+81Ch] [ebp+10h] BYREF

    va_start(va, msg);
    _vsnprintf(text, 0x400u, msg, va);
    if (scrVarPub.evaluate)
    {
        if (!scrVarPub.error_message)
            scrVarPub.error_message = ::va("%s", text);
    }
    else
    {
        Scr_IgnoreLeaks();
        Scr_ShutdownAllocNode();
        Com_PrintError(23, "\n");
        Com_PrintError(23, "******* script compile error *******\n");
        if (scrVarPub.developer)
        {
            if (!scrParserPub.sourceBuf)
                MyAssertHandler(".\\script\\scr_parser.cpp", 1578, 0, "%s", "scrParserPub.sourceBuf");
            Com_PrintError(23, "%s: ", text);
            Scr_PrintSourcePos(23, scrParserPub.scriptfilename, scrParserPub.sourceBuf, sourcePos);
            Scr_GetLineInfo(scrParserPub.sourceBuf, sourcePos, &col, line);
        }
        else
        {
            Com_PrintError(23, "%s\n", text);
            line[0] = 0;
        }
        Com_Printf(23, "************************************\n");
        Com_Error(ERR_SCRIPT_DROP, &byte_8A559C, text, line);
    }
}

scrStringDebugGlob_t *Scr_IgnoreLeaks()
{
    scrStringDebugGlob_t *result; // eax

    if (scrStringDebugGlob)
    {
        result = scrStringDebugGlob;
        scrStringDebugGlob->ignoreLeaks = 1;
    }
    scrVarDebugPub = 0;
    return result;
}

void CompileError2(char *codePos, char *msg, ...)
{
    char line[1024]; // [esp+4h] [ebp-808h] BYREF
    char text[1028]; // [esp+404h] [ebp-408h] BYREF
    va_list va; // [esp+81Ch] [ebp+10h] BYREF

    va_start(va, msg);
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_parser.cpp", 1595, 0, "%s", "!scrVarPub.evaluate");
    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1596, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    Scr_IgnoreLeaks();
    Com_PrintError(23, "\n");
    Com_PrintError(23, "******* script compile error *******\n");
    _vsnprintf(text, 0x400u, msg, va);
    Com_PrintError(23, "%s: ", text);
    Scr_PrintPrevCodePos(23, codePos, 0);
    Com_Printf(23, "************************************\n");
    Scr_GetTextSourcePos(scrParserPub.sourceBuf, codePos, line);
    Com_Error(ERR_SCRIPT_DROP, &byte_8A559C, text, line);
}

void __cdecl Scr_GetTextSourcePos(const char *buf, char *codePos, char *line)
{
    unsigned int PrevSourcePos; // eax
    unsigned int bufferIndex; // [esp+0h] [ebp-8h]
    int col; // [esp+4h] [ebp-4h] BYREF

    if (scrVarPub.developer
        && codePos
        && codePos != &g_EndPos
        && scrVarPub.programBuffer
        && Scr_IsInOpcodeMemory(codePos))
    {
        bufferIndex = Scr_GetSourceBuffer(codePos - 1);
        PrevSourcePos = Scr_GetPrevSourcePos(codePos - 1, 0);
        Scr_GetLineInfo(scrParserPub.sourceBufferLookup[bufferIndex].sourceBuf, PrevSourcePos, &col, line);
    }
    else
    {
        *line = 0;
    }
}

void __cdecl RuntimeError(char *codePos, unsigned int index, const char *msg, const char *dialogMessage)
{
    const char *v4; // [esp+4h] [ebp-Ch]
    bool v5; // [esp+8h] [ebp-8h]
    bool abort_on_error; // [esp+Fh] [ebp-1h]

    if (scrVarPub.developer)
        goto LABEL_5;
    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1650, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    if (scrVmPub.terminal_error)
    {
    LABEL_5:
        if (scrVmPub.debugCode)
        {
            Com_Printf(23, "%s\n", msg);
            if (!scrVmPub.terminal_error)
                return;
            goto error;
        }
        v5 = scrVmPub.abort_on_error || scrVmPub.terminal_error;
        abort_on_error = v5;
        if (Scr_IgnoreErrors())
            abort_on_error = 0;
        RuntimeErrorInternal(abort_on_error ? 23 : 6, codePos, index, msg);
        if (abort_on_error)
        {
        error:
            if (scrVmPub.terminal_error)
                Scr_IgnoreLeaks();
            if (dialogMessage)
                v4 = dialogMessage;
            else
                v4 = "";
            if (dialogMessage)
                Com_Error((errorParm_t)(scrVmPub.terminal_error + 4), &byte_8A563C, msg, "\n", v4);
            else
                Com_Error((errorParm_t)(scrVmPub.terminal_error + 4), &byte_8A563C, msg, "", v4);
        }
    }
}

void __cdecl RuntimeErrorInternal(int channel, char *codePos, unsigned int index, const char *msg)
{
    int i; // [esp+4h] [ebp-4h]

    if (!Scr_IsInOpcodeMemory(codePos))
        MyAssertHandler(".\\script\\scr_parser.cpp", 1622, 0, "%s", "Scr_IsInOpcodeMemory( codePos )");
    Com_PrintError(channel, "\n******* script runtime error *******\n%s: ", msg);
    Scr_PrintPrevCodePos(channel, codePos, index);
    if (scrVmPub.function_count)
    {
        for (i = scrVmPub.function_count - 1; i >= 1; --i)
        {
            Com_PrintError(channel, "called from:\n");
            Scr_PrintPrevCodePos(
                0,
                (char *)scrVmPub.stack[3 * i - 96].u.intValue,
                scrVmPub.function_frame_start[i].fs.localId == 0);
        }
        Com_PrintError(channel, "started from:\n");
        Scr_PrintPrevCodePos(0, (char *)scrVmPub.function_frame_start[0].fs.pos, 1u);
    }
    Com_PrintError(channel, "************************************\n");
}

