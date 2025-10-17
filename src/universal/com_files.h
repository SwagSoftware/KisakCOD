#pragma once

#include "q_shared.h"

enum FsThread : __int32
{                                       // ...
    FS_THREAD_MAIN = 0x0,
    FS_THREAD_STREAM = 0x1,
    FS_THREAD_DATABASE = 0x2,
    FS_THREAD_BACKEND = 0x3,
    FS_THREAD_SERVER = 0x4,
    FS_THREAD_COUNT = 0x5,
    FS_THREAD_INVALID = 0x6,
};

enum fsMode_t : __int32
{                                       // ...
    FS_READ = 0x0,
    FS_WRITE = 0x1,
    FS_APPEND = 0x2,
    FS_APPEND_SYNC = 0x3,
};

struct directory_t // sizeof=0x200
{
    char path[256];
    char gamedir[256];
};
struct fileInIwd_s // sizeof=0xC
{
    unsigned int pos;
    char* name;
    fileInIwd_s* next;
};
struct iwd_t // sizeof=0x324
{
    char iwdFilename[256];
    char iwdBasename[256];
    char iwdGamename[256];
    unsigned __int8* handle;
    int checksum;
    int pure_checksum;
    volatile unsigned int hasOpenFile;
    int numfiles;
    unsigned __int8 referenced;
    // padding byte
    // padding byte
    // padding byte
    unsigned int hashSize;
    fileInIwd_s** hashTable;
    fileInIwd_s* buildBuffer;
};
struct searchpath_s // sizeof=0x1C
{
    searchpath_s *next;
    iwd_t *iwd;
    directory_t *dir;
    int bLocalized;
    int ignore;
    int ignorePureCheck;
    int language;
};

union qfile_gus // sizeof=0x4
{                                       // ...
    FILE *o;
    unsigned char *z;
};
struct qfile_us // sizeof=0x8
{                                       // ...
    qfile_gus file;
    int iwdIsClone;
};
struct fileHandleData_t // sizeof=0x11C
{
    qfile_us handleFiles;
    int handleSync;
    int fileSize;
    int zipFilePos;
    iwd_t *zipFile;
    int streamed;
    char name[256];
};

bool __cdecl FS_Initialized();

int __cdecl FS_ConditionalRestart(int localClientNum, int checksumFeed);

char *__cdecl FS_ReferencedIwdPureChecksums();
char *__cdecl FS_ReferencedIwdNames();
char *__cdecl FS_ReferencedIwdChecksums();
char *__cdecl FS_LoadedIwdNames();
char *__cdecl FS_LoadedIwdChecksums();
void __cdecl FS_ClearIwdReferences();

char *__cdecl FS_LoadedIwdPureChecksums();
void __cdecl FS_CheckFileSystemStarted();
int __cdecl FS_OpenFileOverwrite(char *qpath);
int __cdecl FS_LoadStack();
int __cdecl FS_HashFileName(const char *fname, int hashSize);
int __cdecl FS_filelength(int f);
void __cdecl FS_ReplaceSeparators(char *path);
void __cdecl FS_BuildOSPath(const char *base, const char *game, const char *qpath, char *ospath);
void __cdecl FS_BuildOSPathForThread(const char *base, const char *game, const char *qpath, char *ospath, FsThread thread);
int __cdecl FS_CreatePath(char *OSPath);
void __cdecl FS_FCloseFile(int h);
void __cdecl FS_FCloseLogFile(int h);
int __cdecl FS_FOpenFileWrite(const char *filename);
int __cdecl FS_FOpenTextFileWrite(const char *filename);
int __cdecl FS_FOpenFileAppend(const char *filename);
unsigned int __cdecl FS_FOpenFileReadStream(const char *filename, int *file);
unsigned int __cdecl FS_FOpenFileReadForThread(const char *filename, int *file, FsThread thread);
int __cdecl FS_FOpenFileReadDatabase(const char *filename, int *file);
unsigned int __cdecl FS_FOpenFileRead(const char *filename, int *file);
bool __cdecl FS_Delete(const char *filename);
int __cdecl FS_FilenameCompare(const char *s1, const char *s2);
unsigned int __cdecl FS_Read(unsigned __int8 *buffer, unsigned int len, int h);
unsigned int __cdecl FS_Write(const char *buffer, unsigned int len, int h);
unsigned int __cdecl FS_WriteLog(const char *buffer, unsigned int len, int h);
void FS_Printf(int h, const char *fmt, ...);
int __cdecl FS_Seek(int f, int offset, int origin);
int __cdecl FS_ReadFile(const char *qpath, void **buffer);
unsigned int *__cdecl FS_AllocMem(int bytes);
void __cdecl FS_ResetFiles();
void __cdecl FS_FreeFile(char *buffer);
void __cdecl FS_FreeMem(char *buffer);
int __cdecl FS_FileExists(char *file);
int __cdecl FS_WriteFile(char *filename, char *buffer, unsigned int size);
void __cdecl FS_ConvertPath(char *s);
void __cdecl FS_InitFilesystem();
unsigned int __cdecl FS_FOpenFileByMode(char *qpath, int *f, fsMode_t mode);
void __cdecl FS_Flush(int f);
void __cdecl FS_FreeFileList(const char **list);

void __cdecl FS_CopyFile(char *fromOSPath, char *toOSPath);

void __cdecl FS_Remove(const char *osPath);

char *__cdecl FS_ShiftStr(const char *string, char shift);
int __cdecl FS_SV_FOpenFileRead(const char *filename, int *fp);
int __cdecl FS_SV_FOpenFileWrite(const char *filename);
void __cdecl FS_SV_Rename(char *from, char *to);
int __cdecl FS_SV_FileExists(char *file);

unsigned int __cdecl FS_FTell(int f);
enum FsListBehavior_e : __int32
{                                       // ...
    FS_LIST_PURE_ONLY = 0x0,
    FS_LIST_ALL = 0x1,
};
int __cdecl FS_GetFileList(
    const char *path,
    const char *extension,
    FsListBehavior_e behavior,
    char *listbuf,
    int bufsize);

const char **__cdecl FS_ListFiles(const char *path, const char *extension, FsListBehavior_e behavior, int *numfiles);
const char **__cdecl FS_ListFilesInLocation(
    const char *path,
    const char *extension,
    FsListBehavior_e behavior,
    int *numfiles,
    int lookInFlags);

void __cdecl FS_FileClose(struct iobuf *stream);
int __cdecl FS_FOpenFileWriteToDirForThread(const char *filename, const char *dir, FsThread thread);
int __cdecl FS_FOpenFileWriteToDir(const char *filename, const char *dir);
int __cdecl FS_WriteFileToDir(const char *filename, const char *path, char *buffer, unsigned int size);

void __cdecl FS_Restart(int localClientNum, int checksumFeed);
bool __cdecl FS_NeedRestart(int checksumFeed);

int __cdecl FS_TouchFile(const char *name);

void FS_RegisterDvars();
void __cdecl FS_Shutdown();
bool __cdecl FS_DeleteInDir(char *filename, char *dir);
void __cdecl FS_Rename(char *from, char *fromDir, char *to, char *toDir);

extern const dvar_t *fs_remotePCDirectory;
extern const dvar_t *fs_remotePCName;
extern const dvar_t *fs_homepath;
extern const dvar_s *fs_debug;
extern const dvar_s *fs_restrict;
extern const dvar_s *fs_ignoreLocalized;
extern const dvar_s *fs_basepath;
extern const dvar_s *fs_copyfiles;
extern const dvar_s *fs_cdpath;
extern const dvar_s *fs_basegame;

extern searchpath_s *fs_searchpaths;

extern int fs_numServerIwds;
extern int fs_serverIwds[1024];
extern int com_fileAccessed;
extern void *g_writeLogEvent;
extern int marker_com_files;
extern void *g_writeLogCompleteEvent;
extern int fs_loadStack;
extern const char *fs_serverIwdNames[1024];

extern int fs_checksumFeed;

extern char fs_gamedir[256];