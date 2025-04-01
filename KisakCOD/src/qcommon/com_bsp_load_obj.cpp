#include "com_bsp.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>

BspGlob comBspGlob;

char *__cdecl Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int *count)
{
    unsigned int chunkIter; // [esp+0h] [ebp-10h]
    unsigned int offset; // [esp+4h] [ebp-Ch]

    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 92, 0, "%s", "Com_IsBspLoaded()");
    if (comBspGlob.header->version > 0x12)
    {
        offset = 8 * comBspGlob.header->chunkCount + 12;
        for (chunkIter = 0; chunkIter < comBspGlob.header->chunkCount; ++chunkIter)
        {
            if (comBspGlob.header->chunks[chunkIter].type == type)
                return Com_ValidateBspLumpData(type, offset, comBspGlob.header->chunks[chunkIter].length, elemSize, count);
            offset += (comBspGlob.header->chunks[chunkIter].length + 3) & 0xFFFFFFFC;
        }
        *count = 0;
        return 0;
    }
    else if (type < (unsigned int)Com_GetBspLumpCountForVersion(comBspGlob.header->version))
    {
        return Com_ValidateBspLumpData(
            type,
            comBspGlob.header->chunks[type].type,
            *(&comBspGlob.header->chunkCount + 2 * type),
            elemSize,
            count);
    }
    else
    {
        *count = 0;
        return 0;
    }
}

BOOL __cdecl Com_IsBspLoaded()
{
    return comBspGlob.header != 0;
}

void __cdecl Com_LoadBsp(char *filename)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    unsigned int v4; // edx
    unsigned int bytesRead; // [esp+18h] [ebp-Ch]
    unsigned int len; // [esp+1Ch] [ebp-8h]
    int h; // [esp+20h] [ebp-4h] BYREF

    if (!filename)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 304, 0, "%s", "filename");
    if (Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 305, 0, "%s", "!Com_IsBspLoaded()");
    if (comBspGlob.loadedLumpData)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 306, 0, "%s", "comBspGlob.loadedLumpData == NULL");
    ProfLoad_Begin("Load bsp file");
    comBspGlob.fileSize = FS_FOpenFileRead(filename, &h);
    if (!h)
    {
        v1 = va("EXE_ERR_COULDNT_LOAD %s", filename);
        Com_Error(ERR_DROP, v1);
    }
    comBspGlob.header = (BspHeader *)Z_MallocGarbage(comBspGlob.fileSize, "Com_LoadBsp", 10);
    bytesRead = FS_Read((unsigned __int8 *)comBspGlob.header, comBspGlob.fileSize, h);
    FS_FCloseFile(h);
    if (bytesRead != comBspGlob.fileSize)
    {
        Z_Free((char *)comBspGlob.header, 10);
        v2 = va("EXE_ERR_COULDNT_LOAD %s", filename);
        Com_Error(ERR_DROP, v2);
    }
    ProfLoad_Begin("Bsp checksum");
    comBspGlob.checksum = Com_BlockChecksumKey32((const unsigned __int8 *)comBspGlob.header, comBspGlob.fileSize, 0);
    ProfLoad_End();
    if (Com_BspError())
    {
        Z_Free((char *)comBspGlob.header, 10);
        comBspGlob.header = 0;
        v3 = va("EXE_ERR_WRONG_MAP_VERSION_NUM %s", filename);
        Com_Error(ERR_DROP, v3);
    }
    ProfLoad_End();
    v4 = strlen(filename);
    len = v4;
    if (v4 >= 0x40)
        MyAssertHandler(
            ".\\qcommon\\com_bsp_load_obj.cpp",
            337,
            0,
            "%s\n\t(len) = %i",
            "(len < (sizeof( comBspGlob.name ) / (sizeof( comBspGlob.name[0] ) * (sizeof( comBspGlob.name ) != 4 || sizeof( com"
            "BspGlob.name[0] ) <= 4))))",
            v4);
    memcpy((unsigned __int8 *)&comBspGlob, (unsigned __int8 *)filename, len + 1);
    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 340, 0, "%s", "Com_IsBspLoaded()");
}

void __cdecl Com_UnloadBsp()
{
    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 346, 0, "%s", "Com_IsBspLoaded()");
    if (comBspGlob.loadedLumpData)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 347, 0, "%s", "comBspGlob.loadedLumpData == NULL");
    ProfLoad_Begin("Unload bsp file");
    Z_Free((char *)comBspGlob.header, 10);
    comBspGlob.header = 0;
    comBspGlob.name[0] = 0;
    ProfLoad_End();
    if (Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 357, 0, "%s", "!Com_IsBspLoaded()");
}

const char *__cdecl Com_GetBspLumpCountForVersion(int version)
{
    if (version < 6 || version > 18)
        MyAssertHandler(
            ".\\qcommon\\com_bsp_load_obj.cpp",
            58,
            0,
            "version not in [OLDEST_BSP_VERSION, 18]\n\t%i not in [%i, %i]",
            version,
            6,
            18);
    return WeaponStateNames[version + 21];
}

char *__cdecl Com_ValidateBspLumpData(
    LumpType type,
    unsigned int offset,
    unsigned int length,
    unsigned int elemSize,
    unsigned int *count)
{
    if (!count)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 66, 0, "%s", "count");
    if (length)
    {
        if (length + offset > comBspGlob.fileSize)
            Com_Error(ERR_DROP, "LoadMap: lump %i extends past end of file", type);
        *count = length / elemSize;
        if (elemSize * *count != length)
            Com_Error(ERR_DROP, "LoadMap: lump %i has funny size", type);
        return (char *)comBspGlob.header + offset;
    }
    else
    {
        *count = 0;
        return 0;
    }
}

int __cdecl Com_BlockChecksumKey32(const unsigned __int8 *data, unsigned int length, unsigned int initialCrc)
{
    const unsigned __int8 *dataIter; // [esp+8h] [ebp-8h]
    unsigned int crc; // [esp+Ch] [ebp-4h]
    unsigned int crca; // [esp+Ch] [ebp-4h]
    unsigned int crcb; // [esp+Ch] [ebp-4h]
    unsigned int crcc; // [esp+Ch] [ebp-4h]
    unsigned int crcd; // [esp+Ch] [ebp-4h]
    unsigned int crce; // [esp+Ch] [ebp-4h]
    unsigned int crcf; // [esp+Ch] [ebp-4h]

    crc = ~initialCrc;
    for (dataIter = data; dataIter != &data[length]; ++dataIter)
    {
        crca = (-306674912 * (((unsigned __int8)crc ^ *dataIter) & 1)) ^ ((crc ^ *dataIter) >> 1);
        crcb = (-306674912 * (crca & 1)) ^ (crca >> 1);
        crcc = (-306674912 * (crcb & 1)) ^ (crcb >> 1);
        crcd = (-306674912 * (crcc & 1)) ^ (crcc >> 1);
        crce = (-306674912 * (crcd & 1)) ^ (crcd >> 1);
        crcf = (-306674912 * (crce & 1)) ^ (crce >> 1);
        crc = (-306674912 * (((unsigned __int8)(32 * (crcf & 1)) ^ (unsigned __int8)(crcf >> 1)) & 1))
            ^ (((-306674912 * (crcf & 1)) ^ (crcf >> 1)) >> 1);
    }
    return ~crc;
}