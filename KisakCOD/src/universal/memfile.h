#pragma once

#include <qcommon/qcommon.h>

struct MemoryFile
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

void MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data);

void MemFile_CommonInit(MemoryFile* memFile, int size, byte* buffer, bool errorOnOverflow, bool compress);

void MemFile_InitForReading(MemoryFile* memFile, int size, byte* buffer, bool compress);
void MemFile_InitForWriting(MemoryFile* memFile, int size, byte* buffer, bool errorOnOverflow, bool compress);

bool MemFile_IsReading(MemoryFile* memFile);
bool MemFile_IsWriting(MemoryFile* memFile);

double MemFile_ReadFloat(MemoryFile* memFile);

void __cdecl MemFile_StartSegment(MemoryFile* memFile, int index);
void __cdecl MemFile_deflateInit(unsigned __int8* next_out, unsigned int avail_out, bool compress);
void __cdecl MemFile_EndSegment(MemoryFile* memFile);
unsigned int __cdecl MemFile_deflateEnd(bool compress);
void __cdecl MemFile_MoveToSegment(MemoryFile* memFile, int index);
void __cdecl MemFile_inflateInit(unsigned __int8* next_in, unsigned int len, bool compress);
int __cdecl MemFile_inflateEnd(bool compress);
unsigned __int8* __cdecl MemFile_GetSegmentAddess(MemoryFile* memFile, unsigned int index);
void __cdecl MemFile_WriteError(MemoryFile* memFile);
int __cdecl MemFile_WriteDataInternal(
    MemoryFile* memFile,
    int bytes,
    char nonZeroCount,
    char cacheBufferLen,
    unsigned __int8 nextByte);
int __cdecl MemFile_GetUsedSize(MemoryFile* memFile);
void __cdecl MemFile_WriteData(MemoryFile* memFile, int byteCount, _BYTE* p);
void __cdecl MemFile_WriteCString(MemoryFile* memFile, char* string);
const char* __cdecl MemFile_ReadCString(MemoryFile* memFile);
void __cdecl MemFile_ReadData(MemoryFile* memFile, int byteCount, unsigned __int8* p);
unsigned __int8 __cdecl MemFile_ReadByteInternal(MemoryFile* memFile);
