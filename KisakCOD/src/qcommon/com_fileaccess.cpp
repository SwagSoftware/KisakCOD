#include "com_fileaccess.h"


unsigned int __cdecl FS_FileRead(void *ptr, unsigned int len, iobuf *stream)
{
    unsigned int read_size; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_READ);
    read_size = fread(ptr, 1u, len, stream);
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_READ);
    return read_size;
}

unsigned int __cdecl FS_FileWrite(const void *ptr, unsigned int len, iobuf *stream)
{
    return fwrite(ptr, 1u, len, stream);
}

iobuf *__cdecl FS_FileOpenReadBinary(const char *filename)
{
    iobuf *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "rb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

iobuf *__cdecl FS_FileOpenReadText(const char *filename)
{
    iobuf *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "rt");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

iobuf *__cdecl FS_FileOpenWriteBinary(const char *filename)
{
    iobuf *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "wb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

iobuf *__cdecl FS_FileOpenAppendText(const char *filename)
{
    iobuf *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "at");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

iobuf *__cdecl FS_FileOpenWriteText(const char *filename)
{
    iobuf *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "w+t");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

void __cdecl FS_FileClose(iobuf *stream)
{
    fclose(stream);
}

int __cdecl FS_FileSeek(iobuf *file, int offset, int whence)
{
    int seek; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_SEEK);
    seek = FileWrapper_Seek(file, offset, whence);
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_SEEK);
    return seek;
}

int __cdecl FileWrapper_Seek(iobuf *h, int offset, int origin)
{
    const char *v4; // eax

    switch (origin)
    {
    case 0:
        return fseek(h, offset, 1);
    case 1:
        return fseek(h, offset, 2);
    case 2:
        return fseek(h, offset, 0);
    }
    if (!alwaysfails)
    {
        v4 = va("Bad origin %i in FS_Seek", origin);
        MyAssertHandler("c:\\trees\\cod3\\src\\qcommon\\../universal/com_files_wrapper_stdio.h", 96, 0, v4);
    }
    return 0;
}

int __cdecl FS_FileGetFileSize(iobuf *file)
{
    return FileWrapper_GetFileSize(file);
}

int __cdecl FileWrapper_GetFileSize(iobuf *h)
{
    int startPos; // [esp+0h] [ebp-8h]
    int fileSize; // [esp+4h] [ebp-4h]

    startPos = ftell(h);
    fseek(h, 0, 2);
    fileSize = ftell(h);
    fseek(h, startPos, 0);
    return fileSize;
}

