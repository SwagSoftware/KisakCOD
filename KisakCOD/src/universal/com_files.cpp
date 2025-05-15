#include "com_files.h"
#include "q_shared.h"

#include <universal/com_memory.h>
#include <qcommon/com_fileaccess.h>
#include <qcommon/qcommon.h>
#include <win32/win_local.h>
#include <qcommon/threads.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/unzip.h>
#include <qcommon/com_bsp.h>
#include <qcommon/cmd.h>
#include <qcommon/files.h>
#include <io.h>

const dvar_t *fs_remotePCDirectory;
const dvar_t *fs_remotePCName;
const dvar_t *fs_homepath;
const dvar_s *fs_debug;
const dvar_s *fs_restrict;
const dvar_s *fs_ignoreLocalized;
const dvar_s *fs_basepath;
const dvar_s *fs_copyfiles;
const dvar_s *fs_cdpath;
const dvar_s *fs_gameDirVar;
const dvar_s *fs_basegame;

int fs_fakeChkSum;

int fs_numServerIwds;
int fs_serverIwds[1024];
int com_fileAccessed;
void *g_writeLogEvent;
int marker_com_files;
void *g_writeLogCompleteEvent;
int fs_loadStack;
const char *fs_serverIwdNames[1024];

int fs_iwdFileCount;
int fs_checksumFeed;
int bLanguagesListed;

static fileHandleData_t fsh[65];

char fs_gamedir[256];
searchpath_s *fs_searchpaths;

char __cdecl FS_SanitizeFilename(const char *filename, char *sanitizedName, int sanitizedNameSize);
BOOL __cdecl FS_UseSearchPath(const searchpath_s *pSearch);
int __cdecl FS_GetHandleAndOpenFile(const char *filename, const char *ospath, FsThread thread);
int __cdecl FS_IwdIsPure(iwd_t *iwd);
const char **__cdecl FS_ListFilteredFiles(
    searchpath_s *searchPath,
    const char *path,
    const char *extension,
    const char *filter,
    FsListBehavior_e behavior,
    int *numfiles);



bool __cdecl FS_Initialized()
{
    return fs_searchpaths != 0;
}

int __cdecl FS_ConditionalRestart(int localClientNum, int checksumFeed)
{
    if (!FS_NeedRestart(checksumFeed))
        return 0;
    FS_Restart(localClientNum, checksumFeed);
    return 1;
}

char info6[8192];
char *__cdecl FS_ReferencedIwdPureChecksums()
{
    char *v0; // eax
    char *v1; // eax
    char *v2; // eax
    int checksum; // [esp+40h] [ebp-Ch]
    searchpath_s *search; // [esp+44h] [ebp-8h]
    int numIwds; // [esp+48h] [ebp-4h]

    info6[0] = 0;
    checksum = fs_checksumFeed;
    numIwds = 0;
    info6[8191] = 0;
    info6[8190] = 0;
    info6[strlen(info6)] = '@';
    info6[strlen(info6)] = ' ';
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd && !search->bLocalized)
        {
            if (search->iwd->referenced)
            {
                v0 = va("%i ", search->iwd->pure_checksum);
                I_strncat(info6, 0x2000, v0);
                checksum ^= search->iwd->pure_checksum;
                ++numIwds;
            }
        }
    }
    if (fs_fakeChkSum)
    {
        v1 = va("%i ", fs_fakeChkSum);
        I_strncat(info6, 0x2000, v1);
    }
    v2 = va("%i ", numIwds ^ checksum);
    I_strncat(info6, 0x2000, v2);
    return info6;
}

BOOL __cdecl FS_PureIgnoresExtension(const char *extension)
{
    if (*extension == 46)
        ++extension;
    return !I_stricmp(extension, "cfg") || I_stricmp(extension, ".dm_NETWORK_PROTOCOL_VERSION") == 0;
}

char __cdecl FS_FilesAreLoadedGlobally(const char *filename)
{
    const char *extensions[8]; // [esp+20h] [ebp-28h]
    int filenameLen; // [esp+40h] [ebp-8h]
    int extensionNum; // [esp+44h] [ebp-4h]

    extensions[0] = ".hlsl";
    extensions[1] = ".txt";
    extensions[2] = ".cfg";
    extensions[3] = ".levelshots";
    extensions[4] = ".menu";
    extensions[5] = ".arena";
    extensions[6] = ".str";
    extensions[7] = "";
    filenameLen = strlen(filename);
    for (extensionNum = 0; *extensions[extensionNum]; ++extensionNum)
    {
        if (!I_stricmp(&filename[filenameLen - strlen(extensions[extensionNum])], extensions[extensionNum]))
            return 1;
    }
    return 0;
}

char info8[8192];
char *__cdecl FS_ReferencedIwdNames()
{
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info8[0] = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd && (search->iwd->referenced || I_strnicmp(search->iwd->iwdGamename, "main", 4)))
        {
            if (info8[0])
                I_strncat(info8, 0x2000, " ");
            I_strncat(info8, 0x2000, search->iwd->iwdGamename);
            I_strncat(info8, 0x2000, "/");
            I_strncat(info8, 0x2000, search->iwd->iwdBasename);
        }
    }
    return info8;
}

char info5[8192];
char *__cdecl FS_ReferencedIwdChecksums()
{
    char *v0; // eax
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info5[0] = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd && (search->iwd->referenced || I_strnicmp(search->iwd->iwdGamename, "main", 4)))
        {
            v0 = va("%i ", search->iwd->checksum);
            I_strncat(info5, 0x2000, v0);
        }
    }
    return info5;
}

char info3[8192];
char *__cdecl FS_LoadedIwdNames()
{
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info3[0] = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd && !search->bLocalized)
        {
            if (info3[0])
                I_strncat(info3, 0x2000, " ");
            I_strncat(info3, 0x2000, search->iwd->iwdBasename);
        }
    }
    return info3;
}

char info2_0[8192];
char *__cdecl FS_LoadedIwdChecksums()
{
    char *v0; // eax
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info2_0[0] = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd)
        {
            if (!search->bLocalized)
            {
                v0 = va("%i ", search->iwd->checksum);
                I_strncat(info2_0, 0x2000, v0);
            }
        }
    }
    return info2_0;
}

void __cdecl FS_ClearIwdReferences()
{
    searchpath_s *search; // [esp+0h] [ebp-4h]

    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd)
            search->iwd->referenced = 0;
    }
}

char info4[8192];
char *__cdecl FS_LoadedIwdPureChecksums()
{
    char *v0; // eax
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info4[0] = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->iwd)
        {
            if (!search->bLocalized)
            {
                v0 = va("%i ", search->iwd->pure_checksum);
                I_strncat(info4, 0x2000, v0);
            }
        }
    }
    return info4;
}

void __cdecl FS_CheckFileSystemStarted()
{
    if (!fs_searchpaths)
        MyAssertHandler(".\\universal\\com_files.cpp", 708, 0, "%s", "fs_searchpaths");
}

int __cdecl FS_GetFileOsPath(const char *filename, char *ospath)
{
    char sanitizedName[256]; // [esp+0h] [ebp-110h] BYREF
    directory_t *dir; // [esp+104h] [ebp-Ch]
    searchpath_s *search; // [esp+108h] [ebp-8h]
    FILE *fp; // [esp+10Ch] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 2932, 0, "%s", "filename");
    if (!ospath)
        MyAssertHandler(".\\universal\\com_files.cpp", 2933, 0, "%s", "ospath");
    if (!FS_SanitizeFilename(filename, sanitizedName, 256))
        return -1;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (FS_UseSearchPath(search))
        {
            if (!search->iwd)
            {
                dir = search->dir;
                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, ospath, FS_THREAD_MAIN);
                fp = FS_FileOpenReadBinary(ospath);
                if (fp)
                {
                    FS_FileClose(fp);
                    return 0;
                }
            }
        }
    }
    return -1;
}

int __cdecl FS_OpenFileOverwrite(char *qpath)
{
    DWORD oldAttributes; // [esp+0h] [ebp-10Ch]
    char ospath[256]; // [esp+4h] [ebp-108h] BYREF
    unsigned int attributes; // [esp+108h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!qpath)
        MyAssertHandler(".\\universal\\com_files.cpp", 2971, 0, "%s", "qpath");
    if (FS_GetFileOsPath(qpath, ospath) >= 0)
    {
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_FOpenFileOverWrite: %s\n", ospath);
        oldAttributes = GetFileAttributesA(ospath);
        attributes = oldAttributes & 0xFFFFFFFE;
        if ((oldAttributes & 0xFFFFFFFE) != oldAttributes)
            SetFileAttributesA(ospath, attributes);
        return FS_GetHandleAndOpenFile(qpath, ospath, FS_THREAD_MAIN);
    }
    else
    {
        Com_Error(
            ERR_DROP,
            "FS_FOpenFileOverWrite: Failed to open %s for writing.  It either does not exist or is in a iwd file.",
            qpath);
        return 0;
    }
}

int __cdecl FS_LoadStack()
{
    return fs_loadStack;
}

int __cdecl FS_HashFileName(const char *fname, int hashSize)
{
    int hash; // [esp+0h] [ebp-Ch]
    int letter; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    hash = 0;
    for (i = 0; fname[i]; ++i)
    {
        letter = tolower(fname[i]);
        if (letter == 46)
            break;
        if (letter == 92)
            letter = 47;
        hash += letter * (i + 119);
    }
    return ((hash >> 20) ^ hash ^ (hash >> 10)) & (hashSize - 1);
}

FILE *__cdecl FS_FileForHandle(int f)
{
    if (f <= 0 || f >= 65)
        MyAssertHandler(
            ".\\universal\\com_files.cpp",
            952,
            0,
            "%s\n\t(f) = %i",
            "(f > 0 && f < (1 + 48 + 13 + 1 + 1 + 1))",
            f);
    if (fsh[f].zipFile)
        MyAssertHandler(".\\universal\\com_files.cpp", 953, 0, "%s", "!fsh[f].zipFile");
    if (!fsh[f].handleFiles.file.o)
        MyAssertHandler(".\\universal\\com_files.cpp", 954, 0, "%s", "fsh[f].handleFiles.file.o");
    return fsh[f].handleFiles.file.o;
}

int __cdecl FS_filelength(int f)
{
    FILE *h; // [esp+4h] [ebp-4h]

    if (!f)
        MyAssertHandler(".\\universal\\com_files.cpp", 968, 0, "%s", "f");
    FS_CheckFileSystemStarted();

    if (fsh[f].zipFile)
    {
        unz_s *zfi = (unz_s *)fsh[f].handleFiles.file.o;
        return zfi->cur_file_info.uncompressed_size;
    }

    h = FS_FileForHandle(f);
    return FS_FileGetFileSize(h);
}

void __cdecl FS_ReplaceSeparators(char *path)
{
    char *src; // [esp+0h] [ebp-Ch]
    char *dst; // [esp+4h] [ebp-8h]
    bool wasSep; // [esp+Bh] [ebp-1h]

    wasSep = 0;
    src = path;
    dst = path;
    while (*src)
    {
        if (*src == 47 || *src == 92)
        {
            if (!wasSep)
            {
                wasSep = 1;
                *dst++ = 92;
            }
        }
        else
        {
            wasSep = 0;
            *dst++ = *src;
        }
        ++src;
    }
    *dst = 0;
}

void __cdecl FS_BuildOSPath(const char *base, const char *game, const char *qpath, char *ospath)
{
    FS_BuildOSPathForThread(base, game, qpath, ospath, FS_THREAD_MAIN);
}

void __cdecl FS_BuildOSPathForThread(const char *base, const char *game, const char *qpath, char *ospath, FsThread thread)
{
    unsigned int v5; // [esp+0h] [ebp-3Ch]
    unsigned int v6; // [esp+10h] [ebp-2Ch]
    unsigned int v7; // [esp+20h] [ebp-1Ch]

    if (!base)
        MyAssertHandler(".\\universal\\com_files.cpp", 1049, 0, "%s", "base");
    if (!qpath)
        MyAssertHandler(".\\universal\\com_files.cpp", 1050, 0, "%s", "qpath");
    if (!ospath)
        MyAssertHandler(".\\universal\\com_files.cpp", 1051, 0, "%s", "ospath");
    if (!game || !*game)
        game = fs_gamedir;
    v7 = strlen(base);
    v6 = strlen(game);
    v5 = strlen(qpath);
    if ((int)(v7 + v6 + 1 + v5 + 1) >= 256)
    {
        if (thread)
        {
            *ospath = 0;
            return;
        }
        Com_Error(ERR_FATAL, "FS_BuildOSPath: os path length exceeded");
    }
    memcpy((unsigned __int8 *)ospath, (unsigned __int8 *)base, v7);
    ospath[v7] = 47;
    memcpy((unsigned __int8 *)&ospath[v7 + 1], (unsigned __int8 *)game, v6);
    ospath[v7 + 1 + v6] = 47;
    memcpy((unsigned __int8 *)&ospath[v7 + 2 + v6], (unsigned __int8 *)qpath, v5 + 1);
    FS_ReplaceSeparators(ospath);
}

int __cdecl FS_CreatePath(char *OSPath)
{
    const char *v1; // eax
    int v2; // eax
    char *ofs; // [esp+0h] [ebp-4h]

    v1 = strstr(OSPath, "..");
    if (v1 || (strstr(OSPath, "::")))
    {
        Com_PrintWarning(10, "WARNING: refusing to create relative path \"%s\"\n", OSPath);
        return 1;
    }
    else
    {
        for (ofs = OSPath + 1; *ofs; ++ofs)
        {
            if (*ofs == 92)
            {
                *ofs = 0;
                Sys_Mkdir(OSPath);
                *ofs = 92;
            }
        }
        return 0;
    }
}

void __cdecl FS_FileClose(FILE *stream)
{
    fclose(stream);
}

void __cdecl FS_FCloseFile(int h)
{
    FILE *f; // [esp+0h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (fsh[h].streamed)
        MyAssertHandler(".\\universal\\com_files.cpp", 1275, 0, "%s", "!fsh[h].streamed");
    if (fsh[h].zipFile)
    {
        unzCloseCurrentFile(fsh[h].handleFiles.file.z);
        if (fsh[h].handleFiles.iwdIsClone)
        {
            unzClose(fsh[h].handleFiles.file.z);
        }
        else
        {
            if (!fsh[h].zipFile->hasOpenFile)
                MyAssertHandler(".\\universal\\com_files.cpp", 1287, 0, "%s", "fsh[h].zipFile->hasOpenFile");
            fsh[h].zipFile->hasOpenFile = 0;
        }
    }
    else if (h)
    {
        f = FS_FileForHandle(h);
        FS_FileClose(f);
    }
    Com_Memset((unsigned int *)&fsh[h], 0, 284);
}

void __cdecl FS_FCloseLogFile(int h)
{
    FS_FCloseFile(h);
}

int __cdecl FS_FOpenFileWrite(const char *filename)
{
    return FS_FOpenFileWriteToDirForThread(filename, fs_gamedir, FS_THREAD_MAIN);
}

int __cdecl FS_HandleForFile(FsThread thread)
{
    int i; // [esp+8h] [ebp-Ch]
    int ia; // [esp+8h] [ebp-Ch]
    int count; // [esp+Ch] [ebp-8h]
    int first; // [esp+10h] [ebp-4h]

    if (thread)
    {
        if (thread == FS_THREAD_STREAM)
        {
            first = 49;
            count = 13;
        }
        else if (thread == FS_THREAD_BACKEND)
        {
            first = 63;
            count = 1;
        }
        else
        {
            if (thread != FS_THREAD_DATABASE)
                MyAssertHandler(".\\universal\\com_files.cpp", 920, 0, "thread == FS_THREAD_DATABASE\n\t%i, %i", thread, 2);
            if (useFastFile->current.enabled)
            {
                if (!Sys_IsDatabaseThread())
                    MyAssertHandler(".\\universal\\com_files.cpp", 921, 0, "%s", "Sys_IsDatabaseThread()");
            }
            else if (!Sys_IsMainThread())
            {
                MyAssertHandler(".\\universal\\com_files.cpp", 921, 0, "%s", "Sys_IsMainThread()");
            }
            first = 62;
            count = 1;
        }
    }
    else
    {
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\universal\\com_files.cpp", 897, 0, "%s", "Sys_IsMainThread()");
        first = 1;
        count = 48;
    }
    for (i = 0; i < count; ++i)
    {
        if (!fsh[i + first].handleFiles.file.o)
            return i + first;
    }
    if (thread == FS_THREAD_MAIN)
    {
        for (ia = 1; ia < 65; ++ia)
            Com_Printf(10, "FILE %2i: '%s' 0x%x\n", ia, fsh[ia].name, fsh[ia].handleFiles.file.o);
        Com_Error(ERR_DROP, "FS_HandleForFile: none free");
    }
    Com_PrintWarning(10, "FILE %2i: '%s' 0x%x\n", first, fsh[first].name, fsh[first].handleFiles.file.o);
    Com_PrintWarning(10, "FS_HandleForFile: none free (%d)\n", thread);
    return 0;
}

int __cdecl FS_FOpenTextFileWrite(const char *filename)
{
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    FILE *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    FS_CheckFileSystemStarted();
    h = FS_HandleForFile(FS_THREAD_MAIN);
    fsh[h].zipFile = 0;
    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenWriteText(ospath);
    fsh[h].handleFiles.file.o = f;
    I_strncpyz(fsh[h].name, filename, 256);
    fsh[h].handleSync = 0;
    if (!fsh[h].handleFiles.file.o)
        return 0;
    return h;
}

int __cdecl FS_FOpenFileAppend(const char *filename)
{
    bool IsMainThread; // al
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    FILE *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\universal\\com_files.cpp", 1486, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    FS_CheckFileSystemStarted();
    IsMainThread = Sys_IsMainThread();
    h = FS_HandleForFile(IsMainThread ? FS_THREAD_MAIN : FS_THREAD_BACKEND);
    fsh[h].zipFile = 0;
    I_strncpyz(fsh[h].name, filename, 256);
    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileAppend: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenAppendText(ospath);
    fsh[h].handleFiles.file.o = f;
    fsh[h].handleSync = 0;
    if (!fsh[h].handleFiles.file.o)
        return 0;
    return h;
}

unsigned int __cdecl FS_FOpenFileReadStream(const char *filename, int *file)
{
    return FS_FOpenFileReadForThread(filename, file, FS_THREAD_STREAM);
}

bool __cdecl FS_IsBackupSubStr(const char *filenameSubStr)
{
    if (*filenameSubStr == '.' && filenameSubStr[1] == '.')
        return 1;
    return *filenameSubStr == ':' && filenameSubStr[1] == ':';
}

char __cdecl FS_SanitizeFilename(const char *filename, char *sanitizedName, int sanitizedNameSize)
{
    const char *v4; // eax
    char v7; // [esp+8h] [ebp-14h]
    char v9; // [esp+10h] [ebp-Ch]
    char v10; // [esp+11h] [ebp-Bh]
    char v11; // [esp+12h] [ebp-Ah]
    char v12; // [esp+13h] [ebp-9h]
    int srcIndex; // [esp+14h] [ebp-8h]
    int dstIndex; // [esp+18h] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 1707, 0, "%s", "filename");
    if (!sanitizedName)
        MyAssertHandler(".\\universal\\com_files.cpp", 1708, 0, "%s", "sanitizedName");
    if (sanitizedNameSize <= 0)
        MyAssertHandler(
            ".\\universal\\com_files.cpp",
            1709,
            0,
            "%s\n\t(sanitizedNameSize) = %i",
            "(sanitizedNameSize > 0)",
            sanitizedNameSize);
    for (srcIndex = 0; ; ++srcIndex)
    {
        v12 = filename[srcIndex];
        if (v12 != 47 && v12 != 92)
            break;
    }
    dstIndex = 0;
    while (filename[srcIndex])
    {
        if (FS_IsBackupSubStr(&filename[srcIndex]))
            return 0;
        if (filename[srcIndex] != 46
            || filename[srcIndex + 1] && ((v11 = filename[srcIndex + 1], v11 == 47) || v11 == 92 ? (v7 = 1) : (v7 = 0), !v7))
        {
            if (dstIndex + 1 >= sanitizedNameSize)
            {
                v4 = va("%i + 1 > %i", dstIndex, sanitizedNameSize);
                MyAssertHandler(".\\universal\\com_files.cpp", 1725, 0, "%s\n\t%s", "dstIndex + 1 < sanitizedNameSize", v4);
                return 0;
            }
            v10 = filename[srcIndex];
            if (v10 == 47 || v10 == 92)
            {
                sanitizedName[dstIndex] = 47;
                while (1)
                {
                    v9 = filename[srcIndex + 1];
                    if (v9 != 47 && v9 != 92)
                        break;
                    ++srcIndex;
                }
            }
            else
            {
                sanitizedName[dstIndex] = filename[srcIndex];
            }
            ++dstIndex;
        }
        ++srcIndex;
    }
    if (dstIndex > srcIndex)
        MyAssertHandler(".\\universal\\com_files.cpp", 1740, 1, "dstIndex <= srcIndex\n\t%i, %i", dstIndex, srcIndex);
    sanitizedName[dstIndex] = 0;
    return 1;
}

BOOL __cdecl FS_UseSearchPath(const searchpath_s *pSearch)
{
    if (pSearch->bLocalized && fs_ignoreLocalized->current.enabled)
        return 0;
    return !pSearch->bLocalized || pSearch->language == SEH_GetCurrentLanguage();
}

int __cdecl FS_FilenameCompare(const char *s1, const char *s2)
{
    int c2; // [esp+0h] [ebp-8h]
    int c1; // [esp+4h] [ebp-4h]

    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if (I_islower(c1))
            c1 -= ' ';
        if (I_islower(c2))
            c2 -= ' ';
        if (c1 == '\\' || c1 == ':')
            c1 = '/';
        if (c2 == '\\' || c2 == ':')
            c2 = '/';
        if (c1 != c2)
            return -1;
    } while (c1);
    return 0;
}

unsigned int __cdecl FS_FOpenFileReadForThread(const char *filename, int *file, FsThread thread)
{
    char *v4; // eax
    const char *v5; // eax
    const char *v6; // [esp-4h] [ebp-334h]
    char copypath[256]; // [esp+0h] [ebp-330h] BYREF
    char sanitizedName[256]; // [esp+100h] [ebp-230h] BYREF
    fileInIwd_s *iwdFile; // [esp+200h] [ebp-130h]
    directory_t *dir; // [esp+204h] [ebp-12Ch]
    int hash; // [esp+208h] [ebp-128h]
    unz_s *zfi; // [esp+20Ch] [ebp-124h]
    const char *impureIwd; // [esp+210h] [ebp-120h]
    iwd_t *iwd; // [esp+214h] [ebp-11Ch]
    const char *extension; // [esp+218h] [ebp-118h]
    file_in_zip_read_info_s *ziptemp; // [esp+21Ch] [ebp-114h]
    char netpath[256]; // [esp+220h] [ebp-110h] BYREF
    bool wasSkipped; // [esp+327h] [ebp-9h]
    searchpath_s *search; // [esp+328h] [ebp-8h]
    FILE *filetemp; // [esp+32Ch] [ebp-4h]

    impureIwd = 0;
    wasSkipped = 0;
    hash = 0;
    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 1799, 0, "%s", "filename");
    FS_CheckFileSystemStarted();
    if (!FS_SanitizeFilename(filename, sanitizedName, 256))
    {
        if (file)
            *file = 0;
        return -1;
    }
    if (!file)
    {
        for (search = fs_searchpaths; ; search = search->next)
        {
            if (!search)
                return -1;
            if (FS_UseSearchPath(search))
            {
                if (search->iwd)
                    hash = FS_HashFileName(sanitizedName, search->iwd->hashSize);
                if (search->iwd && search->iwd->hashTable[hash])
                {
                    iwd = search->iwd;
                    iwdFile = iwd->hashTable[hash];
                    while (FS_FilenameCompare(iwdFile->name, sanitizedName))
                    {
                        iwdFile = iwdFile->next;
                        if (!iwdFile)
                            goto LABEL_9;
                    }
                    return 1;
                }
                if (search->dir)
                {
                    dir = search->dir;
                    FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                    filetemp = FS_FileOpenReadBinary(netpath);
                    if (filetemp)
                    {
                        FS_FileClose(filetemp);
                        return 1;
                    }
                }
            }
        LABEL_9:
            ;
        }
    }
    *file = FS_HandleForFile(thread);
    if (!*file)
        return -1;
    for (search = fs_searchpaths; ; search = search->next)
    {
        if (!search)
        {
            if (fs_debug->current.integer && thread == FS_THREAD_MAIN)
                Com_Printf(10, "Can't find %s\n", filename);
            *file = 0;
            if (impureIwd)
            {
                v6 = impureIwd;
                v4 = SEH_SafeTranslateString((char*)"EXE_UNPURECLIENTDETECTED");
                v5 = va("%s %s", v4, v6);
                Com_Error(ERR_DROP, v5);
            }
            if (!wasSkipped)
                return -1;
            if (fs_numServerIwds || fs_restrict->current.enabled)
                Com_Printf(10, "Error: %s must be in an IWD\n", filename);
            else
                Com_Printf(10, "Error: %s must be in an IWD or not in the main directory\n", filename);
            return -2;
        }
        if (FS_UseSearchPath(search))
            break;
    LABEL_29:
        ;
    }
    iwd = search->iwd;
    if (iwd)
        hash = FS_HashFileName(sanitizedName, iwd->hashSize);
    if (!iwd || !iwd->hashTable[hash])
    {
        if (search->dir)
        {
            extension = Com_GetExtensionSubString(sanitizedName);
            if (!search->ignore && !fs_restrict->current.enabled && !fs_numServerIwds
                || search->bLocalized
                || search->ignorePureCheck
                || FS_PureIgnoresExtension(extension))
            {
                dir = search->dir;
                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                fsh[*file].handleFiles.file.o = FS_FileOpenReadBinary(netpath);
                if (fsh[*file].handleFiles.file.o)
                {
                    if (!search->bLocalized && !search->ignorePureCheck && !FS_PureIgnoresExtension(extension))
                        fs_fakeChkSum = rand() + 1;
                    I_strncpyz(fsh[*file].name, sanitizedName, 256);
                    fsh[*file].zipFile = 0;
                    if (fs_debug->current.integer && thread == FS_THREAD_MAIN)
                        Com_Printf(10, "FS_FOpenFileRead: %s (found in '%s/%s')\n", sanitizedName, dir->path, dir->gamedir);
                    if (fs_copyfiles->current.enabled && !I_stricmp(dir->path, fs_cdpath->current.string))
                    {
                        FS_BuildOSPathForThread((char*)fs_basepath->current.integer, dir->gamedir, sanitizedName, copypath, thread);
                        FS_CopyFile(netpath, copypath);
                    }
                    return FS_filelength(*file);
                }
            }
            else if (!wasSkipped)
            {
                dir = search->dir;
                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                filetemp = FS_FileOpenReadBinary(netpath);
                if (filetemp)
                {
                    wasSkipped = 1;
                    FS_FileClose(filetemp);
                }
            }
        }
        goto LABEL_29;
    }
    iwdFile = iwd->hashTable[hash];
    while (FS_FilenameCompare(iwdFile->name, sanitizedName))
    {
        iwdFile = iwdFile->next;
        if (!iwdFile)
            goto LABEL_29;
    }
    if (!search->bLocalized && !search->ignorePureCheck && !FS_IwdIsPure(iwd))
    {
        impureIwd = (const char *)iwd;
        goto LABEL_29;
    }
    if (!iwd->referenced && !FS_FilesAreLoadedGlobally(sanitizedName))
        iwd->referenced = 1;
    if (InterlockedCompareExchange(&iwd->hasOpenFile, 1, 0) == 1)
    {
        fsh[*file].handleFiles.iwdIsClone = 1;
        fsh[*file].handleFiles.file.o = (FILE*)unzReOpen(iwd->iwdFilename, iwd->handle);
        if (!fsh[*file].handleFiles.file.o)
        {
            if (thread)
            {
                FS_FCloseFile(*file);
                *file = 0;
                return -1;
            }
            Com_Error(ERR_FATAL, "Couldn't reopen %s", iwd);
        }
    }
    else
    {
        fsh[*file].handleFiles.iwdIsClone = 0;
        fsh[*file].handleFiles.file.o = (FILE*)iwd->handle;
    }
    I_strncpyz(fsh[*file].name, sanitizedName, 256);
    fsh[*file].zipFile = iwd;
    zfi = (unz_s *)fsh[*file].handleFiles.file.o;
    filetemp = zfi->file;
    ziptemp = zfi->pfile_in_zip_read;
    unzSetCurrentFileInfoPosition(iwd->handle, iwdFile->pos);
    Com_Memcpy((char *)zfi, (char *)iwd->handle, 128);
    zfi->file = filetemp;
    zfi->pfile_in_zip_read = ziptemp;
    unzOpenCurrentFile(fsh[*file].handleFiles.file.z);
    fsh[*file].zipFilePos = iwdFile->pos;
    if (fs_debug->current.integer && thread == FS_THREAD_MAIN)
        Com_Printf(10, "FS_FOpenFileRead: %s (found in '%s')\n", sanitizedName, iwd->iwdFilename);
    return zfi->cur_file_info.uncompressed_size;
}

unsigned int __cdecl FS_FOpenFileReadDatabase(const char *filename, int *file)
{
    return FS_FOpenFileReadForThread(filename, file, FS_THREAD_DATABASE);
}

unsigned int __cdecl FS_FOpenFileRead(const char *filename, int *file)
{
    com_fileAccessed = 1;
    return FS_FOpenFileReadForThread(filename, file, FS_THREAD_MAIN);
}

bool __cdecl FS_Delete(const char *filename)
{
    char ospath[260]; // [esp+0h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 2205, 0, "%s", "filename");
    if (!*filename)
        return 0;
    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, filename, ospath);
    return remove(ospath) != -1;
}

unsigned int __cdecl FS_Read(unsigned __int8 *buffer, unsigned int len, int h)
{
    int tries; // [esp+4h] [ebp-14h]
    unsigned int remaining; // [esp+8h] [ebp-10h]
    unsigned __int8 *buf; // [esp+Ch] [ebp-Ch]
    FILE *f; // [esp+10h] [ebp-8h]
    int read; // [esp+14h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!h)
        return 0;
    if (fsh[h].zipFile)
        return unzReadCurrentFile(fsh[h].handleFiles.file.z, buffer, len);
    f = FS_FileForHandle(h);
    buf = buffer;
    remaining = len;
    tries = 0;
    while (remaining)
    {
        read = FS_FileRead(buf, remaining, f);
        if (!read)
        {
            if (tries)
                return len - remaining;
            tries = 1;
        }
        if (read == -1)
        {
            if (h >= 49 && h < 62)
                return -1;
            Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
        }
        remaining -= read;
        buf += read;
    }
    return len;
}

unsigned int __cdecl FS_Write(const char *buffer, unsigned int len, int h)
{
    int tries; // [esp+4h] [ebp-14h]
    unsigned int remaining; // [esp+8h] [ebp-10h]
    int written; // [esp+10h] [ebp-8h]
    FILE *f; // [esp+14h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!h)
        return 0;
    f = FS_FileForHandle(h);
    remaining = len;
    tries = 0;
    while (remaining)
    {
        written = FS_FileWrite(buffer, remaining, f);
        if (!written)
        {
            if (tries)
                return 0;
            tries = 1;
        }
        if (written == -1)
            return 0;
        remaining -= written;
        buffer += written;
    }
    if (fsh[h].handleSync)
        fflush(f);
    return len;
}

unsigned int __cdecl FS_WriteLog(const char *buffer, unsigned int len, int h)
{
    return FS_Write(buffer, len, h);
}

void FS_Printf(int h, const char *fmt, ...)
{
    char string[4100]; // [esp+14h] [ebp-1008h] BYREF
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    _vsnprintf(string, 0x1000u, fmt, va);
    FS_Write(string, &string[strlen(string) + 1] - &string[1], h);
}

int __cdecl FS_Seek(int f, int offset, int origin)
{
    unsigned int CurrentFile; // eax
    const char *v5; // eax
    FILE*v6; // eax
    signed int iZipPos; // [esp+8h] [ebp-8h]
    unsigned int iZipOffset; // [esp+Ch] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (fsh[f].streamed)
        MyAssertHandler(".\\universal\\com_files.cpp", 2647, 0, "%s", "!fsh[f].streamed");
    if (!fsh[f].zipFile)
    {
        v6 = FS_FileForHandle(f);
        return FS_FileSeek(v6, offset, origin);
    }
    if (!offset && origin == 2)
    {
        unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
        return unzOpenCurrentFile(fsh[f].handleFiles.file.z);
    }
    if (!offset && !origin)
        return 0;
    iZipPos = unztell(fsh[f].handleFiles.file.z);
    switch (origin)
    {
    case 0:
        if (!offset)
            MyAssertHandler(".\\universal\\com_files.cpp", 2668, 0, "%s", "offset != 0");
        if (offset >= 0)
        {
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset);
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset + iZipPos);
        }
        goto LABEL_28;
    case 1:
        if (offset + FS_filelength(f) >= iZipPos)
        {
            iZipOffset = offset + FS_filelength(f) - iZipPos;
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            iZipOffset = offset + FS_filelength(f);
        }
        CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, iZipOffset);
        goto LABEL_28;
    case 2:
        if (offset >= iZipPos)
        {
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset - iZipPos);
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset);
        }
    LABEL_28:
        if (CurrentFile)
            return 0;
        else
            return -1;
    }
    if (!alwaysfails)
    {
        v5 = va("Bad origin %i in FS_Seek", origin);
        MyAssertHandler(".\\universal\\com_files.cpp", 2712, 0, v5);
    }
    return -1;
}

int __cdecl FS_ReadFile(const char *qpath, void **buffer)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    int h; // [esp+8h] [ebp-4h] BYREF

    FS_CheckFileSystemStarted();
    if (!qpath || !*qpath)
        Com_Error(ERR_FATAL, "FS_ReadFile with empty name");
    len = FS_FOpenFileRead(qpath, &h);
    if (h)
    {
        if (buffer)
        {
            ++fs_loadStack;
            buf = (unsigned __int8 *)FS_AllocMem(len + 1);
            *buffer = buf;
            FS_Read(buf, len, h);
            buf[len] = 0;
        }
        FS_FCloseFile(h);
        return len;
    }
    else
    {
        if (buffer)
            *buffer = 0;
        return -1;
    }
}

unsigned int *__cdecl FS_AllocMem(int bytes)
{
    return Hunk_AllocateTempMemory(bytes, "FS_AllocMem");
}

void __cdecl FS_ResetFiles()
{
    fs_loadStack = 0;
}

void __cdecl FS_FreeFile(char *buffer)
{
    FS_CheckFileSystemStarted();
    if (!buffer)
        MyAssertHandler(".\\universal\\com_files.cpp", 2840, 0, "%s", "buffer");
    --fs_loadStack;
    FS_FreeMem(buffer);
}

void __cdecl FS_FreeMem(char *buffer)
{
    Hunk_FreeTempMemory(buffer);
}

int __cdecl FS_FileExists(char *file)
{
    FILE *f; // [esp+0h] [ebp-10Ch]
    char testpath[260]; // [esp+4h] [ebp-108h] BYREF

    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, file, testpath);
    f = FS_FileOpenReadBinary(testpath);
    if (!f)
        return 0;
    FS_FileClose(f);
    return 1;
}

int __cdecl FS_WriteFile(char *filename, char *buffer, unsigned int size)
{
    int f; // [esp+0h] [ebp-8h]
    unsigned int actualSize; // [esp+4h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 2896, 0, "%s", "filename");
    if (!buffer)
        MyAssertHandler(".\\universal\\com_files.cpp", 2897, 0, "%s", "buffer");
    f = FS_FOpenFileWrite(filename);
    if (f)
    {
        actualSize = FS_Write(buffer, size, f);
        FS_FCloseFile(f);
        if (actualSize == size)
        {
            return 1;
        }
        else
        {
            FS_Delete(filename);
            return 0;
        }
    }
    else
    {
        Com_Printf(10, "Failed to open %s\n", filename);
        return 0;
    }
}

void __cdecl FS_ConvertPath(char *s)
{
    while (*s)
    {
        if (*s == 92 || *s == 58)
            *s = 47;
        ++s;
    }
}

bool __cdecl FS_GameDirDomainFunc(dvar_s *dvar, DvarValue newValue)
{
    bool result; // al
    int v3; // eax
    int v4; // eax

    if (!dvar)
        MyAssertHandler(".\\universal\\com_files.cpp", 4241, 0, "%s", "dvar");
    if (!*(_BYTE *)newValue.integer)
        return 1;
    if (I_strnicmp(newValue.string, "mods", 4))
        return 0;
    if (strlen(newValue.string) < 6 || *(_BYTE *)(newValue.integer + 4) != 47 && *(_BYTE *)(newValue.integer + 4) != 92)
        return 0;
    v3 = (int)strstr((char*)newValue.integer, "..");
    result = 0;
    if (!v3)
    {
        v4 = (int)strstr((char*)newValue.integer, "::");
        if (!v4)
            return 1;
    }
    return result;
}

void FS_RegisterDvars()
{
    char *v1; // eax
    char *v2; // eax
    const dvar_s *result; // eax
    char *homePath; // [esp+0h] [ebp-4h]

    fs_debug = Dvar_RegisterInt("fs_debug", 0, (DvarLimits)0x200000000LL, 0, "Enable file system debugging information");
    fs_copyfiles = Dvar_RegisterBool("fs_copyfiles", 0, 0x10u, "Copy all used files to another location");
    v1 = (char *)Sys_DefaultCDPath();
    fs_cdpath = Dvar_RegisterString("fs_cdpath", v1, 0x10u, "CD path");
    v2 = Sys_Cwd();
    fs_basepath = Dvar_RegisterString("fs_basepath", v2, 0x210u, "Base game path");
    fs_basegame = Dvar_RegisterString("fs_basegame", (char *)"", 0x10u, "Base game name");
    fs_gameDirVar = Dvar_RegisterString(
        "fs_game",
        (char *)"",
        0x1Cu,
        "Game data directory. Must be \"\" or a sub directory of 'mods/'.");
    Dvar_SetDomainFunc((dvar_s *)fs_gameDirVar, FS_GameDirDomainFunc);
    fs_ignoreLocalized = Dvar_RegisterBool("fs_ignoreLocalized", 0, 0xA0u, "Ignore localized files");
    homePath = (char *)RETURN_ZERO32();
    if (!homePath || !*homePath)
        homePath = (char *)fs_basepath->reset.integer;
    fs_homepath = Dvar_RegisterString("fs_homepath", homePath, 0x210u, "Game home path");
    fs_restrict = Dvar_RegisterBool("fs_restrict", 0, 0x10u, "Restrict file access for demos etc.");
}

void __cdecl FS_AddSearchPath(searchpath_s *search)
{
    searchpath_s **pSearch; // [esp+0h] [ebp-4h]

    pSearch = &fs_searchpaths;
    if (search->bLocalized)
    {
        while (*pSearch && !(*pSearch)->bLocalized)
            pSearch = (searchpath_s **)*pSearch;
    }
    search->next = *pSearch;
    *pSearch = search;
}

iwd_t *__cdecl FS_LoadZipFile(char *zipfile, char *basename)
{
    char v3; // [esp+17h] [ebp-1DDh]
    char *name; // [esp+1Ch] [ebp-1D8h]
    char *v5; // [esp+20h] [ebp-1D4h]
    unsigned __int8 *uf; // [esp+70h] [ebp-184h]
    char *namePtr; // [esp+74h] [ebp-180h]
    int hash; // [esp+7Ch] [ebp-178h]
    int fs_numHeaderLongs; // [esp+80h] [ebp-174h]
    int *fs_headerLongs; // [esp+84h] [ebp-170h]
    iwd_t *iwd; // [esp+88h] [ebp-16Ch]
    unsigned int len; // [esp+8Ch] [ebp-168h]
    fileInIwd_s *buildBuffer; // [esp+90h] [ebp-164h]
    unz_file_info_s file_info; // [esp+94h] [ebp-160h] BYREF
    char filename_inzip[256]; // [esp+E4h] [ebp-110h] BYREF
    unsigned int i; // [esp+1E8h] [ebp-Ch]
    unz_global_info_s gi; // [esp+1ECh] [ebp-8h] BYREF

    fs_numHeaderLongs = 0;
    uf = unzOpen(zipfile);
    if (unzGetGlobalInfo(uf, &gi))
        return 0;
    fs_iwdFileCount += gi.number_entry;
    len = 0;
    unzGoToFirstFile(uf);
    for (i = 0; i < gi.number_entry && !unzGetCurrentFileInfo(uf, &file_info, filename_inzip, 0x100u, 0, 0, 0, 0); ++i)
    {
        len += &filename_inzip[strlen(filename_inzip) + 1] - &filename_inzip[1] + 1;
        unzGoToNextFile(uf);
    }
    //buildBuffer = (fileInIwd_s*)Z_Malloc(len + 12 * gi.number_entry, "FS_LoadZipFile1", 3);
    buildBuffer = (fileInIwd_s*)Z_Malloc(gi.number_entry * sizeof(fileInIwd_s) + len, "FS_LoadZipFile1", 3);
    namePtr = (char*)&buildBuffer[gi.number_entry];
    //fs_headerLongs = (int*)Z_Malloc(4 * gi.number_entry, "FS_LoadZipFile2", 3);
    fs_headerLongs = (int*)Z_Malloc(gi.number_entry * sizeof(int), "FS_LoadZipFile2", 3);
    for (i = 1; i <= 0x400 && i <= gi.number_entry; i *= 2)
        ;
    //iwd = (iwd_t*) Z_Malloc(4 * i + 0x324, "FS_LoadZipFile3", 3);
    iwd = (iwd_t*) Z_Malloc(sizeof(iwd_t) + i * sizeof(fileInIwd_s *), "FS_LoadZipFile3", 3);
    iwd->hashSize = i;
    //iwd->hashTable = &iwd[1];
    iwd->hashTable = (fileInIwd_s**)(((char *)iwd) + sizeof(iwd_t));
    for (i = 0; i < iwd->hashSize; ++i)
        iwd->hashTable[i] = 0;
    I_strncpyz(iwd->iwdFilename, zipfile, 256);
    I_strncpyz(iwd->iwdBasename, basename, 256);
    if (strlen(iwd->iwdBasename) > 4 && !I_stricmp(&iwd->iwdFilename[strlen(iwd->iwdBasename) + 252], ".iwd"))
        iwd->iwdFilename[strlen(iwd->iwdBasename) + 252] = 0;
    iwd->handle = uf;
    iwd->numfiles = gi.number_entry;
    iwd->hasOpenFile = 0;
    unzGoToFirstFile(uf);
    for (i = 0; i < gi.number_entry && !unzGetCurrentFileInfo(uf, &file_info, filename_inzip, 0x100u, 0, 0, 0, 0); ++i)
    {
        if (file_info.uncompressed_size)
            fs_headerLongs[fs_numHeaderLongs++] = file_info.crc;
        I_strlwr(filename_inzip);
        hash = FS_HashFileName(filename_inzip, iwd->hashSize);
        buildBuffer[i].name = namePtr;
        v5 = filename_inzip;
        name = buildBuffer[i].name;
        do
        {
            v3 = *v5;
            *name++ = *v5++;
        } while (v3);
        namePtr += &filename_inzip[strlen(filename_inzip) + 1] - &filename_inzip[1] + 1;
        unzGetCurrentFileInfoPosition(uf, (unsigned long*)&buildBuffer[i].pos);
        buildBuffer[i].next = iwd->hashTable[hash];
        iwd->hashTable[hash] = &buildBuffer[i];
        unzGoToNextFile(uf);
    }
    iwd->checksum = Com_BlockChecksumKey32((const unsigned char *)fs_headerLongs, 4 * fs_numHeaderLongs, 0);
    if (fs_checksumFeed)
        iwd->pure_checksum = Com_BlockChecksumKey32((const unsigned char*)fs_headerLongs, 4 * fs_numHeaderLongs, fs_checksumFeed);
    else
        iwd->pure_checksum = iwd->checksum;
    iwd->checksum = iwd->checksum;
    iwd->pure_checksum = iwd->pure_checksum;
    Z_Free(fs_headerLongs, 3);
    iwd->buildBuffer = buildBuffer;
    return iwd;
}

char szIwdLanguageName[2][64];
int iString = 0;
char *__cdecl IwdFileLanguage(const char *pszIwdFileName)
{
    int iCurrChar; // [esp+10h] [ebp-4h]

    iString ^= 1u;
    if (strlen(pszIwdFileName) >= 0xA)
    {
        iCurrChar = 10;
        memset((unsigned __int8 *)szIwdLanguageName[iString], 0, sizeof(char[64]));
        while (iCurrChar < 64 && pszIwdFileName[iCurrChar] && isalpha(pszIwdFileName[iCurrChar]))
        {
            //*(_BYTE *)((iString << 6) + iCurrChar + 230219974) = pszIwdFileName[iCurrChar];
            szIwdLanguageName[iString][iCurrChar - 10] = pszIwdFileName[iCurrChar];
            ++iCurrChar;
        }
    }
    else
    {
        szIwdLanguageName[iString][0] = 0;
    }
    return szIwdLanguageName[iString];
}

int __cdecl FS_PathCmp(const char *s1, const char *s2)
{
    int c2; // [esp+0h] [ebp-8h]
    int c1; // [esp+4h] [ebp-4h]

    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if (I_islower(c1))
            c1 -= ' ';
        if (I_islower(c2))
            c2 -= ' ';
        if (c1 == '\\' || c1 == ':')
            c1 = '/';
        if (c2 == '\\' || c2 == ':')
            c2 = '/';
        if (c1 < c2)
            return -1;
        if (c1 > c2)
            return 1;
    } while (c1);
    return 0;
}

int __cdecl iwdsort(const char **a, char **b)
{
    char *pszLanguageB; // [esp+0h] [ebp-10h]
    char *pszLanguageA; // [esp+4h] [ebp-Ch]
    char *bb; // [esp+8h] [ebp-8h]
    char *aa; // [esp+Ch] [ebp-4h]

    aa = (char *)*a;
    bb = *b;
    if (!I_strncmp(*a, "          ", 10) && !I_strncmp(bb, "          ", 10))
    {
        pszLanguageA = IwdFileLanguage(aa);
        pszLanguageB = IwdFileLanguage(bb);
        if (I_stricmp(pszLanguageA, "english"))
        {
            if (!I_stricmp(pszLanguageB, "english"))
                return 1;
        }
        else if (I_stricmp(pszLanguageB, "english"))
        {
            return -1;
        }
    }
    return FS_PathCmp(aa, bb);
}

void __cdecl FS_AddIwdFilesForGameDirectory(char *path, char *pszGameFolder)
{
    char *v2; // eax
    const char *LanguageName; // eax
    char v4; // [esp+3h] [ebp-1149h]
    char *iwdGamename; // [esp+8h] [ebp-1144h]
    char *v6; // [esp+Ch] [ebp-1140h]
    signed int j; // [esp+20h] [ebp-112Ch]
    char ospath[260]; // [esp+24h] [ebp-1128h] BYREF
    iwd_t *ZipFile; // [esp+12Ch] [ebp-1020h]
    int v10; // [esp+130h] [ebp-101Ch]
    char **list; // [esp+134h] [ebp-1018h]
    char *pszLanguageName; // [esp+138h] [ebp-1014h]
    searchpath_s *search; // [esp+13Ch] [ebp-1010h]
    int i; // [esp+140h] [ebp-100Ch]
    int piLanguageIndex; // [esp+144h] [ebp-1008h] BYREF
    int numfiles; // [esp+148h] [ebp-1004h] BYREF
    char *s0[1024]; // [esp+14Ch] [ebp-1000h] BYREF

    FS_BuildOSPath(path, pszGameFolder, (char *)"", ospath);
    ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
    list = Sys_ListFiles(ospath, "iwd", 0, &numfiles, 0);
    if (numfiles > 1024)
    {
        Com_PrintWarning(
            10,
            "WARNING: Exceeded max number of iwd files in %s/%s (%1/%1)\n",
            path,
            pszGameFolder,
            numfiles,
            1024);
        numfiles = 1024;
    }
    for (i = 0; i < numfiles; ++i)
    {
        s0[i] = list[i];
        if (!I_strncmp(s0[i], "localized_", 10))
        {
            v2 = s0[i];
            *(_DWORD *)v2 = *(_DWORD *)"          ";
            *((_DWORD *)v2 + 1) = 538976288;    
            *((_WORD *)v2 + 4) = 8224;
        }
    }
    qsort(s0, numfiles, 4u, (int(__cdecl *)(const void *, const void *))iwdsort);
    for (i = 0; i < numfiles; ++i)
    {
        if (I_strncmp(s0[i], "          ", 10))
        {
            v10 = 0;
            piLanguageIndex = 0;
        }
        else
        {
            qmemcpy(s0[i], "localized_", 10);
            v10 = 1;
            pszLanguageName = IwdFileLanguage(s0[i]);
            if (!*pszLanguageName)
            {
                Com_PrintWarning(
                    10,
                    "WARNING: Localized assets iwd file %s/%s/%s has invalid name (no language specified). Proper naming convention"
                    " is: localized_[language]_iwd#.iwd\n",
                    path,
                    pszGameFolder,
                    s0[i]);
                continue;
            }
            if (!SEH_GetLanguageIndexForName(pszLanguageName, &piLanguageIndex))
            {
                Com_PrintWarning(
                    10,
                    "WARNING: Localized assets iwd file %s/%s/%s has invalid name (bad language name specified). Proper naming conv"
                    "ention is: localized_[language]_iwd#.iwd\n",
                    path,
                    pszGameFolder,
                    s0[i]);
                if (!bLanguagesListed)
                {
                    Com_Printf(10, "Supported languages are:\n");
                    for (j = 0; j < 15; ++j)
                    {
                        LanguageName = SEH_GetLanguageName(j);
                        Com_Printf(10, "    %s\n", LanguageName);
                    }
                    bLanguagesListed = 1;
                }
                continue;
            }
        }
        FS_BuildOSPath(path, pszGameFolder, s0[i], ospath);
        ZipFile = FS_LoadZipFile(ospath, s0[i]);
        if (ZipFile)
        {
            v6 = pszGameFolder;
            iwdGamename = ZipFile->iwdGamename;
            do
            {
                v4 = *v6;
                *iwdGamename++ = *v6++;
            } while (v4);
            search = (searchpath_s *)Z_Malloc(28, "FS_AddIwdFilesForGameDirectory", 3);
            search->iwd = ZipFile;
            search->bLocalized = v10;
            search->language = piLanguageIndex;
            FS_AddSearchPath(search);
        }
    }
    FS_FreeFileList((const char **)list);
}

#ifdef WIN32
int __cdecl Sys_DirectoryHasContents(const char *directory)
{
    _finddata64i32_t findinfo; // [esp+0h] [ebp-238h] BYREF
    int findhandle; // [esp+12Ch] [ebp-10Ch]
    char search[260]; // [esp+130h] [ebp-108h] BYREF

    Com_sprintf(search, 0x100u, "%s\\*", directory);
    findhandle = _findfirst64i32(search, &findinfo);
    if (findhandle == -1)
        return 0;
    do
    {
        if ((findinfo.attrib & 0x10) == 0
            || I_stricmp(findinfo.name, ".") && I_stricmp(findinfo.name, "..") && I_stricmp(findinfo.name, "CVS"))
        {
            _findclose(findhandle);
            return 1;
        }
    } while (_findnext64i32(findhandle, &findinfo) != -1);
    _findclose(findhandle);
    return 0;
}
#endif

void __cdecl FS_AddGameDirectory(char *path, char *dir, int bLanguageDirectory, int iLanguage)
{
    unsigned int *v4; // eax
    int v5; // eax
    const char *v6; // [esp+10h] [ebp-15Ch]
    char ospath[260]; // [esp+14h] [ebp-158h] BYREF
    const char *pszLanguage; // [esp+118h] [ebp-54h]
    char szGameFolder[68]; // [esp+11Ch] [ebp-50h] BYREF
    searchpath_s *search; // [esp+164h] [ebp-8h]
    searchpath_s *i; // [esp+168h] [ebp-4h]

    if (bLanguageDirectory)
    {
        pszLanguage = SEH_GetLanguageName(iLanguage);
        Com_sprintf(szGameFolder, 0x40u, "%s/%s", dir, pszLanguage);
    }
    else
    {
        I_strncpyz(szGameFolder, dir, 64);
    }
    for (i = fs_searchpaths; i; i = i->next)
    {
        if (i->dir && !I_stricmp(i->dir->path, path) && !I_stricmp(i->dir->gamedir, szGameFolder))
        {
            if (i->bLocalized != bLanguageDirectory)
            {
                if (i->bLocalized)
                    v6 = "localized";
                else
                    v6 = "non-localized";
                Com_PrintWarning(
                    10,
                    "WARNING: game folder %s/%s added as both localized & non-localized. Using folder as %s\n",
                    path,
                    szGameFolder,
                    v6);
            }
            if (i->bLocalized)
            {
                if (i->language != iLanguage)
                    Com_PrintWarning(
                        10,
                        "WARNING: game folder %s/%s re-added as localized folder with different language\n",
                        path,
                        szGameFolder);
            }
            return;
        }
    }
    if (bLanguageDirectory)
    {
        FS_BuildOSPath(path, szGameFolder, (char *)"", ospath);
        ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
        if (!Sys_DirectoryHasContents(ospath))
            return;
    }
    else
    {
        I_strncpyz(fs_gamedir, szGameFolder, 256);
    }
    search = (searchpath_s *)Z_Malloc(28, "FS_AddGameDirectory", 3);
    v4 = (unsigned int*)Z_Malloc(512, "FS_AddGameDirectory", 3);
    search->dir = (directory_t *)v4;
    I_strncpyz(search->dir->path, path, 256);
    I_strncpyz(search->dir->gamedir, szGameFolder, 256);
    if (!bLanguageDirectory && iLanguage)
        MyAssertHandler(
            ".\\universal\\com_files.cpp",
            4059,
            0,
            "%s",
            "bLanguageDirectory || (!bLanguageDirectory && !iLanguage)");
    search->bLocalized = bLanguageDirectory;
    search->language = iLanguage;
    search->ignore = 0;
    v5 = I_stricmp(dir, "players");
    search->ignorePureCheck = v5 == 0;
    FS_AddSearchPath(search);
    FS_AddIwdFilesForGameDirectory(path, szGameFolder);
}

void __cdecl FS_AddLocalizedGameDirectory(char *path, char *dir)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 14; i >= 0; --i)
        FS_AddGameDirectory(path, dir, 1, i);
    FS_AddGameDirectory(path, dir, 0, 0);
}

void __cdecl Com_ReadCDKey()
{
    // KISAKTODO: this sucks!
    //unsigned int size; // [esp+0h] [ebp-28h] BYREF
    //unsigned int type; // [esp+4h] [ebp-24h] BYREF
    //char regkey[21]; // [esp+8h] [ebp-20h] BYREF
    //HKEY__ *hkey; // [esp+24h] [ebp-4h] BYREF
    //
    //if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4", &hkey))
    //{
    //    Com_ClearCDKey();
    //}
    //else
    //{
    //    type = 1;
    //    size = 21;
    //    if (RegQueryValueExA(hkey, "codkey", 0, &type, regkey, &size))
    //    {
    //        RegCloseKey(hkey);
    //        Com_ClearCDKey();
    //    }
    //    else
    //    {
    //        RegCloseKey(hkey);
    //        if (size != 21
    //            || (*cl_cdkey = *regkey,
    //                *&cl_cdkey[4] = *&regkey[4],
    //                *&cl_cdkey[8] = *&regkey[8],
    //                *&cl_cdkey[12] = *&regkey[12],
    //                cl_cdkey[16] = 0,
    //                *cl_cdkeychecksum = *&regkey[16],
    //                byte_9487EC = 0,
    //                !CL_CDKeyValidate(cl_cdkey, cl_cdkeychecksum)))
    //        {
    //            Com_ClearCDKey();
    //        }
    //    }
    //}
}

void __cdecl FS_DisplayPath(int bLanguageCull)
{
    const char *LanguageName; // eax
    const char *v2; // eax
    unsigned int iLanguage; // [esp+0h] [ebp-10h]
    searchpath_s *s; // [esp+4h] [ebp-Ch]
    const char *pszLanguageName; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    iLanguage = SEH_GetCurrentLanguage();
    pszLanguageName = SEH_GetLanguageName(iLanguage);
    Com_Printf(10, "Current language: %s\n", pszLanguageName);
    if (fs_ignoreLocalized->current.enabled)
        Com_Printf(10, "    localized assets are being ignored\n");
    Com_Printf(10, "Current search path:\n");
    for (s = fs_searchpaths; s; s = s->next)
    {
        if (!bLanguageCull || FS_UseSearchPath(s))
        {
            if (s->iwd)
            {
                Com_Printf(10, "%s (%i files)\n", s->iwd->iwdFilename, s->iwd->numfiles);
                if (s->bLocalized)
                {
                    LanguageName = SEH_GetLanguageName(s->language);
                    Com_Printf(10, "    localized assets iwd file for %s\n", LanguageName);
                }
                if (fs_numServerIwds)
                {
                    if (FS_IwdIsPure(s->iwd))
                        Com_Printf(10, "    on the pure list\n");
                    else
                        Com_Printf(10, "    not on the pure list\n");
                }
            }
            else
            {
                Com_Printf(10, "%s/%s\n", s->dir->path, s->dir->gamedir);
                if (s->bLocalized)
                {
                    v2 = SEH_GetLanguageName(s->language);
                    Com_Printf(10, "    localized assets game folder for %s\n", v2);
                }
            }
        }
    }
    Com_Printf(10, "\nFile Handles:\n");
    for (i = 1; i < 65; ++i)
    {
        if (fsh[i].handleFiles.file.o)
            Com_Printf(10, "handle %i: %s\n", i, fsh[i].name);
    }
}

void __cdecl FS_Path_f()
{
    FS_DisplayPath(1);
}
void __cdecl FS_FullPath_f()
{
    FS_DisplayPath(0);
}

void __cdecl FS_Dir_f()
{
    const char *path; // [esp+0h] [ebp-14h]
    const char *extension; // [esp+4h] [ebp-10h]
    int ndirs; // [esp+8h] [ebp-Ch] BYREF
    int i; // [esp+Ch] [ebp-8h]
    const char **dirnames; // [esp+10h] [ebp-4h]

    if (Cmd_Argc() >= 2 && Cmd_Argc() <= 3)
    {
        if (Cmd_Argc() == 2)
        {
            path = Cmd_Argv(1);
            extension = "";
            Com_Printf(0, "Directory of %s %s\n", path, "");
        }
        else
        {
            path = Cmd_Argv(1);
            extension = Cmd_Argv(2);
            Com_Printf(0, "Directory of %s %s\n", path, extension);
        }
        Com_Printf(0, "---------------\n");
        dirnames = FS_ListFiles(path, extension, FS_LIST_PURE_ONLY, &ndirs);
        for (i = 0; i < ndirs; ++i)
            Com_Printf(0, "%s\n", dirnames[i]);
        FS_FreeFileList(dirnames);
    }
    else
    {
        Com_Printf(0, "usage: dir <directory> [extension]\n");
    }
}

void __cdecl FS_SortFileList(const char **filelist, int numfiles)
{
    int j; // [esp+4h] [ebp-14h]
    int k; // [esp+8h] [ebp-10h]
    int numsortedfiles; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    char *sortedlist; // [esp+14h] [ebp-4h]

    sortedlist = (char*)Z_Malloc(4 * numfiles + 4, "FS_SortFileList", 3);
    *sortedlist = 0;
    numsortedfiles = 0;
    for (i = 0; i < numfiles; ++i)
    {
        for (j = 0; j < numsortedfiles && FS_PathCmp(filelist[i], (const char*)*&sortedlist[4 * j]) >= 0; ++j)
            ;
        for (k = numsortedfiles; k > j; --k)
            *&sortedlist[4 * k] = *&sortedlist[4 * k - 4];
        *&sortedlist[4 * j] = (char)filelist[i]; // KISAKTODO: probably cooked
        ++numsortedfiles;
    }
    Com_Memcpy(filelist, sortedlist, 4 * numfiles);
    Z_Free(sortedlist, 3);
}

void __cdecl FS_NewDir_f()
{
    int ndirs; // [esp+0h] [ebp-10h] BYREF
    int i; // [esp+4h] [ebp-Ch]
    const char *filter; // [esp+8h] [ebp-8h]
    const char **dirnames; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        filter = Cmd_Argv(1);
        Com_Printf(0, "---------------\n");
        dirnames = FS_ListFilteredFiles(fs_searchpaths, "", "", filter, FS_LIST_PURE_ONLY, &ndirs);
        FS_SortFileList(dirnames, ndirs);
        for (i = 0; i < ndirs; ++i)
        {
            FS_ConvertPath((char*)dirnames[i]);
            Com_Printf(0, "%s\n", dirnames[i]);
        }
        Com_Printf(0, "%d files listed\n", ndirs);
        FS_FreeFileList(dirnames);
    }
    else
    {
        Com_Printf(0, "usage: fdir <filter>\n");
        Com_Printf(0, "example: fdir *q3dm*.bsp\n");
    }
}

int __cdecl FS_TouchFile(const char *name)
{
    int f; // [esp+0h] [ebp-4h] BYREF

    FS_FOpenFileRead(name, &f);
    if (!f)
        return 0;
    FS_FCloseFile(f);
    return 1;
}

void __cdecl FS_TouchFile_f()
{
    const char *v0; // eax

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        FS_TouchFile(v0);
    }
    else
    {
        Com_Printf(0, "Usage: touchFile <file>\n");
    }
}

cmd_function_s FS_Path_f_VAR;
cmd_function_s FS_FullPath_f_VAR;
cmd_function_s FS_NewDir_f_VAR;
cmd_function_s FS_Dir_f_VAR;
cmd_function_s FS_TouchFile_f_VAR;
void __cdecl FS_AddCommands()
{
    Cmd_AddCommandInternal("path", FS_Path_f, &FS_Path_f_VAR);
    Cmd_AddCommandInternal("fullpath", FS_FullPath_f, &FS_FullPath_f_VAR);
    Cmd_AddCommandInternal("dir", FS_Dir_f, &FS_Dir_f_VAR);
    Cmd_AddCommandInternal("fdir", FS_NewDir_f, &FS_NewDir_f_VAR);
    Cmd_AddCommandInternal("touchFile", FS_TouchFile_f, &FS_TouchFile_f_VAR);
}

void __cdecl FS_Startup(char *gameName)
{
    char *v2; // eax
    char *v3; // eax

    Com_Printf(10, "----- FS_Startup -----\n");
    FS_RegisterDvars();
    if (*(_BYTE *)fs_basepath->current.integer)
    {
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char*)"devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char*)"devraw");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char*)"raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char*)"raw");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char*)"players");
    }
    if (*(_BYTE *)fs_homepath->current.integer && I_stricmp(fs_basepath->current.string, fs_homepath->current.string))
    {
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char*)"devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char*)"devraw");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char*)"raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char*)"raw");
    }
    if (*(_BYTE *)fs_cdpath->current.integer && I_stricmp(fs_basepath->current.string, fs_cdpath->current.string))
    {
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char*)"devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char*)"devraw");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char*)"raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char*)"raw");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, gameName);
    }
    if (*(_BYTE *)fs_basepath->current.integer)
    {
        v2 = va("%s_shared", gameName);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, v2);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, gameName);
    }
    if (*(_BYTE *)fs_basepath->current.integer && I_stricmp(fs_homepath->current.string, fs_basepath->current.string))
    {
        v3 = va("%s_shared", gameName);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, v3);
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, gameName);
    }
    if (*(_BYTE *)fs_basegame->current.integer
        && !I_stricmp(gameName, "main")
        && I_stricmp(fs_basegame->current.string, gameName))
    {
        if (*(_BYTE *)fs_cdpath->current.integer)
            FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char *)fs_basegame->current.integer);
        if (*(_BYTE *)fs_basepath->current.integer)
            FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char *)fs_basegame->current.integer);
        if (*(_BYTE *)fs_homepath->current.integer && I_stricmp(fs_homepath->current.string, fs_basepath->current.string))
            FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char *)fs_basegame->current.integer);
    }
    if (*(_BYTE *)fs_gameDirVar->current.integer
        && !I_stricmp(gameName, "main")
        && I_stricmp(fs_gameDirVar->current.string, gameName))
    {
        if (*(_BYTE *)fs_cdpath->current.integer)
            FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, (char *)fs_gameDirVar->current.integer);
        if (*(_BYTE *)fs_basepath->current.integer)
            FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, (char *)fs_gameDirVar->current.integer);
        if (*(_BYTE *)fs_homepath->current.integer && I_stricmp(fs_homepath->current.string, fs_basepath->current.string))
            FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, (char *)fs_gameDirVar->current.integer);
    }
    Com_ReadCDKey();
    FS_AddCommands();
    FS_Path_f();
    Dvar_ClearModified((dvar_s*)fs_gameDirVar);
    Com_Printf(10, "----------------------\n");
    Com_Printf(10, "%d files in iwd files\n", fs_iwdFileCount);
}

void __cdecl FS_SetRestrictions()
{
    searchpath_s *path; // [esp+0h] [ebp-4h]

    if (fs_restrict->current.enabled)
    {
        Dvar_SetBool((dvar_s*)fs_restrict, 1);
        Com_Printf(10, "\nRunning in restricted demo mode.\n\n");
        FS_Shutdown();
        FS_Startup((char*)"demomain");
        for (path = fs_searchpaths; path; path = path->next)
        {
            if (FS_UseSearchPath(path) && path->iwd && (path->iwd->checksum ^ 0x2261994) != 0xB3D38C61)
                Com_Error(ERR_FATAL, "Corrupted iw0.iwd: %u", path->iwd->checksum);
        }
    }
}

bool __cdecl FS_IsBasePathValid()
{
    return FS_ReadFile("fileSysCheck.cfg", 0) > 0;
}

char lastValidBase[256];
char lastValidGame[256];
void __cdecl FS_InitFilesystem()
{
    Com_StartupVariable("fs_cdpath");
    Com_StartupVariable("fs_basepath");
    Com_StartupVariable("fs_homepath");
    Com_StartupVariable("fs_game");
    Com_StartupVariable("fs_copyfiles");
    Com_StartupVariable("fs_restrict");
    Com_StartupVariable("loc_language");
    SEH_InitLanguage();
    FS_Startup((char*)"main");
    SEH_Init_StringEd();
    SEH_UpdateLanguageInfo();
    FS_SetRestrictions();
    if (!FS_IsBasePathValid())
        Com_Error(
            ERR_FATAL,
            "Couldn't load %s.  Make sure Call of Duty is run from the correct folder.",
            "fileSysCheck.cfg");
    I_strncpyz(lastValidBase, (char *)fs_basepath->current.integer, 256);
    I_strncpyz(lastValidGame, (char *)fs_gameDirVar->current.integer, 256);
}

unsigned int __cdecl FS_FOpenFileByMode(char *qpath, int *f, fsMode_t mode)
{
    unsigned int r; // [esp+4h] [ebp-8h]
    int sync; // [esp+8h] [ebp-4h]

    r = 6969;
    sync = 0;
    switch (mode)
    {
    case FS_READ:
        r = FS_FOpenFileRead(qpath, f);
        break;
    case FS_WRITE:
        *f = FS_FOpenFileWrite(qpath);
        r = 0;
        if (!*f)
            r = -1;
        break;
    case FS_APPEND:
        goto $LN5_83;
    case FS_APPEND_SYNC:
        sync = 1;
    $LN5_83:
        *f = FS_FOpenFileAppend(qpath);
        r = 0;
        if (!*f)
            r = -1;
        break;
    default:
        Com_Error(ERR_FATAL, "FSH_FOpenFile: bad mode");
        break;
    }
    if (!f)
        return r;
    if (*f)
    {
        fsh[*f].fileSize = r;
        fsh[*f].streamed = 0;
    }
    fsh[*f].handleSync = sync;
    return r;
}

void __cdecl FS_Flush(int f)
{
    FILE*v1; // eax

    v1 = FS_FileForHandle(f);
    fflush(v1);
}

void __cdecl Com_GetBspFilename(char *filename, unsigned int size, const char *mapname)
{
    Com_sprintf(filename, size, "maps/mp/%s.d3dbsp", mapname);
}

void __cdecl FS_FreeFileList(const char **list)
{
    if (list)
        Hunk_UserDestroy((HunkUser *)*(list - 1));
}

unsigned int __cdecl FS_FTell(int f)
{
    FILE *v1; // eax

    if (fsh[f].zipFile)
        return unztell(fsh[f].handleFiles.file.z);
    v1 = FS_FileForHandle(f);
    return ftell(v1);
}

int __cdecl FS_GetModList(char *listbuf, int bufsize)
{
    char v3; // [esp+3h] [ebp-18Dh]
    char *v4; // [esp+8h] [ebp-188h]
    char *v5; // [esp+Ch] [ebp-184h]
    char v6; // [esp+13h] [ebp-17Dh]
    char *v7; // [esp+18h] [ebp-178h]
    char *v8; // [esp+1Ch] [ebp-174h]
    char v9; // [esp+33h] [ebp-15Dh]
    char *v10; // [esp+38h] [ebp-158h]
    char *v11; // [esp+3Ch] [ebp-154h]
    char v12; // [esp+43h] [ebp-14Dh]
    char *v13; // [esp+48h] [ebp-148h]
    char *v14; // [esp+4Ch] [ebp-144h]
    FILE *file; // [esp+60h] [ebp-130h]
    int nMods; // [esp+64h] [ebp-12Ch]
    int nDescLen; // [esp+68h] [ebp-128h]
    int nDescLena; // [esp+68h] [ebp-128h]
    int descHandle; // [esp+6Ch] [ebp-124h] BYREF
    int dummy; // [esp+70h] [ebp-120h] BYREF
    char *name; // [esp+74h] [ebp-11Ch]
    char descPath[256]; // [esp+78h] [ebp-118h] BYREF
    int nPotential; // [esp+17Ch] [ebp-14h]
    int nLen; // [esp+180h] [ebp-10h]
    int nTotal; // [esp+184h] [ebp-Ch]
    int i; // [esp+188h] [ebp-8h]
    char **pFiles; // [esp+18Ch] [ebp-4h]
    char *listbufa; // [esp+198h] [ebp+8h]

    pFiles = 0;
    *listbuf = 0;
    nTotal = 0;
    nPotential = 0;
    nMods = 0;
    sprintf(descPath, "%s/%s", fs_homepath->current.string, "mods");
    pFiles = Sys_ListFiles(descPath, 0, 0, &dummy, 1);
    nPotential = Sys_CountFileList(pFiles);
    for (i = 0; i < nPotential; ++i)
    {
        name = pFiles[i];
        nLen = strlen(name) + 1;
        v14 = name;
        v13 = descPath;
        do
        {
            v12 = *v14;
            *v13++ = *v14++;
        } while (v12);
        I_strncat(descPath, 256, "/description.txt");
        if (FS_SV_FOpenFileRead(descPath, &descHandle) > 0 && descHandle)
        {
            file = FS_FileForHandle(descHandle);
            Com_Memset((unsigned int *)descPath, 0, 256);
            nDescLen = FS_FileRead(descPath, 0x30u, file);
            if (nDescLen >= 0)
                descPath[nDescLen] = 0;
            FS_FCloseFile(descHandle);
        }
        else
        {
            v11 = name;
            v10 = descPath;
            do
            {
                v9 = *v11;
                *v10++ = *v11++;
            } while (v9);
        }
        nDescLena = &descPath[strlen(descPath) + 1] - &descPath[1] + 1;
        if (nLen + nTotal + nDescLena + 2 >= bufsize)
            break;
        v8 = name;
        v7 = listbuf;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        } while (v6);
        listbufa = &listbuf[nLen];
        v5 = descPath;
        v4 = listbufa;
        do
        {
            v3 = *v5;
            *v4++ = *v5++;
        } while (v3);
        listbuf = &listbufa[nDescLena];
        nTotal += nDescLena + nLen;
        ++nMods;
    }
    FS_FreeFileList((const char **)pFiles);
    return nMods;
}

int __cdecl FS_GetFileList(
    const char *path,
    const char *extension,
    FsListBehavior_e behavior,
    char *listbuf,
    int bufsize)
{
    char v6; // [esp+3h] [ebp-35h]
    char *v7; // [esp+8h] [ebp-30h]
    const char *v8; // [esp+Ch] [ebp-2Ch]
    const char **fileNames; // [esp+24h] [ebp-14h]
    int nLen; // [esp+28h] [ebp-10h]
    int nTotal; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    int fileCount; // [esp+34h] [ebp-4h] BYREF

    *listbuf = 0;
    fileCount = 0;
    nTotal = 0;
    if (!I_stricmp(path, "$modlist"))
        return FS_GetModList(listbuf, bufsize);
    fileNames = FS_ListFiles(path, extension, behavior, &fileCount);
    for (i = 0; i < fileCount; ++i)
    {
        nLen = strlen(fileNames[i]) + 1;
        if (nTotal + nLen + 1 >= bufsize)
        {
            fileCount = i;
            break;
        }
        v8 = fileNames[i];
        v7 = listbuf;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        } while (v6);
        listbuf += nLen;
        nTotal += nLen;
    }
    FS_FreeFileList(fileNames);
    return fileCount;
}

int __cdecl FS_AddFileToList(HunkUser *user, const char *name, const char **list, int nfiles)
{
    int i; // [esp+0h] [ebp-4h]

    if (nfiles == 0x1FFF)
        return 0x1FFF;
    for (i = 0; i < nfiles; ++i)
    {
        if (!I_stricmp(name, list[i]))
            return nfiles;
    }
    list[nfiles] = Hunk_CopyString(user, name);
    return nfiles + 1;
}

int __cdecl FS_ReturnPath(const char *zname, char *zpath, int *depth)
{
    char v3; // cl
    char *v5; // [esp+8h] [ebp-14h]
    const char *v6; // [esp+Ch] [ebp-10h]
    int at; // [esp+10h] [ebp-Ch]
    int len; // [esp+14h] [ebp-8h]
    int newdep; // [esp+18h] [ebp-4h]

    newdep = 0;
    *zpath = 0;
    len = 0;
    for (at = 0; zname[at]; ++at)
    {
        if (zname[at] == '/' || zname[at] == '\\')
        {
            len = at;
            ++newdep;
        }
    }
    v6 = zname;
    v5 = zpath;
    do
    {
        v3 = *v6;
        *v5++ = *v6++;
    } while (v3);
    //zpath[len] = 0;
    *v5 = 0;
    if (len + 1 == at)
        --newdep;
    *depth = newdep;
    return len;
}

int __cdecl FS_IwdIsPure(iwd_t *iwd)
{
    int i; // [esp+0h] [ebp-4h]

    if (!fs_numServerIwds)
        return 1;
    for (i = 0; i < fs_numServerIwds; ++i)
    {
        if (iwd->checksum == fs_serverIwds[i])
            return 1;
    }
    return 0;
}

const char **__cdecl FS_ListFilteredFiles(
    searchpath_s *searchPath,
    const char *path,
    const char *extension,
    const char *filter,
    FsListBehavior_e behavior,
    int *numfiles)
{
    char v7; // [esp+13h] [ebp-3E1h]
    char *v8; // [esp+18h] [ebp-3DCh]
    char *v9; // [esp+1Ch] [ebp-3D8h]
    char *v10; // [esp+5Ch] [ebp-398h]
    char netpath[256]; // [esp+64h] [ebp-390h] BYREF
    int numSysFiles; // [esp+164h] [ebp-290h] BYREF
    char **sysFiles; // [esp+168h] [ebp-28Ch]
    char szTrimmedName[68]; // [esp+16Ch] [ebp-288h] BYREF
    int depth; // [esp+1B0h] [ebp-244h] BYREF
    char *name; // [esp+1B4h] [ebp-240h]
    int zpathLen; // [esp+1B8h] [ebp-23Ch]
    int pathDepth; // [esp+1BCh] [ebp-238h] BYREF
    iwd_t *iwd; // [esp+1C0h] [ebp-234h]
    char zpath[259]; // [esp+1C4h] [ebp-230h] BYREF
    bool isDirSearch; // [esp+2C7h] [ebp-12Dh]
    const char **list; // [esp+2C8h] [ebp-12Ch]
    int extensionLength; // [esp+2CCh] [ebp-128h]
    int nfiles; // [esp+2D0h] [ebp-124h]
    int pathLength; // [esp+2D4h] [ebp-120h]
    fileInIwd_s *buildBuffer; // [esp+2D8h] [ebp-11Ch]
    HunkUser *user; // [esp+2DCh] [ebp-118h]
    int temp; // [esp+2E0h] [ebp-114h]
    char sanitizedPath[256]; // [esp+2E4h] [ebp-110h] BYREF
    searchpath_s *search; // [esp+3E8h] [ebp-Ch]
    int i; // [esp+3ECh] [ebp-8h]
    int length; // [esp+3F0h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!path)
    {
        *numfiles = 0;
        return 0;
    }
    if (!extension)
        extension = "";
    if (!FS_SanitizeFilename(path, sanitizedPath, 256))
    {
        *numfiles = 0;
        return 0;
    }
    isDirSearch = I_stricmp(extension, "/") == 0;
    v10 = &sanitizedPath[strlen(sanitizedPath) + 1];
    pathLength = v10 - &sanitizedPath[1];
    if (v10 != &sanitizedPath[1] && (sanitizedPath[pathLength - 1] == 92 || sanitizedPath[pathLength - 1] == 47))
        --pathLength;
    extensionLength = strlen(extension);
    nfiles = 0;
    FS_ReturnPath(sanitizedPath, zpath, &pathDepth);
    if (sanitizedPath[0])
        ++pathDepth;
    user = Hunk_UserCreate(0x20000, "FS_ListFilteredFiles", 0, 0, 3);
    list = (const char **)Hunk_UserAlloc(user, 0x8004u, 4);
    *list++ = (const char *)user;
    for (search = searchPath; search; search = search->next)
    {
        if (FS_UseSearchPath(search))
        {
            if (search->iwd)
            {
                if (search->bLocalized || FS_IwdIsPure(search->iwd))
                {
                    iwd = search->iwd;
                    buildBuffer = iwd->buildBuffer;
                    for (i = 0; i < iwd->numfiles; ++i)
                    {
                        name = buildBuffer[i].name;
                        if (filter)
                        {
                            if (Com_FilterPath(filter, name, 0))
                                nfiles = FS_AddFileToList(user, name, list, nfiles);
                            continue;
                        }
                        zpathLen = FS_ReturnPath(name, zpath, &depth);
                        if (depth == pathDepth
                            && pathLength <= zpathLen
                            && (pathLength <= 0 || name[pathLength] == 47)
                            && !I_strnicmp(name, sanitizedPath, pathLength))
                        {
                            if (!isDirSearch)
                            {
                                if (extensionLength)
                                {
                                    length = strlen(name);
                                    if (length <= extensionLength
                                        || name[length - extensionLength - 1] != 46
                                        || I_stricmp(&name[length - extensionLength], extension))
                                    {
                                        continue;
                                    }
                                }
                            LABEL_44:
                                temp = pathLength;
                                if (pathLength)
                                    ++temp;
                                if (isDirSearch)
                                {
                                    v9 = &name[temp];
                                    v8 = szTrimmedName;
                                    do
                                    {
                                        v7 = *v9;
                                        *v8++ = *v9++;
                                    } while (v7);
                                    szTrimmedName[&szTrimmedName[strlen(szTrimmedName) + 1] - &szTrimmedName[1] - 1] = 0;
                                    nfiles = FS_AddFileToList(user, szTrimmedName, list, nfiles);
                                }
                                else
                                {
                                    nfiles = FS_AddFileToList(user, &name[temp], list, nfiles);
                                }
                                continue;
                            }
                            if (extensionLength != 1)
                                MyAssertHandler(".\\universal\\com_files.cpp", 3314, 1, "%s", "extensionLength == 1");
                            if (*extension != 47 || extension[1])
                                MyAssertHandler(
                                    ".\\universal\\com_files.cpp",
                                    3315,
                                    1,
                                    "%s",
                                    "extension[0] == '/' && extension[1] == '\\0'");
                            if (name[strlen(name) - 1] == 47)
                                goto LABEL_44;
                        }
                    }
                }
            }
            else if (search->dir && (!fs_restrict->current.enabled && !fs_numServerIwds || behavior))
            {
                FS_BuildOSPath(search->dir->path, search->dir->gamedir, sanitizedPath, netpath);
                sysFiles = Sys_ListFiles(netpath, extension, filter, &numSysFiles, isDirSearch);
                for (i = 0; i < numSysFiles; ++i)
                    nfiles = FS_AddFileToList(user, sysFiles[i], list, nfiles);
                FS_FreeFileList((const char **)sysFiles);
            }
        }
    }
    *numfiles = nfiles;
    if (nfiles)
    {
        list[nfiles] = 0;
        return list;
    }
    else
    {
        Hunk_UserDestroy(user);
        return 0;
    }
}

const char **__cdecl FS_ListFiles(const char *path, const char *extension, FsListBehavior_e behavior, int *numfiles)
{
    return FS_ListFilteredFiles(fs_searchpaths, path, extension, 0, behavior, numfiles);
}

bool __cdecl FS_CheckLocation(const char *path, int lookInFlags)
{
    if (lookInFlags == 63)
        return 1;
    if ((lookInFlags & 1) != 0 && !I_strncmp(path, "main", 4))
        return 1;
    if ((lookInFlags & 2) != 0 && !I_strncmp(path, "dev", 3))
        return 1;
    if ((lookInFlags & 4) != 0 && !I_strncmp(path, "temp", 4))
        return 1;
    if ((lookInFlags & 8) != 0 && !I_strncmp(path, "raw", 3))
        return 1;
    if ((lookInFlags & 0x10) != 0 && !I_strncmp(path, "raw_shared", 10))
        return 1;
    return (lookInFlags & 0x20) != 0 && !I_strncmp(path, "devraw", 6);
}

const char **__cdecl FS_ListFilteredFilesInLocation(
    const char *path,
    const char *extension,
    const char *filter,
    FsListBehavior_e behavior,
    int *numfiles,
    int lookInFlags)
{
    const char **result; // [esp+0h] [ebp-18h]
    searchpath_s *locationSearchPath; // [esp+4h] [ebp-14h]
    HunkUser *user; // [esp+8h] [ebp-10h]
    char *pathDir; // [esp+Ch] [ebp-Ch]
    searchpath_s *search; // [esp+10h] [ebp-8h]
    searchpath_s *locationSearch; // [esp+14h] [ebp-4h]

    user = Hunk_UserCreate(0x20000, "FS_ListFilteredFilesInLocation", 0, 0, 3);
    locationSearchPath = 0;
    locationSearch = 0;
    for (search = fs_searchpaths; search; search = search->next)
    {
        if (search->dir)
        {
            pathDir = search->dir->gamedir;
        }
        else if (search->iwd)
        {
            pathDir = search->iwd->iwdGamename;
        }
        else
        {
            pathDir = 0;
        }
        if (!pathDir)
            MyAssertHandler(".\\universal\\com_files.cpp", 3483, 0, "%s", "pathDir");
        if (FS_CheckLocation(pathDir, lookInFlags))
        {
            if (locationSearchPath)
            {
                locationSearch->next = (searchpath_s *)Hunk_UserAlloc(user, 0x1Cu, 4);
                locationSearch = locationSearch->next;
            }
            else
            {
                locationSearchPath = (searchpath_s *)Hunk_UserAlloc(user, 0x1Cu, 4);
                locationSearch = locationSearchPath;
            }
            locationSearch->next = 0;
            locationSearch->dir = search->dir;
            locationSearch->language = search->language;
            locationSearch->bLocalized = search->bLocalized;
            locationSearch->iwd = search->iwd;
        }
    }
    result = FS_ListFilteredFiles(locationSearchPath, path, extension, filter, behavior, numfiles);
    Hunk_UserDestroy(user);
    return result;
}

const char **__cdecl FS_ListFilesInLocation(
    const char *path,
    const char *extension,
    FsListBehavior_e behavior,
    int *numfiles,
    int lookInFlags)
{
    return FS_ListFilteredFilesInLocation(path, extension, 0, behavior, numfiles, lookInFlags);
}

char buf[1024];
char *__cdecl FS_ShiftStr(const char *string, char shift)
{
    signed int v2; // kr00_4
    int i; // [esp+14h] [ebp-4h]

    v2 = strlen(string);
    for (i = 0; i < v2; ++i)
        buf[i] = shift + string[i];
    buf[i] = 0;
    return buf;
}

int __cdecl FS_SV_FOpenFileRead(const char *filename, int *fp)
{
    FILE *Binary; // eax
    FILE *v3; // eax
    FILE *v4; // eax
    char *v6; // [esp+2Ch] [ebp-10Ch]
    char ospath[256]; // [esp+30h] [ebp-108h] BYREF
    int f; // [esp+134h] [ebp-4h]

    FS_CheckFileSystemStarted();
    f = FS_HandleForFile(FS_THREAD_MAIN);
    fsh[f].zipFile = 0;
    I_strncpyz(fsh[f].name, filename, 256);
    FS_BuildOSPath((char *)fs_homepath->current.integer, (char*)filename, (char *)"", ospath);
    v6 = ospath;
    v6 += strlen(v6) + 1;
    ospath[v6 - &ospath[1] - 1] = 0;
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_SV_FOpenFileRead (fs_homepath): %s\n", ospath);
    Binary = FS_FileOpenReadBinary(ospath);
    fsh[f].handleFiles.file.o = Binary;
    fsh[f].handleSync = 0;
    if (!fsh[f].handleFiles.file.o && I_stricmp(fs_homepath->current.string, fs_basepath->current.string))
    {
        FS_BuildOSPath((char *)fs_basepath->current.integer, (char*)filename, (char *)"", ospath);
        ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_SV_FOpenFileRead (fs_basepath): %s\n", ospath);
        v3 = FS_FileOpenReadBinary(ospath);
        fsh[f].handleFiles.file.o = v3;
        fsh[f].handleSync = 0;
        if (!fsh[f].handleFiles.file.o)
            f = 0;
    }
    if (!fsh[f].handleFiles.file.o)
    {
        FS_BuildOSPath((char *)fs_cdpath->current.integer, (char*)filename, (char *)"", ospath);
        ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_SV_FOpenFileRead (fs_cdpath) : %s\n", ospath);
        v4 = FS_FileOpenReadBinary(ospath);
        fsh[f].handleFiles.file.o = v4;
        fsh[f].handleSync = 0;
        if (!fsh[f].handleFiles.file.o)
            f = 0;
    }
    *fp = f;
    if (f)
        return FS_filelength(f);
    else
        return 0;
}

int __cdecl FS_SV_FOpenFileWrite(const char *filename)
{
    FILE *v2; // eax
    char *v3; // [esp+Ch] [ebp-10Ch]
    char ospath[256]; // [esp+10h] [ebp-108h] BYREF
    int f; // [esp+114h] [ebp-4h]

    FS_CheckFileSystemStarted();
    FS_BuildOSPath((char *)fs_homepath->current.integer, (char *)filename, (char *)"", ospath);
    v3 = ospath;
    v3 += strlen(v3) + 1;
    ospath[v3 - &ospath[1] - 1] = 0;
    f = FS_HandleForFile(FS_THREAD_MAIN);
    fsh[f].zipFile = 0;
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_SV_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    Com_DPrintf(10, "writing to: %s\n", ospath);
    v2 = FS_FileOpenWriteBinary(ospath);
    fsh[f].handleFiles.file.o = v2;
    I_strncpyz(fsh[f].name, (char *)filename, 256);
    fsh[f].handleSync = 0;
    if (!fsh[f].handleFiles.file.o)
        return 0;
    return f;
}

void __cdecl FS_CopyFile(char *fromOSPath, char *toOSPath)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    FILE *f; // [esp+8h] [ebp-4h]
    FILE *fa; // [esp+8h] [ebp-4h]

    f = FS_FileOpenReadBinary(fromOSPath);
    if (f)
    {
        len = FS_FileGetFileSize(f);
        buf = (unsigned __int8 *)malloc(len);
        if (FS_FileRead(buf, len, f) != len)
            Com_Error(ERR_FATAL, "Short read in FS_CopyFile()");
        FS_FileClose(f);
        if (FS_CreatePath(toOSPath) || (fa = FS_FileOpenWriteBinary(toOSPath)) == 0)
        {
            free(buf);
        }
        else
        {
            if (FS_FileWrite(buf, len, fa) != len)
                Com_Error(ERR_FATAL, "Short write in FS_CopyFile()");
            FS_FileClose(fa);
            free(buf);
        }
    }
}

void __cdecl FS_Remove(const char *osPath)
{
    remove(osPath);
}

void __cdecl FS_SV_Rename(char *from, char *to)
{
    char *v2; // [esp+1Ch] [ebp-20Ch]
    char to_ospath[256]; // [esp+20h] [ebp-208h] BYREF
    char from_ospath[260]; // [esp+120h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    FS_BuildOSPath((char *)fs_homepath->current.integer, from, (char *)"", from_ospath);
    FS_BuildOSPath((char *)fs_homepath->current.integer, to, (char *)"", to_ospath);
    v2 = from_ospath;
    v2 += strlen(v2) + 1;
    to_ospath[v2 - &from_ospath[1] + 255] = 0;
    to_ospath[&to_ospath[strlen(to_ospath) + 1] - &to_ospath[1] - 1] = 0;
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_SV_Rename: %s --> %s\n", from_ospath, to_ospath);
    if (rename(from_ospath, to_ospath))
    {
        FS_CopyFile(from_ospath, to_ospath);
        FS_Remove(from_ospath);
    }
}

int __cdecl FS_SV_FileExists(char *file)
{
    FILE *f; // [esp+10h] [ebp-10Ch]
    char testpath[260]; // [esp+14h] [ebp-108h] BYREF

    FS_BuildOSPath((char *)fs_homepath->current.integer, file, (char *)"", testpath);
    testpath[&testpath[strlen(testpath) + 1] - &testpath[1] - 1] = 0;
    f = FS_FileOpenReadBinary(testpath);
    if (!f)
        return 0;
    FS_FileClose(f);
    return 1;
}

void __cdecl FS_Restart(int localClientNum, int checksumFeed)
{
    const char *v2; // eax

    FS_Shutdown();
    fs_checksumFeed = checksumFeed;
    FS_ClearIwdReferences();
    ProfLoad_Begin("Start file system");
    FS_Startup((char*)"main");
    ProfLoad_End();
    ProfLoad_Begin("Init text localization");
    SEH_Init_StringEd();
    SEH_UpdateLanguageInfo();
    ProfLoad_End();
    ProfLoad_Begin("Set restrictions");
    FS_SetRestrictions();
    ProfLoad_End();
    ProfLoad_Begin("Default config");
    if (FS_ReadFile("default_mp.cfg", 0) <= 0)
    {
        if (lastValidBase[0])
        {
            FS_PureServerSetLoadedIwds((char *)"", (char *)"");
            Dvar_SetString((dvar_s *)fs_basepath, lastValidBase);
            Dvar_SetString((dvar_s *)fs_gameDirVar, lastValidGame);
            lastValidBase[0] = 0;
            lastValidGame[0] = 0;
            Dvar_SetBool((dvar_s *)fs_restrict, 0);
            FS_Restart(localClientNum, checksumFeed);
            Com_Error(ERR_DROP, "Invalid game folder\n");
        }
        Com_Error(ERR_FATAL, "Couldn't load %s.  Make sure Call of Duty is run from the correct folder.", "default_mp.cfg");
    }
    if (I_stricmp(fs_gameDirVar->current.string, lastValidGame) && !Com_SafeMode())
    {
        v2 = va("exec %s\n", "config_mp.cfg");
        Cbuf_AddText(0, v2);
    }
    I_strncpyz(lastValidBase, (char *)fs_basepath->current.integer, 256);
    I_strncpyz(lastValidGame, (char *)fs_gameDirVar->current.integer, 256);
    ProfLoad_End();
}

bool __cdecl FS_NeedRestart(int checksumFeed)
{
    if (com_sv_running->current.enabled)
        return 0;
    if (fs_gameDirVar->modified)
        return 1;
    return checksumFeed != fs_checksumFeed;
}

int __cdecl FS_FOpenFileWriteToDir(const char *filename, const  char *dir)
{
    return FS_FOpenFileWriteToDirForThread(filename, dir, FS_THREAD_MAIN);
}

int __cdecl FS_GetHandleAndOpenFile(const char *filename, const char *ospath, FsThread thread)
{
    int f; // [esp+0h] [ebp-8h]
    FILE *fp; // [esp+4h] [ebp-4h]

    fp = FS_FileOpenWriteBinary(ospath);
    if (!fp)
        return 0;
    f = FS_HandleForFile(thread);
    fsh[f].zipFile = 0;
    fsh[f].handleFiles.file.o = fp;
    I_strncpyz(fsh[f].name, filename, 256);
    fsh[f].handleSync = 0;
    return f;
}

int __cdecl FS_FOpenFileWriteToDirForThread(const char *filename, const char *dir, FsThread thread)
{
    char ospath[260]; // [esp+0h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    FS_BuildOSPath((char *)fs_homepath->current.integer, dir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    else
        return FS_GetHandleAndOpenFile(filename, ospath, thread);
}

int __cdecl FS_WriteFileToDir(const char *filename, const char *path, char *buffer, unsigned int size)
{
    int f; // [esp+0h] [ebp-8h]
    unsigned int actualSize; // [esp+4h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 2862, 0, "%s", "filename");
    if (!buffer)
        MyAssertHandler(".\\universal\\com_files.cpp", 2863, 0, "%s", "buffer");
    f = FS_FOpenFileWriteToDir(filename, path);
    if (f)
    {
        actualSize = FS_Write(buffer, size, f);
        FS_FCloseFile(f);
        if (actualSize == size)
        {
            return 1;
        }
        else
        {
            FS_Delete(filename);
            return 0;
        }
    }
    else
    {
        Com_Printf(10, "Failed to open %s\n", filename);
        return 0;
    }
}

void __cdecl FS_ShutdownSearchPaths(searchpath_s *p)
{
    searchpath_s *next; // [esp+0h] [ebp-4h]

    while (p)
    {
        next = p->next;
        if (p->iwd)
        {
            unzClose(p->iwd->handle);
            Z_Free(p->iwd->buildBuffer, 3);
            Z_Free(p->iwd->iwdFilename, 3);
        }
        if (p->dir)
            Z_Free(p->dir->path, 3);
        Z_Free(p, 3);
        p = next;
    }
}

void __cdecl FS_RemoveCommands()
{
    Cmd_RemoveCommand("path");
    Cmd_RemoveCommand("fullpath");
    Cmd_RemoveCommand("dir");
    Cmd_RemoveCommand("fdir");
    Cmd_RemoveCommand("touchFile");
}

void __cdecl FS_Shutdown()
{
    int i; // [esp+0h] [ebp-4h]

    SND_StopSounds(SND_STOP_STREAMED);
    SEH_Shutdown_StringEd();
    for (i = 1; i < 65; ++i)
    {
        if (fsh[i].fileSize)
            FS_FCloseFile(i);
    }
    FS_ShutdownSearchPaths(fs_searchpaths);
    fs_searchpaths = 0;
    FS_RemoveCommands();
}

bool __cdecl FS_DeleteInDir(char *filename, char *dir)
{
    char ospath[260]; // [esp+0h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    if (!filename)
        MyAssertHandler(".\\universal\\com_files.cpp", 2231, 0, "%s", "filename");
    if (!*filename)
        return 0;
    FS_BuildOSPath((char*)fs_homepath->current.integer, dir, filename, ospath);
    return remove(ospath) != -1;
}

void __cdecl FS_Rename(char *from, char *fromDir, char *to, char *toDir)
{
    char to_ospath[256]; // [esp+0h] [ebp-208h] BYREF
    char from_ospath[260]; // [esp+100h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    FS_BuildOSPath((char*)fs_homepath->current.integer, fromDir, from, from_ospath);
    FS_BuildOSPath((char*)fs_homepath->current.integer, toDir, to, to_ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_Rename: %s --> %s\n", from_ospath, to_ospath);
    if (rename(from_ospath, to_ospath))
    {
        FS_Remove(to_ospath);
        if (rename(from_ospath, to_ospath))
        {
            FS_CopyFile(from_ospath, to_ospath);
            FS_Remove(from_ospath);
        }
    }
}