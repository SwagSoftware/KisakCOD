#include <qcommon/qcommon.h>
#include "com_files.h"

char *__cdecl Com_LoadInfoString(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer)
{
    char *buffer; // [esp+20h] [ebp-4h]

    if (useFastFile->current.enabled)
        buffer = (char *)Com_LoadInfoString_FastFile(fileName, fileDesc, ident);
    else
        buffer = Com_LoadInfoString_LoadObj(fileName, fileDesc, ident, loadBuffer);
    if (!Info_Validate(buffer))
        Com_Error(ERR_DROP, "File [%s] is not a valid %s", fileName, fileDesc);
    return buffer;
}

const char *__cdecl Com_LoadInfoString_FastFile(const char *fileName, const char *fileDesc, const char *ident)
{
    unsigned int v4; // [esp+0h] [ebp-20h]
    const char *buffer; // [esp+14h] [ebp-Ch]
    RawFile *rawfile; // [esp+1Ch] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fileName).rawfile;
    if (!rawfile)
        Com_Error(ERR_DROP, "Could not load %s file [%s]", fileDesc, fileName);
    buffer = rawfile->buffer;
    v4 = strlen(ident);
    if (strncmp(buffer, ident, v4))
        Com_Error(ERR_DROP, "File [%s] is not a %s file", fileName, fileDesc);
    return &buffer[v4];
}

char *__cdecl Com_LoadInfoString_LoadObj(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer)
{
    unsigned int v5; // [esp+0h] [ebp-1Ch]
    int fileHandle; // [esp+14h] [ebp-8h] BYREF
    int fileLen; // [esp+18h] [ebp-4h]

    fileLen = FS_FOpenFileByMode(fileName, &fileHandle, FS_READ);
    if (fileLen < 0)
        Com_Error(ERR_DROP, "Could not load %s [%s]", fileDesc, fileName);
    v5 = strlen(ident);
    FS_Read((unsigned __int8 *)loadBuffer, v5, fileHandle);
    loadBuffer[v5] = 0;
    if (strncmp(loadBuffer, ident, v5))
        Com_Error(ERR_DROP, "File [%s] is not a %s", fileName, fileDesc);
    if ((int)(fileLen - v5) >= 0x2000)
        Com_Error(ERR_DROP, "File [%s] is too long of a %s to parse", fileName, fileDesc);
    FS_Read((unsigned __int8 *)loadBuffer, fileLen - v5, fileHandle);
    loadBuffer[fileLen - v5] = 0;
    FS_FCloseFile(fileHandle);
    return loadBuffer;
}

char *__cdecl Com_LoadRawTextFile(const char *fullpath)
{
    if (useFastFile->current.enabled)
        return (char *)Com_LoadRawTextFile_FastFile(fullpath);
    else
        return Com_LoadRawTextFile_LoadObj(fullpath);
}

XModelPiece *__cdecl Com_LoadRawTextFile_FastFile(const char *fullpath)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fullpath).rawfile;
    if (rawfile)
        return (XModelPiece *)rawfile->buffer;
    else
        return 0;
}

char *__cdecl Com_LoadRawTextFile_LoadObj(const char *fullpath)
{
    char *filebuf; // [esp+4h] [ebp-4h] BYREF

    if (FS_ReadFile(fullpath, (void **)&filebuf) >= 0)
        return filebuf;
    else
        return 0;
}

void __cdecl Com_UnloadRawTextFile(char *filebuf)
{
    if (!useFastFile->current.enabled)
        FS_FreeFile(filebuf);
}

