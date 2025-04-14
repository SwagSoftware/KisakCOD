#include "com_files.h"
#include "q_shared.h"

#include <universal/com_memory.h>
#include <qcommon/com_fileaccess.h>
#include <qcommon/qcommon.h>
#include <win32/win_local.h>
#include <qcommon/threads.h>
#include <stringed/stringed_hooks.h>

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

int fs_numServerIwds;
int fs_serverIwds[1024];

int com_fileAccessed;
void *g_writeLogEvent;
int marker_com_files;
void *g_writeLogCompleteEvent;
int fs_loadStack;

static fileHandleData_t fsh[65];

char fs_gamedir[256];
searchpath_s *fs_searchpaths;

void __cdecl FS_CheckFileSystemStarted()
{
    if (!fs_searchpaths)
        MyAssertHandler(".\\universal\\com_files.cpp", 708, 0, "%s", "fs_searchpaths");
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

iobuf *__cdecl FS_FileForHandle(int f)
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
    _iobuf *h; // [esp+4h] [ebp-4h]

    if (!f)
        MyAssertHandler(".\\universal\\com_files.cpp", 968, 0, "%s", "f");
    FS_CheckFileSystemStarted();

    if (fsh[f].zipFile)
        return fsh[f].handleFiles.file.o[2]._cnt;

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

void __cdecl FS_BuildOSPath(char *base, char *game, char *qpath, char *ospath)
{
    FS_BuildOSPathForThread(base, game, qpath, ospath, FS_THREAD_MAIN);
}

void __cdecl FS_BuildOSPathForThread(char *base, char *game, char *qpath, char *ospath, FsThread thread)
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
    int v1; // eax
    int v2; // eax
    char *ofs; // [esp+0h] [ebp-4h]

    strstr(OSPath, "..");
    if (v1 || (strstr(OSPath, "::"), v2))
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

void __cdecl FS_FileClose(iobuf *stream)
{
    fclose(stream);
}

void __cdecl FS_FCloseFile(int h)
{
    iobuf *f; // [esp+0h] [ebp-4h]

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

int __cdecl FS_FOpenFileWrite(char *filename)
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

int __cdecl FS_FOpenTextFileWrite(char *filename)
{
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    iobuf *f; // [esp+108h] [ebp-8h]
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

int __cdecl FS_FOpenFileAppend(char *filename)
{
    bool IsMainThread; // al
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    iobuf *f; // [esp+108h] [ebp-8h]
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
    _iobuf *filetemp; // [esp+32Ch] [ebp-4h]

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
                v4 = SEH_SafeTranslateString("EXE_UNPURECLIENTDETECTED");
                v5 = va(off_8BF65C, v4, v6);
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
                    I_strncpyz(&byte_DB898BC[284 * *file], sanitizedName, 256);
                    dword_DB898B4[71 * *file] = 0;
                    if (fs_debug->current.integer && thread == FS_THREAD_MAIN)
                        Com_Printf(10, "FS_FOpenFileRead: %s (found in '%s/%s')\n", sanitizedName, dir->path, dir->gamedir);
                    if (fs_copyfiles->current.enabled && !I_stricmp(dir->path, fs_cdpath->current.string))
                    {
                        FS_BuildOSPathForThread((char *)fs_basepath->current.integer, dir->gamedir, sanitizedName, copypath, thread);
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
        dword_DB898A4[71 * *file] = 1;
        fsh[*file].handleFiles.file.o = (_iobuf *)unzReOpen(iwd->iwdFilename, iwd->handle);
        if (!fsh[*file].handleFiles.file.o)
        {
            if (thread)
            {
                FS_FCloseFile(*file);
                *file = 0;
                return -1;
            }
            Com_Error(ERR_FATAL, &byte_8BF6C8, iwd);
        }
    }
    else
    {
        dword_DB898A4[71 * *file] = 0;
        fsh[*file].handleFiles.file.o = (_iobuf *)iwd->handle;
    }
    I_strncpyz(&byte_DB898BC[284 * *file], sanitizedName, 256);
    dword_DB898B4[71 * *file] = (int)iwd;
    zfi = (unz_s *)fsh[*file].handleFiles.file.o;
    filetemp = zfi->file;
    ziptemp = zfi->pfile_in_zip_read;
    unzSetCurrentFileInfoPosition(iwd->handle, iwdFile->pos);
    Com_Memcpy((char *)zfi, (char *)iwd->handle, 128);
    zfi->file = filetemp;
    zfi->pfile_in_zip_read = ziptemp;
    unzOpenCurrentFile(fsh[*file].handleFiles.file.z);
    dword_DB898B0[71 * *file] = iwdFile->pos;
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

bool __cdecl FS_Delete(char *filename)
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
    _iobuf *f; // [esp+10h] [ebp-8h]
    int read; // [esp+14h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!h)
        return 0;
    if (dword_DB898B4[71 * h])
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
            Com_Error(ERR_FATAL, &byte_8BF7A0);
        }
        remaining -= read;
        buf += read;
    }
    return len;
}

unsigned int __cdecl FS_Write(char *buffer, unsigned int len, int h)
{
    int tries; // [esp+4h] [ebp-14h]
    unsigned int remaining; // [esp+8h] [ebp-10h]
    int written; // [esp+10h] [ebp-8h]
    iobuf *f; // [esp+14h] [ebp-4h]

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

unsigned int __cdecl FS_WriteLog(char *buffer, unsigned int len, int h)
{
    return FS_Write(buffer, len, h);
}

void FS_Printf(int h, const char *fmt, ...)
{
    char string; // [esp+14h] [ebp-1008h] BYREF
    _BYTE v3[3]; // [esp+15h] [ebp-1007h] BYREF
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    _vsnprintf("", 0x1000u, fmt, va);
    FS_Write("", &v3[strlen("")] - v3, h);
}

int __cdecl FS_Seek(int f, int offset, int origin)
{
    unsigned int CurrentFile; // eax
    const char *v5; // eax
    _iobuf *v6; // eax
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
    iobuf *f; // [esp+0h] [ebp-10Ch]
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
    strstr((unsigned __int8 *)newValue.integer, (unsigned __int8 *)"..");
    result = 0;
    if (!v3)
    {
        strstr((unsigned __int8 *)newValue.integer, (unsigned __int8 *)"::");
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
    unsigned int *iwd; // [esp+88h] [ebp-16Ch]
    unsigned int len; // [esp+8Ch] [ebp-168h]
    fileInIwd_s *buildBuffer; // [esp+90h] [ebp-164h]
    unz_file_info_s file_info; // [esp+94h] [ebp-160h] BYREF
    char filename_inzip[256]; // [esp+E4h] [ebp-110h] BYREF
    unsigned int i; // [esp+1E8h] [ebp-Ch]
    unz_global_info_s gi; // [esp+1ECh] [ebp-8h] BYREF

    fs_numHeaderLongs = 0;
    uf = (unsigned __int8 *)unzOpen(zipfile);
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
    buildBuffer = (fileInIwd_s *)Z_Malloc(len + 12 * gi.number_entry, "FS_LoadZipFile1", 3);
    namePtr = (char *)&buildBuffer[gi.number_entry];
    fs_headerLongs = (int *)Z_Malloc(4 * gi.number_entry, "FS_LoadZipFile2", 3);
    for (i = 1; i <= 0x400 && i <= gi.number_entry; i *= 2)
        ;
    iwd = Z_Malloc(4 * i + 804, "FS_LoadZipFile3", 3);
    iwd[198] = i;
    iwd[199] = (unsigned int)(iwd + 201);
    for (i = 0; i < iwd[198]; ++i)
        *(_DWORD *)(iwd[199] + 4 * i) = 0;
    I_strncpyz((char *)iwd, zipfile, 256);
    I_strncpyz((char *)iwd + 256, basename, 256);
    if (strlen((const char *)iwd + 256) > 4
        && !I_stricmp((const char *)iwd + strlen((const char *)iwd + 256) + 252, ".iwd"))
    {
        *((_BYTE *)iwd + strlen((const char *)iwd + 256) + 252) = 0;
    }
    iwd[192] = (unsigned int)uf;
    iwd[196] = gi.number_entry;
    iwd[195] = 0;
    unzGoToFirstFile(uf);
    for (i = 0; i < gi.number_entry && !unzGetCurrentFileInfo(uf, &file_info, filename_inzip, 0x100u, 0, 0, 0, 0); ++i)
    {
        if (file_info.uncompressed_size)
            fs_headerLongs[fs_numHeaderLongs++] = file_info.crc;
        I_strlwr(filename_inzip);
        hash = FS_HashFileName(filename_inzip, iwd[198]);
        buildBuffer[i].name = namePtr;
        v5 = filename_inzip;
        name = buildBuffer[i].name;
        do
        {
            v3 = *v5;
            *name++ = *v5++;
        } while (v3);
        namePtr += &filename_inzip[strlen(filename_inzip) + 1] - &filename_inzip[1] + 1;
        unzGetCurrentFileInfoPosition(uf, &buildBuffer[i].pos);
        buildBuffer[i].next = *(fileInIwd_s **)(iwd[199] + 4 * hash);
        *(_DWORD *)(iwd[199] + 4 * hash) = &buildBuffer[i];
        unzGoToNextFile(uf);
    }
    iwd[193] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * fs_numHeaderLongs, 0);
    if (fs_checksumFeed)
        iwd[194] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * fs_numHeaderLongs, fs_checksumFeed);
    else
        iwd[194] = iwd[193];
    iwd[193] = iwd[193];
    iwd[194] = iwd[194];
    Z_Free((char *)fs_headerLongs, 3);
    iwd[200] = (unsigned int)buildBuffer;
    return (iwd_t *)iwd;
}

char szIwdLanguageName[2][64];
int iString;
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
            *(_BYTE *)((iString << 6) + iCurrChar + 230219974) = pszIwdFileName[iCurrChar];
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
            *(_DWORD *)v2 = *(_DWORD *)asc_8BFD3C;
            *((_DWORD *)v2 + 1) = 538976288;
            *((_WORD *)v2 + 4) = 8224;
        }
    }
    qsort(s0, numfiles, 4u, (int(__cdecl *)(const void *, const void *))iwdsort);
    for (i = 0; i < numfiles; ++i)
    {
        if (I_strncmp(s0[i], asc_8BFD3C, 10))
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
    v4 = Z_Malloc(512, "FS_AddGameDirectory", 3);
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

void __cdecl FS_Startup(char *gameName)
{
    char *v2; // eax
    char *v3; // eax

    Com_Printf(10, "----- FS_Startup -----\n");
    FS_RegisterDvars();
    if (*(_BYTE *)fs_basepath->current.integer)
    {
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "devraw");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "raw");
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "players");
    }
    if (*(_BYTE *)fs_homepath->current.integer && I_stricmp(fs_basepath->current.string, fs_homepath->current.string))
    {
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, "devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, "devraw");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, "raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, "raw");
    }
    if (*(_BYTE *)fs_cdpath->current.integer && I_stricmp(fs_basepath->current.string, fs_cdpath->current.string))
    {
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, "devraw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, "devraw");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, "raw_shared");
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, "raw");
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
    Dvar_ClearModified(fs_gameDirVar);
    Com_Printf(10, "----------------------\n");
    Com_Printf(10, "%d files in iwd files\n", fs_iwdFileCount);
}

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
    FS_Startup("main");
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
    _iobuf *v1; // eax

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
    iobuf *v1; // eax

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
    iobuf *file; // [esp+60h] [ebp-130h]
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
    zpath[len] = 0;
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

int __cdecl FS_SV_FOpenFileRead(const char *filename, int *fp)
{
    iobuf *Binary; // eax
    iobuf *v3; // eax
    iobuf *v4; // eax
    char *v6; // [esp+2Ch] [ebp-10Ch]
    char ospath[256]; // [esp+30h] [ebp-108h] BYREF
    int f; // [esp+134h] [ebp-4h]

    FS_CheckFileSystemStarted();
    f = FS_HandleForFile(FS_THREAD_MAIN);
    fsh[f].zipFile = 0;
    I_strncpyz(fsh[f].name, filename, 256);
    FS_BuildOSPath((char *)fs_homepath->current.integer, filename, (char *)"", ospath);
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
        FS_BuildOSPath((char *)fs_basepath->current.integer, filename, (char *)"", ospath);
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
        FS_BuildOSPath((char *)fs_cdpath->current.integer, filename, (char *)"", ospath);
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
    iobuf *v2; // eax
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

int __cdecl FS_FOpenFileWriteToDir(char *filename, char *dir)
{
    return FS_FOpenFileWriteToDirForThread(filename, dir, FS_THREAD_MAIN);
}

int __cdecl FS_GetHandleAndOpenFile(char *filename, const char *ospath, FsThread thread)
{
    int f; // [esp+0h] [ebp-8h]
    iobuf *fp; // [esp+4h] [ebp-4h]

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

int __cdecl FS_FOpenFileWriteToDirForThread(char *filename, char *dir, FsThread thread)
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

int __cdecl FS_WriteFileToDir(char *filename, char *path, char *buffer, unsigned int size)
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