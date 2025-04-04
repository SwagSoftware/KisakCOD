#pragma once

struct MemoryFile
{
    unsigned char* buffer;
    int bufferSize;
    int bytesUsed;
    int segmentIndex;
    int segmentStart;
    bool errorOnOverflow;
    bool memoryOverflow;
    bool compress;

    void(__cdecl* archiveProc)(MemoryFile*, int, void*);
};

void MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data);