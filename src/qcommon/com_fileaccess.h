#pragma once
#include <cstdio>
#include <cstdint>

uint32_t __cdecl FS_FileRead(void *ptr, uint32_t len, FILE *stream);
uint32_t __cdecl FS_FileWrite(const void *ptr, uint32_t len, FILE *stream);
FILE *__cdecl FS_FileOpenReadBinary(const char *filename);
FILE *__cdecl FS_FileOpenReadText(const char *filename);
FILE *__cdecl FS_FileOpenWriteBinary(const char *filename);
FILE *__cdecl FS_FileOpenAppendText(const char *filename);
FILE *__cdecl FS_FileOpenWriteText(const char *filename);
FILE *FS_FileOpenWriteReadBinary(const char *filename);
void __cdecl FS_FileClose(FILE *stream);
int __cdecl FS_FileSeek(FILE *file, int offset, int whence);
int __cdecl FileWrapper_Seek(FILE *h, int offset, int origin);
int __cdecl FS_FileGetFileSize(FILE *file);
int __cdecl FileWrapper_GetFileSize(FILE *h);

uint32_t FS_FileTell(FILE *file);