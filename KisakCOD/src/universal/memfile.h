#pragma once

struct MemoryFile // sizeof=0x1C
{                                       // ...
    unsigned __int8* buffer;            // ...
    int bufferSize;
    int bytesUsed;                      // ...
    int segmentIndex;
    int segmentStart;
    bool errorOnOverflow;
    bool memoryOverflow;
    bool compress;
    // padding byte
    void(__cdecl* archiveProc)(MemoryFile*, int, void*);
};

void __cdecl MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data);