#include "com_fileaccess.h"
#include "qcommon.h"

int __cdecl FS_FileGetFileSize(FILE *file)
{
    return FileWrapper_GetFileSize(file);
}

unsigned int __cdecl FS_FileRead(void *ptr, unsigned int len, FILE *stream)
{
    unsigned int read_size; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_READ);
    read_size = fread(ptr, 1u, len, stream);
#ifdef _DEBUG
    if (ferror(stream))
    {
        iassert(0);
    }
    //if (feof(stream))
    //{
    //    iassert(0);
    //}
#endif
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_READ);
    return read_size;
}

unsigned int __cdecl FS_FileWrite(const void *ptr, unsigned int len, FILE *stream)
{
    return fwrite(ptr, 1u, len, stream);
}

FILE *__cdecl FS_FileOpenReadBinary(const char *filename)
{
    FILE *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "rb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenReadText(const char *filename)
{
    FILE *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "rt");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenWriteBinary(const char *filename)
{
    FILE *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "wb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenAppendText(const char *filename)
{
    FILE *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "at");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenWriteText(const char *filename)
{
    FILE *file; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = fopen(filename, "w+t");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

int __cdecl FS_FileSeek(FILE *file, int offset, int whence)
{
    int seek; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_SEEK);
    seek = FileWrapper_Seek(file, offset, whence);
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_SEEK);
    return seek;
}

int __cdecl FileWrapper_Seek(FILE *h, int offset, int origin)
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

int __cdecl FileWrapper_GetFileSize(FILE *h)
{
    int startPos; // [esp+0h] [ebp-8h]
    int fileSize; // [esp+4h] [ebp-4h]

    startPos = ftell(h);
    fseek(h, 0, 2);
    fileSize = ftell(h);
    fseek(h, startPos, 0);
    return fileSize;
}

