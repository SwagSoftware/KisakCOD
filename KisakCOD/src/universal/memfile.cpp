#include <qcommon/qcommon.h>

#include "memfile.h"

static int g_cacheSize;
static int g_nonZeroCount;
static int g_zeroCount;
static int g_cacheBufferLen;

#define CODE_LEN_MASK 63

static byte g_cacheBuffer[CODE_LEN_MASK + 2];

void MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data)
{
    iassert(memFile);
    iassert(memFile->archiveProc);

    memFile->archiveProc(memFile, bytes, (byte *)data);
}

static void MemFile_WriteDataForArchive(MemoryFile* memFile, int bytes, byte* data)
{
    MemFile_WriteData(memFile, bytes, data);
}

void MemFile_CommonInit(MemoryFile* memFile, int size, byte* buffer, bool errorOnOverflow, bool compress)
{
    iassert(memFile);
    iassert(buffer);
    vassert(size > 0, "(size = %d)", size);

    memFile->buffer = buffer;
    memFile->bufferSize = size;
    memFile->bytesUsed = 0;
    memFile->errorOnOverflow = errorOnOverflow;
    memFile->memoryOverflow = 0;
    memFile->segmentIndex = -1;
    memFile->segmentStart = 0;
    memFile->compress = compress;
}

void MemFile_InitForReading(MemoryFile* memFile, int size, byte* buffer, bool compress)
{
    MemFile_CommonInit(memFile, size, buffer, 1, compress);
    memFile->archiveProc = MemFile_ReadData;
    MemFile_MoveToSegment(memFile, 0);
}

void MemFile_InitForWriting(MemoryFile* memFile, int size, byte* buffer,
    bool errorOnOverflow, bool compress)
{
    MemFile_CommonInit(memFile, size, buffer, errorOnOverflow, compress);
    memFile->archiveProc = MemFile_WriteDataForArchive;
    MemFile_StartSegment(memFile, 0);
}

bool MemFile_IsReading(MemoryFile* memFile)
{
    iassert(memFile);
    return memFile->archiveProc == MemFile_ReadData;
}

bool MemFile_IsWriting(MemoryFile* memFile)
{
    iassert(memFile);
    return memFile->archiveProc == MemFile_WriteDataForArchive;
}

double MemFile_ReadFloat(MemoryFile* memFile)
{
    float value = NAN;
    MemFile_ReadData(memFile, 4, (byte *)&value);
    iassert(!isnan(value));
    return value;
}

#define SAVE_SEGMENT_COUNT 7

// KISAKTODO cleaning this up is going to be such a headache

void MemFile_StartSegment(MemoryFile* memFile, int index)
{
    vassert(((index >= -1) && (index < SAVE_SEGMENT_COUNT)), "(index = %d)", index);

    // bail if we've already overflowed
    if (memFile->memoryOverflow)
        return;

    // if we already have a segment, end it and check for overflow again
    if (memFile->segmentIndex >= 0) {
        MemFile_EndSegment(memFile);

        if (memFile->memoryOverflow)
            return;
    }

    // start new segment, for real this time.
    int lastSegmentIndex = memFile->segmentIndex;

    memFile->segmentIndex = index;
    if (index >= 0)
    {
        iassert(index == lastSegmentIndex + 1);

        memFile->segmentStart = memFile->bytesUsed;
        if (memFile->bytesUsed + 69 <= memFile->bufferSize)
        {
            memFile->bytesUsed += 4;
            iassert(!memFile->memoryOverflow);

            MemFile_deflateInit(
                &memFile->buffer[memFile->bytesUsed],
                memFile->bufferSize - memFile->bytesUsed,
                memFile->compress);

            g_cacheSize = 1;
            g_nonZeroCount = 0;
            g_zeroCount = 0;
            g_cacheBufferLen = -1;
        }
        else
        {
            // WHOOPS!

            if (memFile->errorOnOverflow)
                Com_Error(ERR_DROP, "MemFile_StartSegment: Out of memory");

            Com_Printf(10, "MemFile_StartSegment: Out of memory\n");
            memFile->memoryOverflow = 1;
        }
    }
    else
    {
        // NOTE: -1 is a dummy used to indicate "end of file"
        // such that the logic needed to ensure flushed memfiles is completed
        // ... this is in the "start segment" function, for some reason? whatever
        memFile->bufferSize = memFile->bytesUsed;
    }
}

void __cdecl MemFile_deflateInit(unsigned __int8* next_out, unsigned int avail_out, bool compress)
{
    AssertStreamMode(MEM_FILE_MODE_DEFAULT);
    if (compress)
    {
        memset((unsigned __int8*)&stream, 0, sizeof(stream));
        stream.next_out = next_out;
        stream.avail_out = avail_out;
        if (deflateInit_(&stream, 1, "1.1.4", 52))
            MyAssertHandler(".\\universal\\memfile.cpp", 224, 0, "%s", "err == Z_OK");
    }
    SetStreamMode(MEM_FILE_MODE_DEFLATE);
    g_compress = compress;
}

void __cdecl MemFile_EndSegment(MemoryFile* memFile)
{
    unsigned int err; // [esp+0h] [ebp-Ch]
    unsigned int index; // [esp+8h] [ebp-4h]

    if (memFile->memoryOverflow)
        MyAssertHandler(".\\universal\\memfile.cpp", 310, 0, "%s", "!memFile->memoryOverflow");
    if (!memFile->memoryOverflow)
    {
        index = memFile->segmentIndex;
        if (index >= 8)
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                317,
                0,
                "%s\n\t(index) = %i",
                "((index >= 0) && (index < SAVE_SEGMENT_COUNT))",
                index);
        if (g_cacheSize > 1)
        {
            if (g_cacheBufferLen < 0)
                MyAssertHandler(".\\universal\\memfile.cpp", 321, 0, "%s", "g_cacheBufferLen >= 0");
            if (!MemFile_WriteDataInternal(memFile, g_cacheSize, g_nonZeroCount, g_cacheBufferLen, 0))
            {
                MemFile_WriteError(memFile);
                return;
            }
            g_cacheSize = 0;
        }
        AssertStreamMode(MEM_FILE_MODE_DEFLATE);
        if (!memFile->compress)
            goto LABEL_30;
        stream.next_in = g_saveBuffer;
        if (&memFile->buffer[memFile->bytesUsed] != stream.next_out)
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                337,
                0,
                "%s",
                "memFile->buffer + memFile->bytesUsed == stream.next_out");
        err = deflate(&stream, 4u);
        if (err > 1)
            MyAssertHandler(".\\universal\\memfile.cpp", 340, 0, "%s\n\t(err) = %i", "((err == 0) || (err == 1))", err);
        memFile->bytesUsed = stream.next_out - memFile->buffer;
        if (memFile->bytesUsed > memFile->bufferSize)
            MyAssertHandler(".\\universal\\memfile.cpp", 343, 0, "%s", "memFile->bytesUsed <= memFile->bufferSize");
        if (err == 1)
        {
        LABEL_30:
            if (MemFile_deflateEnd(memFile->compress))
                MyAssertHandler(".\\universal\\memfile.cpp", 362, 0, "%s", "err == Z_OK");
            memFile->segmentIndex = -1;
            *(unsigned int*)&memFile->buffer[memFile->segmentStart] = memFile->bytesUsed - memFile->segmentStart;
        }
        else
        {
            if (stream.avail_out)
                MyAssertHandler(
                    ".\\universal\\memfile.cpp",
                    347,
                    0,
                    "%s\n\t(stream.avail_out) = %i",
                    "(!stream.avail_out)",
                    stream.avail_out);
            MemFile_deflateEnd(memFile->compress);
            if (memFile->errorOnOverflow)
                Com_Error(ERR_DROP, "MemFile_EndSegment: Out of memory");
            Com_Printf(10, "MemFile_EndSegment: Out of memory");
            memFile->memoryOverflow = 1;
        }
    }
}

unsigned int __cdecl MemFile_deflateEnd(bool compress)
{
    unsigned int err; // [esp+0h] [ebp-4h]

    AssertStreamMode(MEM_FILE_MODE_DEFLATE);
    if (compress)
        err = deflateEnd(&stream);
    else
        err = 0;
    SetStreamMode(MEM_FILE_MODE_DEFAULT);
    return err;
}

void __cdecl MemFile_MoveToSegment(MemoryFile* memFile, int index)
{
    unsigned __int8* data; // [esp+4h] [ebp-8h]
    unsigned int len; // [esp+8h] [ebp-4h]

    if (index < -1 || index >= 8)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            446,
            0,
            "%s\n\t(index) = %i",
            "((index >= -1) && (index < SAVE_SEGMENT_COUNT))",
            index);
    if (!memFile->memoryOverflow)
    {
        if (memFile->segmentIndex >= 0 && MemFile_inflateEnd(memFile->compress))
            MyAssertHandler(".\\universal\\memfile.cpp", 454, 0, "%s", "err == Z_OK");
        memFile->segmentIndex = index;
        if (index >= 0)
        {
            data = MemFile_GetSegmentAddess(memFile, index);
            len = *(unsigned int*)data - 4;
            memFile->bytesUsed = data - memFile->buffer + 4;
            MemFile_inflateInit(&memFile->buffer[memFile->bytesUsed], len, memFile->compress);
            g_nonZeroCount = 0;
            g_zeroCount = 0;
        }
    }
}

void __cdecl MemFile_inflateInit(unsigned __int8* next_in, unsigned int len, bool compress)
{
    AssertStreamMode(MEM_FILE_MODE_DEFAULT);
    if (compress)
    {
        memset((unsigned __int8*)&stream, 0, sizeof(stream));
        stream.next_in = next_in;
        stream.avail_in = len;
        if (inflateInit_(&stream, "1.1.4", 52))
            MyAssertHandler(".\\universal\\memfile.cpp", 387, 0, "%s", "err == Z_OK");
    }
    SetStreamMode(MEM_FILE_MODE_INFLATE);
    g_compress = compress;
}

int __cdecl MemFile_inflateEnd(bool compress)
{
    int err; // [esp+0h] [ebp-4h]

    AssertStreamMode(MEM_FILE_MODE_INFLATE);
    if (compress)
        err = inflateEnd(&stream);
    else
        err = 0;
    SetStreamMode(MEM_FILE_MODE_DEFAULT);
    return err;
}

unsigned __int8* __cdecl MemFile_GetSegmentAddess(MemoryFile* memFile, unsigned int index)
{
    int segmentStart; // [esp+0h] [ebp-4h]

    if (index >= 8)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            418,
            0,
            "%s\n\t(index) = %i",
            "((index >= 0) && (index < SAVE_SEGMENT_COUNT))",
            index);
    if (memFile->memoryOverflow)
        MyAssertHandler(".\\universal\\memfile.cpp", 419, 0, "%s", "!memFile->memoryOverflow");
    segmentStart = 0;
    while (index)
    {
        if (segmentStart + 4 > memFile->bufferSize)
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                424,
                0,
                "%s",
                "segmentStart + static_cast< int >( sizeof( int ) ) <= memFile->bufferSize");
        segmentStart += *(unsigned int*)&memFile->buffer[segmentStart];
        --index;
    }
    if (segmentStart + 4 > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            429,
            0,
            "%s",
            "segmentStart + static_cast< int >( sizeof( int ) ) <= memFile->bufferSize");
    return &memFile->buffer[segmentStart];
}

void __cdecl MemFile_WriteError(MemoryFile* memFile)
{
    if (memFile->memoryOverflow)
        MyAssertHandler(".\\universal\\memfile.cpp", 523, 0, "%s", "!memFile->memoryOverflow");
    MemFile_deflateEnd(memFile->compress);
    if (memFile->errorOnOverflow)
        Com_Error(ERR_DROP, "MemFile_EndSegment: Out of memory");
    Com_Printf(10, "MemFile_EndSegment: Out of memory");
    memFile->memoryOverflow = 1;
}

int __cdecl MemFile_WriteDataInternal(
    MemoryFile* memFile,
    int bytes,
    char nonZeroCount,
    char cacheBufferLen,
    unsigned __int8 nextByte)
{
    signed int sourceLen; // [esp+0h] [ebp-10h]
    unsigned __int8* data; // [esp+8h] [ebp-8h]
    int len; // [esp+Ch] [ebp-4h]

    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 544, 0, "%s", "memFile");
    if (!MemFile_IsWriting(memFile))
        MyAssertHandler(".\\universal\\memfile.cpp", 545, 0, "%s", "MemFile_IsWriting( memFile )");
    if (!memFile->buffer)
        MyAssertHandler(".\\universal\\memfile.cpp", 546, 0, "%s", "memFile->buffer");
    if (memFile->bytesUsed < 0 || memFile->bytesUsed > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            547,
            0,
            "memFile->bytesUsed not in [0, memFile->bufferSize]\n\t%i not in [%i, %i]",
            memFile->bytesUsed,
            0,
            memFile->bufferSize);
    if (bytes <= 0)
        MyAssertHandler(".\\universal\\memfile.cpp", 548, 0, "%s\n\t(bytes) = %i", "(bytes > 0)", bytes);
    AssertStreamMode(MEM_FILE_MODE_DEFLATE);
    if (memFile->memoryOverflow)
        MyAssertHandler(".\\universal\\memfile.cpp", 550, 0, "%s", "!memFile->memoryOverflow");
    if (memFile->compress)
    {
        data = g_cacheBuffer;
        g_cacheBuffer[0] = cacheBufferLen + (nonZeroCount << 6);
        sourceLen = bytes + stream.avail_in;
        if (!(bytes + stream.avail_in))
            MyAssertHandler(".\\universal\\memfile.cpp", 559, 0, "%s", "sourceLen");
        while (sourceLen >= 0x2000)
        {
            if (stream.avail_in >= 0x2000)
                MyAssertHandler(".\\universal\\memfile.cpp", 569, 0, "%s", "stream.avail_in < TEMP_SAVE_BUFFER_SIZE");
            len = 0x2000 - stream.avail_in;
            memcpy(&g_saveBuffer[stream.avail_in], data, 0x2000 - stream.avail_in);
            stream.avail_in = 0x2000;
            sourceLen -= 0x2000;
            data += len;
            stream.next_in = g_saveBuffer;
            if (&memFile->buffer[memFile->bytesUsed] != stream.next_out)
                MyAssertHandler(
                    ".\\universal\\memfile.cpp",
                    578,
                    0,
                    "%s",
                    "memFile->buffer + memFile->bytesUsed == stream.next_out");
            if (deflate(&stream, 2u))
                MyAssertHandler(".\\universal\\memfile.cpp", 581, 0, "%s", "err == Z_OK");
            memFile->bytesUsed = stream.next_out - memFile->buffer;
            if (memFile->bytesUsed > memFile->bufferSize)
                MyAssertHandler(".\\universal\\memfile.cpp", 584, 0, "%s", "memFile->bytesUsed <= memFile->bufferSize");
            if (!stream.avail_out)
                return 0;
            if (!sourceLen)
                goto LABEL_30;
        }
        memcpy(&g_saveBuffer[stream.avail_in], data, sourceLen - stream.avail_in);
        stream.avail_in = sourceLen;
    LABEL_30:
        g_cacheBuffer[1] = nextByte;
        return 1;
    }
    else
    {
        memFile->buffer[memFile->bytesUsed] = cacheBufferLen + (nonZeroCount << 6);
        memFile->bytesUsed += bytes;
        if (memFile->bytesUsed + 65 > memFile->bufferSize)
        {
            return 0;
        }
        else
        {
            memFile->buffer[memFile->bytesUsed + 1] = nextByte;
            return 1;
        }
    }
}

int __cdecl MemFile_GetUsedSize(MemoryFile* memFile)
{
    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 612, 0, "%s", "memFile");
    return memFile->bytesUsed;
}

void __cdecl MemFile_WriteData(MemoryFile* memFile, int byteCount, byte* p)
{
    unsigned int moveByte; // [esp+0h] [ebp-20h]
    unsigned int nextByte; // [esp+8h] [ebp-18h]
    int nonZeroCount; // [esp+Ch] [ebp-14h]
    int cacheBufferLen; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    int zeroCount; // [esp+18h] [ebp-8h]
    int cacheSize; // [esp+1Ch] [ebp-4h]
    int cacheSizea; // [esp+1Ch] [ebp-4h]

    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 643, 0, "%s", "memFile");
    if (!MemFile_IsWriting(memFile))
        MyAssertHandler(".\\universal\\memfile.cpp", 644, 0, "%s", "MemFile_IsWriting( memFile )");
    if (!memFile->buffer)
        MyAssertHandler(".\\universal\\memfile.cpp", 645, 0, "%s", "memFile->buffer");
    if (memFile->bytesUsed < 0 || memFile->bytesUsed > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            646,
            0,
            "memFile->bytesUsed not in [0, memFile->bufferSize]\n\t%i not in [%i, %i]",
            memFile->bytesUsed,
            0,
            memFile->bufferSize);
    if (byteCount < 0)
        MyAssertHandler(".\\universal\\memfile.cpp", 647, 0, "%s\n\t(byteCount) = %i", "(byteCount >= 0)", byteCount);
    if (memFile->memoryOverflow)
        return;
    if (!p)
        MyAssertHandler(".\\universal\\memfile.cpp", 652, 0, "%s", "p");
    cacheSize = g_cacheSize;
    zeroCount = g_zeroCount;
    nonZeroCount = g_nonZeroCount;
    cacheBufferLen = g_cacheBufferLen;
    for (i = 0; ; ++i)
    {
    LABEL_16:
        if (i >= byteCount)
        {
            g_cacheSize = cacheSize;
            g_zeroCount = zeroCount;
            g_nonZeroCount = nonZeroCount;
            g_cacheBufferLen = cacheBufferLen;
            return;
        }
        if (!nonZeroCount)
            break;
        if (i >= byteCount)
            MyAssertHandler(".\\universal\\memfile.cpp", 666, 0, "%s", "i < byteCount");
        while (!p[i])
        {
            ++zeroCount;
            if (cacheBufferLen == 63)
            {
                zeroCount = 1;
                goto LABEL_25;
            }
            ++cacheBufferLen;
            if (++i >= byteCount)
                goto LABEL_16;
        }
        zeroCount = 0;
    LABEL_25:
        if (!MemFile_WriteDataInternal(memFile, cacheSize, nonZeroCount, cacheBufferLen, p[i]))
            goto LABEL_56;
        cacheBufferLen = 0;
        nonZeroCount = 0;
        cacheSize = 2;
    }
    if (i >= byteCount)
        MyAssertHandler(".\\universal\\memfile.cpp", 701, 0, "%s", "i < byteCount");
    while (1)
    {
        nextByte = (unsigned __int8)p[i];
        if (cacheBufferLen == 63)
        {
            if (!MemFile_WriteDataInternal(memFile, cacheSize, 0, 63, nextByte))
                goto LABEL_56;
            cacheBufferLen = 0;
            nonZeroCount = 0;
            cacheSize = 2;
            zeroCount = nextByte == 0;
            ++i;
            goto LABEL_79;
        }
        if (!p[i])
            break;
        zeroCount = 0;
    LABEL_71:
        ++cacheBufferLen;
        if (cacheSize >= 65)
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                786,
                0,
                "%s\n\t(cacheSize) = %i",
                "(cacheSize < CODE_LEN_MASK + 2)",
                cacheSize);
        if (cacheSize <= 0)
            MyAssertHandler(".\\universal\\memfile.cpp", 787, 0, "%s\n\t(cacheSize) = %i", "(cacheSize > 0)", cacheSize);
        if (memFile->compress)
            g_cacheBuffer[cacheSize] = nextByte;
        else
            memFile->buffer[cacheSize + memFile->bytesUsed] = nextByte;
        ++cacheSize;
        ++i;
    LABEL_79:
        if (i >= byteCount)
            goto LABEL_16;
    }
    ++zeroCount;
    if (cacheBufferLen <= 2)
    {
        if (cacheBufferLen >= 0)
        {
            nonZeroCount = cacheBufferLen + 1;
            cacheBufferLen = 0;
            ++i;
            goto LABEL_16;
        }
        goto LABEL_71;
    }
    if (cacheSize <= 2)
        MyAssertHandler(".\\universal\\memfile.cpp", 728, 0, "%s", "cacheSize > 2");
    if (zeroCount < 3)
    {
        if (memFile->compress)
        {
            if (!*(_BYTE*)(cacheSize + 231103374) && !*(_BYTE*)(cacheSize + 231103375))
                MyAssertHandler(
                    ".\\universal\\memfile.cpp",
                    765,
                    0,
                    "%s",
                    "g_cacheBuffer[cacheSize - 2] != 0 || g_cacheBuffer[cacheSize - 1] != 0");
        }
        else if (!memFile->buffer[cacheSize - 2 + memFile->bytesUsed]
            && !memFile->buffer[cacheSize - 1 + memFile->bytesUsed])
        {
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                768,
                0,
                "%s",
                "memFile->buffer[memFile->bytesUsed + cacheSize - 2] != 0 || memFile->buffer[memFile->bytesUsed + cacheSize - 1] != 0");
        }
        goto LABEL_71;
    }
    if (zeroCount != 3)
        MyAssertHandler(".\\universal\\memfile.cpp", 731, 0, "%s", "zeroCount == 3");
    if (cacheSize <= 4)
        MyAssertHandler(".\\universal\\memfile.cpp", 732, 0, "%s\n\t(cacheSize) = %i", "(cacheSize > 4)", cacheSize);
    cacheSizea = cacheSize - 3;
    if (memFile->compress)
    {
        if (*(_BYTE*)(cacheSizea + 231103377) || *(_BYTE*)(cacheSizea + 231103378))
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                739,
                0,
                "%s",
                "g_cacheBuffer[cacheSize + 1] == 0 && g_cacheBuffer[cacheSize + 2] == 0");
        moveByte = g_cacheBuffer[cacheSizea];
    }
    else
    {
        if (memFile->buffer[cacheSizea + 1 + memFile->bytesUsed] || memFile->buffer[cacheSizea + 2 + memFile->bytesUsed])
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                745,
                0,
                "%s",
                "memFile->buffer[memFile->bytesUsed + cacheSize + 1] == 0 && memFile->buffer[memFile->bytesUsed + cacheSize + 2] == 0");
        moveByte = memFile->buffer[cacheSizea + memFile->bytesUsed];
    }
    if (!moveByte)
        MyAssertHandler(".\\universal\\memfile.cpp", 749, 0, "%s", "moveByte");
    if (MemFile_WriteDataInternal(memFile, cacheSizea, 0, cacheBufferLen - 3, moveByte))
    {
        cacheBufferLen = 2;
        nonZeroCount = 1;
        cacheSize = 2;
        zeroCount = 0;
        ++i;
        goto LABEL_16;
    }
LABEL_56:
    MemFile_WriteError(memFile);
}

void __cdecl MemFile_WriteCString(MemoryFile* memFile, char* string)
{
    if (!string)
        MyAssertHandler(".\\universal\\memfile.cpp", 813, 0, "%s", "string");
    MemFile_WriteData(memFile, strlen(string) + 1, string);
}

const char* __cdecl MemFile_ReadCString(MemoryFile* memFile)
{
    unsigned __int8* string; // [esp+0h] [ebp-4h]

    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 824, 0, "%s", "memFile");
    if (!memFile->buffer)
        MyAssertHandler(".\\universal\\memfile.cpp", 825, 0, "%s", "memFile->buffer");
    if (memFile->bytesUsed < 0 || memFile->bytesUsed > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            826,
            0,
            "memFile->bytesUsed not in [0, memFile->bufferSize]\n\t%i not in [%i, %i]",
            memFile->bytesUsed,
            0,
            memFile->bufferSize);
    string = g_saveBuffer;
    while (1)
    {
        MemFile_ReadData(memFile, 1, string);
        if (memFile->memoryOverflow)
            return "";
        if (!*string)
            break;
        if (++string >= (unsigned __int8*)&g_nonZeroCount)
            Com_Error(ERR_DROP, "Trying to read corrupted file");
    }
    return (const char*)g_saveBuffer;
}

void __cdecl MemFile_ReadData(MemoryFile* memFile, int byteCount, unsigned __int8* p)
{
    unsigned __int8* data; // [esp+0h] [ebp-8h]
    unsigned __int8 code; // [esp+7h] [ebp-1h]

    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 900, 0, "%s", "memFile");
    if (!MemFile_IsReading(memFile))
        MyAssertHandler(".\\universal\\memfile.cpp", 901, 0, "%s", "MemFile_IsReading( memFile )");
    if (!memFile->buffer)
        MyAssertHandler(".\\universal\\memfile.cpp", 902, 0, "%s", "memFile->buffer");
    if (memFile->bytesUsed < 0 || memFile->bytesUsed > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            903,
            0,
            "memFile->bytesUsed not in [0, memFile->bufferSize]\n\t%i not in [%i, %i]",
            memFile->bytesUsed,
            0,
            memFile->bufferSize);
    if (byteCount < 0)
        MyAssertHandler(".\\universal\\memfile.cpp", 904, 0, "%s\n\t(byteCount) = %i", "(byteCount >= 0)", byteCount);
    if (byteCount && !memFile->memoryOverflow)
    {
        if (!p)
            MyAssertHandler(".\\universal\\memfile.cpp", 912, 0, "%s", "p");
        data = p;
        while (1)
        {
            while (g_nonZeroCount)
            {
                --g_nonZeroCount;
                --byteCount;
                *data++ = MemFile_ReadByteInternal(memFile);
                if (memFile->memoryOverflow || !byteCount)
                    return;
            }
            while (g_zeroCount)
            {
                --g_zeroCount;
                --byteCount;
                *data++ = 0;
                if (!byteCount)
                    return;
            }
            code = MemFile_ReadByteInternal(memFile);
            if (memFile->memoryOverflow)
                break;
            if ((code & 0xC0) != 0)
            {
                g_nonZeroCount = (int)code >> 6;
                g_zeroCount = (code & 0x3F) + 1;
            }
            else
            {
                g_nonZeroCount = (code & 0x3F) + 1;
                g_zeroCount = 0;
            }
        }
    }
}

unsigned __int8 __cdecl MemFile_ReadByteInternal(MemoryFile* memFile)
{
    unsigned int err; // [esp+0h] [ebp-8h]
    unsigned __int8 result; // [esp+7h] [ebp-1h] BYREF

    if (!memFile)
        MyAssertHandler(".\\universal\\memfile.cpp", 851, 0, "%s", "memFile");
    if (!MemFile_IsReading(memFile))
        MyAssertHandler(".\\universal\\memfile.cpp", 852, 0, "%s", "MemFile_IsReading( memFile )");
    if (!memFile->buffer)
        MyAssertHandler(".\\universal\\memfile.cpp", 853, 0, "%s", "memFile->buffer");
    if (memFile->bytesUsed < 0 || memFile->bytesUsed > memFile->bufferSize)
        MyAssertHandler(
            ".\\universal\\memfile.cpp",
            854,
            0,
            "memFile->bytesUsed not in [0, memFile->bufferSize]\n\t%i not in [%i, %i]",
            memFile->bytesUsed,
            0,
            memFile->bufferSize);
    if (memFile->memoryOverflow)
        MyAssertHandler(".\\universal\\memfile.cpp", 855, 0, "%s", "!memFile->memoryOverflow");
    AssertStreamMode(MEM_FILE_MODE_INFLATE);
    if (memFile->compress)
    {
        stream.next_out = &result;
        stream.avail_out = 1;
        if (&memFile->buffer[memFile->bytesUsed] != stream.next_in)
            MyAssertHandler(
                ".\\universal\\memfile.cpp",
                865,
                0,
                "%s",
                "memFile->buffer + memFile->bytesUsed == stream.next_in");
        err = inflate(&stream, 2);
        if (err > 1)
            MyAssertHandler(".\\universal\\memfile.cpp", 868, 0, "%s\n\t(err) = %i", "((err == 0) || (err == 1))", err);
        memFile->bytesUsed = stream.next_in - memFile->buffer;
        if (!stream.avail_out)
            return result;
    }
    else if (memFile->bytesUsed < memFile->bufferSize)
    {
        return memFile->buffer[memFile->bytesUsed++];
    }
    if (memFile->errorOnOverflow)
        Com_Error(ERR_DROP, "Trying to read corrupted file");
    Com_Printf(10, "Trying to read corrupted file");
    memFile->memoryOverflow = 1;
    return 0;
}
