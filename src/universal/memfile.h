#pragma once

#include <qcommon/qcommon.h>

struct MemoryFile // (SP/MP Same)
{
    byte* buffer;
    int bufferSize;
    int bytesUsed;
    int segmentIndex;
    int segmentStart;
    bool errorOnOverflow;
    bool memoryOverflow;
    bool compress;

    void(* archiveProc)(MemoryFile*, int, byte *);
};

enum MemFileMode : __int32
{                                       // ...
    MEM_FILE_MODE_DEFAULT = 0x0,
    MEM_FILE_MODE_INFLATE = 0x1,
    MEM_FILE_MODE_DEFLATE = 0x2,
    MEM_FILE_MODENUM      = 0x3,
};

void MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data);

void MemFile_CommonInit(MemoryFile* memFile, int size, byte* buffer, bool errorOnOverflow, bool compress);

void MemFile_InitForReading(MemoryFile* memFile, int size, byte* buffer, bool compress);
void MemFile_InitForWriting(MemoryFile* memFile, int size, byte* buffer, bool errorOnOverflow, bool compress);

bool MemFile_IsReading(MemoryFile* memFile);
bool MemFile_IsWriting(MemoryFile* memFile);

double MemFile_ReadFloat(MemoryFile* memFile);

void __cdecl MemFile_StartSegment(MemoryFile* memFile, int index);
void __cdecl MemFile_deflateInit(uint8_t* next_out, uint32_t avail_out, bool compress);
void __cdecl MemFile_EndSegment(MemoryFile *memFile);
uint32_t __cdecl MemFile_deflateEnd(bool compress);
void __cdecl MemFile_MoveToSegment(MemoryFile* memFile, int index);
void __cdecl MemFile_inflateInit(uint8_t* next_in, uint32_t len, bool compress);
int __cdecl MemFile_inflateEnd(bool compress);
uint8_t* __cdecl MemFile_GetSegmentAddess(MemoryFile* memFile, uint32_t index);
void __cdecl MemFile_WriteError(MemoryFile* memFile);
int __cdecl MemFile_WriteDataInternal(
    MemoryFile* memFile,
    int bytes,
    char nonZeroCount,
    char cacheBufferLen,
    uint8_t nextByte);
int __cdecl MemFile_GetUsedSize(MemoryFile* memFile);
void __cdecl MemFile_WriteData(MemoryFile* memFile, int byteCount, const void* p);
void __cdecl MemFile_WriteCString(MemoryFile* memFile, const char* string);
const char* __cdecl MemFile_ReadCString(MemoryFile* memFile);
void __cdecl MemFile_ReadData(MemoryFile* memFile, int byteCount, uint8_t* p);
uint8_t __cdecl MemFile_ReadByteInternal(MemoryFile* memFile);
void MemFile_Shutdown(MemoryFile *memFile);
uint8_t *MemFile_CopySegments(MemoryFile *memFile, int index, void *buf);
