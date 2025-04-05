#pragma once


unsigned int __cdecl FS_FileRead(void *ptr, unsigned int len, iobuf *stream);
unsigned int __cdecl FS_FileWrite(const void *ptr, unsigned int len, iobuf *stream);
iobuf *__cdecl FS_FileOpenReadBinary(const char *filename);
iobuf *__cdecl FS_FileOpenReadText(const char *filename);
iobuf *__cdecl FS_FileOpenWriteBinary(const char *filename);
iobuf *__cdecl FS_FileOpenAppendText(const char *filename);
iobuf *__cdecl FS_FileOpenWriteText(const char *filename);
void __cdecl FS_FileClose(iobuf *stream);
int __cdecl FS_FileSeek(iobuf *file, int offset, int whence);
int __cdecl FileWrapper_Seek(iobuf *h, int offset, int origin);
int __cdecl FS_FileGetFileSize(iobuf *file);
int __cdecl FileWrapper_GetFileSize(iobuf *h);
