#include "com_files.h"
#include "q_shared.h"

#include <universal/com_memory.h>

dvar_t *fs_remotePCDirectory;
dvar_t *fs_remotePCName;
int com_fileAccessed;
void *g_writeLogEvent;
int marker_com_files;
void *g_writeLogCompleteEvent;
int fs_loadStack;

static fileHandleData_t fsh[65];

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
        Com_Error(ERR_FATAL, &byte_8BF4B0);
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

void __cdecl FS_FCloseFile(int h)
{
    _iobuf *f; // [esp+0h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (dword_DB898B8[71 * h])
        MyAssertHandler(".\\universal\\com_files.cpp", 1275, 0, "%s", "!fsh[h].streamed");
    if (dword_DB898B4[71 * h])
    {
        unzCloseCurrentFile(fsh[h].handleFiles.file.z);
        if (dword_DB898A4[71 * h])
        {
            unzClose(fsh[h].handleFiles.file.z);
        }
        else
        {
            if (!*(unsigned int *)(dword_DB898B4[71 * h] + 780))
                MyAssertHandler(".\\universal\\com_files.cpp", 1287, 0, "%s", "fsh[h].zipFile->hasOpenFile");
            *(unsigned int *)(dword_DB898B4[71 * h] + 780) = 0;
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

int __cdecl FS_FOpenTextFileWrite(const char *filename)
{
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    _iobuf *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    FS_CheckFileSystemStarted();
    h = FS_HandleForFile(FS_THREAD_MAIN);
    dword_DB898B4[71 * h] = 0;
    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenWriteText(ospath);
    fsh[h].handleFiles.file.o = f;
    I_strncpyz(&byte_DB898BC[284 * h], filename, 256);
    dword_DB898A8[71 * h] = 0;
    if (!fsh[h].handleFiles.file.o)
        return 0;
    return h;
}

int __cdecl FS_FOpenFileAppend(char *filename)
{
    bool IsMainThread; // al
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    _iobuf *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\universal\\com_files.cpp", 1486, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    FS_CheckFileSystemStarted();
    IsMainThread = Sys_IsMainThread();
    h = FS_HandleForFile(IsMainThread ? FS_THREAD_MAIN : FS_THREAD_BACKEND);
    dword_DB898B4[71 * h] = 0;
    I_strncpyz(&byte_DB898BC[284 * h], filename, 256);
    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileAppend: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenAppendText(ospath);
    fsh[h].handleFiles.file.o = f;
    dword_DB898A8[71 * h] = 0;
    if (!fsh[h].handleFiles.file.o)
        return 0;
    return h;
}

unsigned int __cdecl FS_FOpenFileReadStream(const char *filename, int *file)
{
    return FS_FOpenFileReadForThread(filename, file, FS_THREAD_STREAM);
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
    _iobuf *f; // [esp+14h] [ebp-4h]

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
    if (dword_DB898A8[71 * h])
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
    _vsnprintf(&string, 0x1000u, fmt, va);
    FS_Write(&string, &v3[strlen(&string)] - v3, h);
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
        Com_Error(ERR_FATAL, &byte_8BF7D8);
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

void __cdecl FS_InitFilesystem()
{
    Com_StartupVariable("fs_cdpath");
    Com_StartupVariable(aFsB);
    Com_StartupVariable(aFsH);
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
        Com_Error(ERR_FATAL, &byte_8BFFCC);
        break;
    }
    if (!f)
        return r;
    if (*f)
    {
        dword_DB898AC[71 * *f] = r;
        dword_DB898B8[71 * *f] = 0;
    }
    dword_DB898A8[71 * *f] = sync;
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
        extension = &String;
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