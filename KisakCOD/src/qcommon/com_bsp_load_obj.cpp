#include "com_bsp.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <database/database.h>

BspGlob comBspGlob;

unsigned int __cdecl Com_GetBspVersion()
{
    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 253, 0, "%s", "Com_IsBspLoaded()");
    return comBspGlob.header->version;
}

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

char *__cdecl Com_EntityString(int *numEntityChars)
{
    char *entityString; // [esp+0h] [ebp-8h]
    unsigned int count; // [esp+4h] [ebp-4h] BYREF

    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 377, 0, "%s", "Com_IsBspLoaded()");
    entityString = Com_GetBspLump(LUMP_ENTITIES, 1u, &count);
    if (numEntityChars)
        *numEntityChars = count;
    return entityString;
}

const char *__cdecl Com_GetHunkStringCopy(char *string)
{
    unsigned int v2; // [esp+0h] [ebp-18h]
    unsigned __int8 *hunkCopy; // [esp+14h] [ebp-4h]

    v2 = strlen(string);
    hunkCopy = Hunk_AllocAlign(v2 + 1, 1, "Com_GetLightDefName", 12);
    memcpy(hunkCopy, (unsigned __int8 *)string, v2 + 1);
    return (const char *)hunkCopy;
}

const char *__cdecl Com_GetLightDefName(
    char *defName,
    const ComPrimaryLight *primaryLights,
    unsigned int primaryLightCount)
{
    unsigned int primaryLightIndex; // [esp+14h] [ebp-8h]

    for (primaryLightIndex = 0; primaryLightIndex < primaryLightCount; ++primaryLightIndex)
    {
        if (primaryLights[primaryLightIndex].defName && !strcmp(defName, primaryLights[primaryLightIndex].defName))
            return primaryLights[primaryLightIndex].defName;
    }
    return Com_GetHunkStringCopy(defName);
}

ComPrimaryLight *Com_LoadPrimaryLights_Version14()
{
    ComPrimaryLight *result; // eax

    comWorld.primaryLightCount = 2;
    comWorld.primaryLights = (ComPrimaryLight *)Hunk_Alloc(0x88u, "Com_LoadPrimaryLights", 12);
    if (comWorld.primaryLights->type)
        MyAssertHandler(
            ".\\qcommon\\com_bsp_load_obj.cpp",
            508,
            1,
            "%s",
            "comWorld.primaryLights[0].type == GFX_LIGHT_TYPE_NONE");
    result = comWorld.primaryLights;
    comWorld.primaryLights[1].type = 1;
    return result;
}

const DiskPrimaryLight_Version16 *Com_LoadPrimaryLights_Version16()
{
    const DiskPrimaryLight_Version16 *result; // eax
    const float *origin; // [esp+4h] [ebp-2Ch]
    const float *dir; // [esp+Ch] [ebp-24h]
    const float *color; // [esp+14h] [ebp-1Ch]
    int exponent; // [esp+18h] [ebp-18h]
    ComPrimaryLight *out; // [esp+1Ch] [ebp-14h]
    const DiskPrimaryLight_Version16 *diskLights; // [esp+20h] [ebp-10h]
    unsigned int diskLightCount; // [esp+24h] [ebp-Ch] BYREF
    const DiskPrimaryLight_Version16 *in; // [esp+28h] [ebp-8h]
    unsigned int lightIndex; // [esp+2Ch] [ebp-4h]

    diskLights = (const DiskPrimaryLight_Version16 *)Com_GetBspLump(LUMP_PRIMARY_LIGHTS, 0x60u, &diskLightCount);
    if (diskLightCount <= 1)
        Com_Error(ERR_DROP, "no primary lights in bsp\n");
    comWorld.primaryLightCount = diskLightCount;
    comWorld.primaryLights = (ComPrimaryLight *)Hunk_Alloc(68 * diskLightCount, "Com_LoadPrimaryLights", 12);
    result = diskLights;
    in = diskLights;
    out = comWorld.primaryLights;
    lightIndex = 0;
    while (lightIndex < diskLightCount)
    {
        out->type = in->type;
        out->canUseShadowMap = 0;
        exponent = in->exponent;
        if (exponent != (unsigned __int8)exponent)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                exponent,
                (unsigned __int8)exponent);
        out->exponent = exponent;
        color = in->color;
        out->color[0] = in->color[0];
        out->color[1] = color[1];
        out->color[2] = color[2];
        dir = in->dir;
        out->dir[0] = in->dir[0];
        out->dir[1] = dir[1];
        out->dir[2] = dir[2];
        origin = in->origin;
        out->origin[0] = in->origin[0];
        out->origin[1] = origin[1];
        out->origin[2] = origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->cosHalfFovExpanded = out->cosHalfFovOuter;
        out->rotationLimit = 1.0;
        out->translationLimit = 0.0;
        if (in->type && in->type != 1)
            out->defName = Com_GetLightDefName((char*)in->defName, comWorld.primaryLights, lightIndex);
        else
            out->defName = 0;
        ++lightIndex;
        result = ++in;
        ++out;
    }
    return result;
}

ComPrimaryLight *Com_LoadPrimaryLights()
{
    ComPrimaryLight *result; // eax
    const float *origin; // [esp+14h] [ebp-2Ch]
    const float *dir; // [esp+1Ch] [ebp-24h]
    const float *color; // [esp+24h] [ebp-1Ch]
    int exponent; // [esp+28h] [ebp-18h]
    ComPrimaryLight *out; // [esp+2Ch] [ebp-14h]
    const DiskPrimaryLight *diskLights; // [esp+30h] [ebp-10h]
    unsigned int diskLightCount; // [esp+34h] [ebp-Ch] BYREF
    const DiskPrimaryLight *in; // [esp+38h] [ebp-8h]
    unsigned int lightIndex; // [esp+3Ch] [ebp-4h]

    if (comBspGlob.header->version <= 0xE)
        return Com_LoadPrimaryLights_Version14();
    if (comBspGlob.header->version <= 0x10)
        return (ComPrimaryLight *)Com_LoadPrimaryLights_Version16();
    diskLights = (const DiskPrimaryLight *)Com_GetBspLump(LUMP_PRIMARY_LIGHTS, 0x80u, &diskLightCount);
    if (diskLightCount <= 1)
        Com_Error(ERR_DROP, "no primary lights in bsp\n");
    comWorld.primaryLightCount = diskLightCount;
    comWorld.primaryLights = (ComPrimaryLight *)Hunk_Alloc(68 * diskLightCount, "Com_LoadPrimaryLights", 12);
    in = diskLights;
    result = comWorld.primaryLights;
    out = comWorld.primaryLights;
    lightIndex = 0;
    while (lightIndex < diskLightCount)
    {
        out->type = in->type;
        out->canUseShadowMap = in->canUseShadowMap;
        exponent = in->exponent;
        if (exponent != (unsigned __int8)exponent)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                exponent,
                (unsigned __int8)exponent);
        out->exponent = exponent;
        color = in->color;
        out->color[0] = in->color[0];
        out->color[1] = color[1];
        out->color[2] = color[2];
        dir = in->dir;
        out->dir[0] = in->dir[0];
        out->dir[1] = dir[1];
        out->dir[2] = dir[2];
        origin = in->origin;
        out->origin[0] = in->origin[0];
        out->origin[1] = origin[1];
        out->origin[2] = origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->rotationLimit = in->rotationLimit;
        out->translationLimit = in->translationLimit;
        if (in->type && in->type != 1)
        {
            out->defName = Com_GetLightDefName((char*)in->defName, comWorld.primaryLights, lightIndex);
            if (out->cosHalfFovOuter >= (double)out->cosHalfFovInner)
                out->cosHalfFovInner = out->cosHalfFovOuter * 0.75 + 0.25;
            if (out->rotationLimit == 1.0)
            {
                out->cosHalfFovExpanded = out->cosHalfFovOuter;
            }
            else if (out->rotationLimit > -out->cosHalfFovOuter)
            {
                out->cosHalfFovExpanded = CosOfSumOfArcCos(out->cosHalfFovOuter, out->rotationLimit);
            }
            else
            {
                out->cosHalfFovExpanded = -1.0;
            }
        }
        else
        {
            out->defName = 0;
            out->cosHalfFovExpanded = in->cosHalfFovOuter;
        }
        ++lightIndex;
        ++in;
        result = ++out;
    }
    return result;
}

void __cdecl Com_LoadWorld_LoadObj(char *name)
{
    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 623, 0, "%s", "Com_IsBspLoaded()");
    Com_LoadPrimaryLights();
    comWorld.name = Com_GetHunkStringCopy(name);
    comWorld.isInUse = 1;
}

void __cdecl Com_LoadWorld(char *name)
{
    if (useFastFile->current.enabled)
        Com_LoadWorld_FastFile(name);
    else
        Com_LoadWorld_LoadObj(name);
}

void __cdecl Com_LoadWorld_FastFile(const char *name)
{
    if (DB_FindXAssetHeader(ASSET_TYPE_COMWORLD, name).comWorld != &comWorld)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 640, 0, "%s", "asset == &comWorld");
    if (!comWorld.isInUse)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 641, 0, "%s", "comWorld.isInUse");
}

void __cdecl Com_ShutdownWorld()
{
    comWorld.isInUse = 0;
}

void __cdecl Com_SaveLump(LumpType type, const void *newLump, unsigned int size, ComSaveLumpBehavior behavior)
{
    char v4; // [esp+3h] [ebp-535h]
    char *v5; // [esp+8h] [ebp-530h]
    BspGlob *v6; // [esp+Ch] [ebp-52Ch]
    char savedName[68]; // [esp+10h] [ebp-528h] BYREF
    unsigned int chunkIter; // [esp+58h] [ebp-4E0h]
    unsigned int zero; // [esp+5Ch] [ebp-4DCh] BYREF
    BspHeader newHeader; // [esp+60h] [ebp-4D8h] BYREF
    unsigned int offset; // [esp+394h] [ebp-1A4h]
    const BspChunk *chunk; // [esp+398h] [ebp-1A0h]
    bool isNewChunk; // [esp+39Fh] [ebp-199h]
    const void *chunkData[100]; // [esp+3A0h] [ebp-198h]
    int h; // [esp+530h] [ebp-8h]
    unsigned int zeroCount; // [esp+534h] [ebp-4h]

    if (!Com_IsBspLoaded())
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 399, 0, "%s", "Com_IsBspLoaded()");
    if (comBspGlob.header->version != 22)
        MyAssertHandler(".\\qcommon\\com_bsp_load_obj.cpp", 400, 0, "%s", "comBspGlob.header->version == BSP_VERSION");
    if (comBspGlob.header->chunkCount > 0x64)
        MyAssertHandler(
            ".\\qcommon\\com_bsp_load_obj.cpp",
            402,
            0,
            "comBspGlob.header->chunkCount <= ARRAY_COUNT( newHeader.chunks )\n\t%i, %i",
            comBspGlob.header->chunkCount,
            100);
    newHeader.ident = 1347633737;
    newHeader.version = 22;
    newHeader.chunkCount = 0;
    isNewChunk = 1;
    offset = 8 * comBspGlob.header->chunkCount + 12;
    for (chunkIter = 0; chunkIter < comBspGlob.header->chunkCount; ++chunkIter)
    {
        chunk = &comBspGlob.header->chunks[chunkIter];
        if (chunk->type == type)
        {
            isNewChunk = 0;
            if (size)
            {
                newHeader.chunks[newHeader.chunkCount].type = type;
                newHeader.chunks[newHeader.chunkCount].length = size;
                chunkData[newHeader.chunkCount++] = newLump;
            }
        }
        else
        {
            newHeader.chunks[newHeader.chunkCount].type = chunk->type;
            newHeader.chunks[newHeader.chunkCount].length = chunk->length;
            chunkData[newHeader.chunkCount++] = comBspGlob.header + offset;
        }
        offset += (chunk->length + 3) & 0xFFFFFFFC;
    }
    if (isNewChunk && size)
    {
        if (newHeader.chunkCount >= 0x64)
            MyAssertHandler(
                ".\\qcommon\\com_bsp_load_obj.cpp",
                436,
                0,
                "newHeader.chunkCount < ARRAY_COUNT( newHeader.chunks )\n\t%i, %i",
                newHeader.chunkCount,
                100);
        newHeader.chunks[newHeader.chunkCount].type = type;
        newHeader.chunks[newHeader.chunkCount].length = size;
        chunkData[newHeader.chunkCount++] = newLump;
    }
    h = FS_OpenFileOverwrite(comBspGlob.name);
    if (h)
    {
        FS_Write((char*)&newHeader, 8 * newHeader.chunkCount + 12, h);
        for (chunkIter = 0; chunkIter < newHeader.chunkCount; ++chunkIter)
        {
            FS_Write((char*)chunkData[chunkIter], newHeader.chunks[chunkIter].length, h);
            zeroCount = -newHeader.chunks[chunkIter].length & 3;
            if (zeroCount)
                FS_Write((char*)&zero, zeroCount, h);
        }
        FS_FCloseFile(h);
        if (behavior)
        {
            if (behavior != COM_SAVE_LUMP_AND_REOPEN)
                MyAssertHandler(
                    ".\\qcommon\\com_bsp_load_obj.cpp",
                    465,
                    1,
                    "%s\n\t(behavior) = %i",
                    "(behavior == COM_SAVE_LUMP_AND_REOPEN)",
                    behavior);
            v6 = &comBspGlob;
            v5 = savedName;
            do
            {
                v4 = v6->name[0];
                *v5 = v6->name[0];
                v6 = (v6 + 1);
                ++v5;
            } while (v4);
            Com_UnloadBsp();
            Com_LoadBsp(savedName);
        }
        else
        {
            Com_UnloadBsp();
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Failed to open file %s for writing", comBspGlob.name);
    }
}