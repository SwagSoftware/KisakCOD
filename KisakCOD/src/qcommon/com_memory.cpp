#include "com_memory.h"

#include <string.h>
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include "qcommon.h"
#include "threads.h"

unsigned __int8* __cdecl Hunk_AllocXAnimPrecache(unsigned int size)
{
    return Hunk_AllocAlign(size, 4, "XAnimPrecache", 11);
}

unsigned __int8* __cdecl Hunk_AllocPhysPresetPrecache(unsigned int size)
{
    return Hunk_Alloc(size, "PhysPresetPrecache", 0);
}

unsigned __int8* __cdecl Hunk_AllocXAnimClient(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXAnimClient", 11);
}

void __cdecl DB_EnumXAssets_FastFile(
    XAssetType type,
    void(__cdecl* func)(XAssetHeader, void*),
    void* inData,
    bool includeOverride)
{
    unsigned int hash; // [esp+4h] [ebp-14h]
    unsigned int assetEntryIndex; // [esp+8h] [ebp-10h]
    XAssetEntryPoolEntry* assetEntry; // [esp+10h] [ebp-8h]
    unsigned int overrideAssetEntryIndex; // [esp+14h] [ebp-4h]

    InterlockedIncrement(&db_hashCritSect.readCount);
    while (db_hashCritSect.writeCount)
        NET_Sleep(0);
    for (hash = 0; hash < 0x8000; ++hash)
    {
        for (assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash)
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            if (assetEntry->entry.asset.type == type)
            {
                ((void(__cdecl*)(unsigned int, unsigned int))func)((XAssetHeader)assetEntry->entry.asset.header.xmodelPieces, inData);
                if (includeOverride)
                {
                    for (overrideAssetEntryIndex = assetEntry->entry.nextOverride;
                        overrideAssetEntryIndex;
                        overrideAssetEntryIndex = g_assetEntryPool[overrideAssetEntryIndex].entry.nextOverride)
                    {
                        ((void(__cdecl*)(unsigned int, unsigned int))func)(
                            (XAssetHeader)g_assetEntryPool[overrideAssetEntryIndex].entry.asset.header.xmodelPieces,
                            inData);
                    }
                }
            }
        }
    }
    if (db_hashCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&db_hashCritSect.readCount);
}

int __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType type, XAssetHeader* assets, int maxCount)
{
    unsigned int hash; // [esp+4h] [ebp-10h]
    unsigned int assetEntryIndex; // [esp+8h] [ebp-Ch]
    int assetCount; // [esp+Ch] [ebp-8h]
    XAssetEntryPoolEntry* assetEntry; // [esp+10h] [ebp-4h]

    assetCount = 0;
    InterlockedIncrement(&db_hashCritSect.readCount);
    while (db_hashCritSect.writeCount)
        _NET_Sleep(0);
    for (hash = 0; hash < 0x8000; ++hash)
    {
        for (assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash)
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            if (assetEntry->entry.asset.type == type)
            {
                if (assets)
                {
                    if (assetCount >= maxCount)
                        MyAssertHandler(".\\database\\db_registry.cpp", 2877, 0, "%s", "assetCount < maxCount");
                    assets[assetCount] = assetEntry->entry.asset.header;
                }
                ++assetCount;
            }
        }
    }
    if (db_hashCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&db_hashCritSect.readCount);
    return assetCount;
}

unsigned __int8* __cdecl Hunk_AllocPhysPresetPrecache(int size)
{
    if (size <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 237, 0, "%s", "size > 0");
    return Hunk_Alloc(size, "Hunk_AllocPhysPresetPrecache", 21);
}

unsigned __int8* __cdecl Hunk_AllocXAnimServer(unsigned int size)
{
    return Hunk_AllocLow(size, "Hunk_AllocXAnimServer", 11);
}

//void __cdecl TRACK_com_memory()
//{
//    track_static_alloc_internal(com_fileDataHashTable, 4096, "com_fileDataHashTable", 10);
//    track_static_alloc_internal(g_largeLocalBuf, 0x80000, "g_largeLocalBuf", 10);
//}

void* __cdecl Z_VirtualReserve(int size)
{
    void* buf; // [esp+0h] [ebp-4h]

    if (size <= 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 150, 0, "%s\n\t(size) = %i", "(size > 0)", size);
    buf = VirtualAlloc(0, size, 0x2000u, 4u);
    if (!buf)
        MyAssertHandler(".\\universal\\com_memory.cpp", 208, 0, "%s", "buf");
    return buf;
}

void __cdecl Z_VirtualDecommitInternal(void* ptr, int size)
{
    if (size < 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 325, 0, "%s\n\t(size) = %i", "(size >= 0)", size);
    VirtualFree(ptr, size, 0x4000u);
}

void __cdecl Z_VirtualFreeInternal(void* ptr)
{
    VirtualFree(ptr, 0, 0x8000u);
}

void* __cdecl Z_TryVirtualAllocInternal(int size)
{
    void* ptr; // [esp+0h] [ebp-4h]

    ptr = Z_VirtualReserve(size);
    if (Z_TryVirtualCommitInternal(ptr, size))
        return ptr;
    Z_VirtualFree(ptr);
    return 0;
}

bool __cdecl Z_TryVirtualCommitInternal(void* ptr, int size)
{
    if (size < 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 294, 0, "%s\n\t(size) = %i", "(size >= 0)", size);
    return VirtualAlloc(ptr, size, 0x1000u, 4u) != 0;
}

void __cdecl Z_VirtualCommitInternal(void* ptr, int size)
{
    if (!Z_TryVirtualCommitInternal(ptr, size))
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 426);
}

void __cdecl Z_VirtualFree(void* ptr)
{
    Z_VirtualFreeInternal(ptr);
}

void __cdecl Z_VirtualDecommit(void* ptr, int size)
{
    Z_VirtualDecommitInternal(ptr, size);
}

char* __cdecl Z_TryVirtualAlloc(int size, const char* name, int type)
{
    char* buf; // [esp+0h] [ebp-4h]

    buf = (char*)Z_TryVirtualAllocInternal(size);
    if (buf)
        track_z_commit((size + 4095) & 0xFFFFF000, type);
    return buf;
}

char* __cdecl Z_VirtualAlloc(int size, const char* name, int type)
{
    char* buf; // [esp+0h] [ebp-4h]

    buf = Z_TryVirtualAlloc(size, name, type);
    if (!buf)
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 716);
    return buf;
}

void __cdecl Z_VirtualCommit(void* ptr, int size)
{
    Z_VirtualCommitInternal(ptr, size);
}

char* __cdecl CopyString(char* in)
{
    HashEntry_unnamed_type_u out; // [esp+0h] [ebp-4h]

    if (!in)
        MyAssertHandler(".\\universal\\com_memory.cpp", 829, 0, "%s", "in");
    out.prev = SL_GetString_(in, 0, 21).prev;
    return SL_ConvertToString(out.prev);
}

void __cdecl ReplaceString(const char** str, char* in)
{
    char* newStr; // [esp+0h] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\universal\\com_memory.cpp", 845, 0, "%s", "str");
    if (!in)
        MyAssertHandler(".\\universal\\com_memory.cpp", 846, 0, "%s", "in");
    newStr = CopyString(in);
    if (*str)
        FreeString(*str);
    *str = newStr;
}

void __cdecl FreeString(const char* str)
{
    unsigned int out; // [esp+0h] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\universal\\com_memory.cpp", 864, 0, "%s", "str");
    out = SL_FindString(str);
    if (!out)
        MyAssertHandler(".\\universal\\com_memory.cpp", 866, 0, "%s", "out");
    SL_RemoveRefToString(out);
}

void Com_InitHunkMemory()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1258, 0, "%s", "Sys_IsMainThread()");
    if (s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1259, 0, "%s", "!s_hunkData");
    if (FS_LoadStack())
        Com_Error(ERR_FATAL, &byte_8C0604);
    if (!useFastFile->current.enabled)
        s_hunkTotal = (int)&svs.archivedSnapshotBuffer[22495840];
    if (useFastFile->current.enabled)
        s_hunkTotal = (int)&unk_A00000;
    R_ReflectionProbeRegisterDvars();
    if (r_reflectionProbeGenerate->current.enabled)
        s_hunkTotal = 0x20000000;
    s_hunkData = (unsigned __int8*)Z_VirtualReserve(s_hunkTotal);
    if (!s_hunkData)
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 1318);
    s_origHunkData = s_hunkData;
    track_set_hunk_size(s_hunkTotal);
    Hunk_Clear();
    _Cmd_AddCommandInternal("meminfo", Com_Meminfo_f, &Com_Meminfo_f_VAR);
    _Cmd_AddCommandInternal("tempmeminfo", Com_TempMeminfo_f, &Com_TempMeminfo_f_VAR);
}

void __cdecl Com_Meminfo_f()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1140, 0, "%s", "Sys_IsMainThread()");
    track_PrintInfo();
    if (useFastFile->current.enabled)
        PMem_DumpMemStats();
    Com_Printf(0, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void* __cdecl Hunk_FindDataForFile(int type, const char* name)
{
    int hash; // [esp+0h] [ebp-4h]

    hash = FS_HashFileName(name, 1024);
    return Hunk_FindDataForFileInternal(type, name, hash);
}

void* __cdecl Hunk_FindDataForFileInternal(int type, const char* name, int hash)
{
    fileData_s* searchFileData; // [esp+0h] [ebp-4h]

    for (searchFileData = com_fileDataHashTable[hash]; searchFileData; searchFileData = searchFileData->next)
    {
        if (searchFileData->type == type && !I_stricmp(searchFileData->name, name))
            return searchFileData->data;
    }
    return 0;
}

bool __cdecl Hunk_DataOnHunk(unsigned __int8* data)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1457, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1458, 0, "%s", "s_hunkData");
    return data >= s_hunkData && data < &s_hunkData[s_hunkTotal];
}

char* __cdecl Hunk_SetDataForFile(int type, const char* name, void* data, void* (__cdecl* alloc)(int))
{
    char v5; // [esp+3h] [ebp-25h]
    char* v6; // [esp+8h] [ebp-20h]
    const char* v7; // [esp+Ch] [ebp-1Ch]
    int hash; // [esp+20h] [ebp-8h]
    fileData_s* fileData; // [esp+24h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1480, 0, "%s", "Sys_IsMainThread()");
    hash = FS_HashFileName(name, 1024);
    if (Hunk_FindDataForFileInternal(type, name, hash))
        MyAssertHandler(".\\universal\\com_memory.cpp", 1483, 0, "%s", "!Hunk_FindDataForFileInternal( type, name, hash )");
    fileData = (fileData_s*)alloc(strlen(name) + 10);
    if (!Hunk_DataOnHunk((unsigned __int8*)fileData))
        MyAssertHandler(".\\universal\\com_memory.cpp", 1488, 0, "%s", "Hunk_DataOnHunk( fileData )");
    fileData->data = data;
    fileData->type = type;
    if (type != fileData->type)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1493, 0, "%s", "type == fileData->type");
    v7 = name;
    v6 = fileData->name;
    do
    {
        v5 = *v7;
        *v6++ = *v7++;
    } while (v5);
    fileData->next = com_fileDataHashTable[hash];
    com_fileDataHashTable[hash] = fileData;
    return fileData->name;
}

void __cdecl Hunk_AddData(int type, void* data, void* (__cdecl* alloc)(int))
{
    fileData_s* fileData; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1511, 0, "%s", "Sys_IsMainThread()");
    fileData = (fileData_s*)alloc(9);
    if (!Hunk_DataOnHunk((unsigned __int8*)fileData))
        MyAssertHandler(".\\universal\\com_memory.cpp", 1516, 0, "%s", "Hunk_DataOnHunk( fileData )");
    fileData->data = data;
    fileData->type = type;
    if (type != fileData->type)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1521, 0, "%s", "type == fileData->type");
    fileData->next = com_hunkData;
    com_hunkData = fileData;
}

int __cdecl DB_GetAllXAssetOfType(XAssetType type, XAssetHeader* assets, int maxCount)
{
    if (useFastFile->current.enabled)
        return DB_GetAllXAssetOfType_FastFile(type, assets, maxCount);
    else
        return DB_GetAllXAssetOfType_LoadObj(type, assets, maxCount);
}

int __cdecl DB_GetAllXAssetOfType_LoadObj(XAssetType type, XAssetHeader* assets, int maxCount)
{
    AssetList assetList; // [esp+0h] [ebp-Ch] BYREF

    assetList.assets = assets;
    assetList.assetCount = 0;
    assetList.maxCount = maxCount;
    DB_EnumXAssets(type, (void(__cdecl*)(XAssetHeader, void*))Hunk_AddAsset, &assetList, 0);
    return assetList.assetCount;
}

void __cdecl DB_EnumXAssets(
    XAssetType type,
    void(__cdecl* func)(XAssetHeader, void*),
    void* inData,
    bool includeOverride)
{
    if (useFastFile->current.enabled)
        DB_EnumXAssets_FastFile(type, func, inData, includeOverride);
    else
        ((void(__cdecl*)(XAssetType, void(__cdecl*)(XAssetHeader, void*), void*, bool))DB_EnumXAssets_LoadObj)(
            type,
            func,
            inData,
            includeOverride);
}

void __cdecl DB_EnumXAssets_LoadObj(XAssetType type, void(__cdecl* func)(void*, void*), void* inData)
{
    unsigned int hash; // [esp+4h] [ebp-Ch]

    switch (type)
    {
    case ASSET_TYPE_XMODEL:
        for (hash = 0; hash < 0x400; ++hash)
            DB_EnumXAssetsFor(com_fileDataHashTable[hash], 5, func, inData);
        break;
    case ASSET_TYPE_MATERIAL:
        R_EnumMaterials((void(__cdecl*)(Material*, void*))func, inData);
        break;
    case ASSET_TYPE_TECHNIQUE_SET:
        R_EnumTechniqueSets((void(__cdecl*)(MaterialTechniqueSet*, void*))func, inData);
        break;
    case ASSET_TYPE_IMAGE:
        R_EnumImages((void(__cdecl*)(GfxImage*, void*))func, inData);
        break;
    default:
        return;
    }
}

void __cdecl DB_EnumXAssetsFor(
    fileData_s* fileData,
    int fileDataType,
    void(__cdecl* func)(void*, void*),
    void* inData)
{
    while (fileData)
    {
        if (fileData->type == fileDataType && fileData->type == 5)
            func(fileData->data, inData);
        fileData = fileData->next;
    }
}

void Hunk_ClearData()
{
    unsigned __int8* low; // [esp+0h] [ebp-10h]
    unsigned int hash; // [esp+4h] [ebp-Ch]
    unsigned __int8* high; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1619, 0, "%s", "Sys_IsMainThread()");
    low = &s_hunkData[hunk_low.permanent];
    high = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    for (hash = 0; hash < 0x400; ++hash)
        Hunk_ClearDataFor((fileData_s**)(4 * hash + 230744488), low, high);
    Hunk_ClearDataFor(&com_hunkData, low, high);
}

void __cdecl Hunk_ClearDataFor(fileData_s** pFileData, unsigned __int8* low, unsigned __int8* high)
{
    unsigned __int8 type; // [esp+0h] [ebp-Ch]
    XAnim_s* data; // [esp+4h] [ebp-8h]
    fileData_s* fileData; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1570, 0, "%s", "Sys_IsMainThread()");
    while (*pFileData)
    {
        fileData = *pFileData;
        if (*pFileData >= (fileData_s*)low && fileData < (fileData_s*)high)
        {
            *pFileData = fileData->next;
            data = (XAnim_s*)fileData->data;
            type = fileData->type;
            switch (type)
            {
            case 2u:
                XAnimFreeList(data);
                break;
            case 4u:
                XModelPartsFree((XModelPartsLoad*)data);
                break;
            case 6u:
                XAnimFree((XAnimParts*)data);
                break;
            }
        }
        else
        {
            pFileData = &fileData->next;
        }
    }
}

void __cdecl Hunk_ClearToMarkLow(int mark)
{
    unsigned __int8* endBuf; // [esp+0h] [ebp-Ch]
    unsigned __int8* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1849, 0, "%s", "Sys_IsMainThread()");
    Hunk_CheckTempMemoryClear();
    endBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = mark;
    hunk_low.permanent = mark;
    Hunk_ClearData();
    beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    if (endBuf != beginBuf)
        Z_VirtualDecommit(beginBuf, endBuf - beginBuf);
    track_hunk_ClearToMarkLow(mark);
}

void Hunk_Clear()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1882, 0, "%s", "Sys_IsMainThread()");
    hunk_low.permanent = 0;
    hunk_low.temp = 0;
    hunk_high.permanent = 0;
    hunk_high.temp = 0;
    Hunk_ClearData();
    Z_VirtualDecommit(s_hunkData, s_hunkTotal);
    track_hunk_ClearToMarkLow(0);
    track_hunk_ClearToMarkHigh(0);
    track_hunk_ClearToStart();
}

int __cdecl Hunk_Used()
{
    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1912, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    return hunk_high.permanent + hunk_low.permanent;
}

unsigned __int8* __cdecl Hunk_Alloc(unsigned int size, const char* name, int type)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1928, 0, "%s", "Sys_IsMainThread()");
    return Hunk_AllocAlign(size, 32, name, type);
}

unsigned __int8* __cdecl Hunk_AllocAlign(unsigned int size, int alignment, const char* name, int type)
{
    int old_permanent; // [esp+0h] [ebp-14h]
    unsigned __int8* buf; // [esp+4h] [ebp-10h]
    unsigned __int8* endBuf; // [esp+8h] [ebp-Ch]
    int alignmenta; // [esp+20h] [ebp+Ch]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1975, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1976, 0, "%s", "s_hunkData");
    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1983, 0, "%s", "!(alignment & (alignment - 1))");
    if (alignment > 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1984, 0, "%s", "alignment <= HUNK_MAX_ALIGNEMT");
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryHighClear();
    old_permanent = hunk_high.permanent;
    endBuf = (unsigned __int8*)((unsigned int)&s_hunkData[s_hunkTotal - hunk_high.permanent] & 0xFFFFF000);
    hunk_high.permanent += size;
    hunk_high.permanent = ~alignmenta & (alignmenta + hunk_high.permanent);
    hunk_high.temp = hunk_high.permanent;
    if (hunk_high.permanent + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, &byte_8C07A8, size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    if ((alignmenta & (unsigned int)buf) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2011, 0, "%s", "!(((psize_int)buf) & alignment)");
    if (endBuf != (unsigned __int8*)((unsigned int)buf & 0xFFFFF000))
        Z_VirtualCommit((void*)((unsigned int)buf & 0xFFFFF000), (int)&endBuf[-((unsigned int)buf & 0xFFFFF000)]);
    track_hunk_alloc(hunk_high.permanent - old_permanent, hunk_high.temp, name, type);
    memset(buf, 0, size);
    return buf;
}

unsigned int __cdecl Hunk_AllocateTempMemoryHigh(int size, const char* name)
{
    unsigned int buf; // [esp+0h] [ebp-10h]
    unsigned __int8* endBuf; // [esp+4h] [ebp-Ch]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2050, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2051, 0, "%s", "s_hunkData");
    endBuf = (unsigned __int8*)((unsigned int)&s_hunkData[s_hunkTotal - hunk_high.temp] & 0xFFFFF000);
    hunk_high.temp += size;
    hunk_high.temp = (hunk_high.temp + 15) & 0xFFFFFFF0;
    if (hunk_high.temp + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, &byte_8C0850, size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = (unsigned int)&s_hunkData[s_hunkTotal - hunk_high.temp];
    if ((((_BYTE)s_hunkTotal + (_BYTE)s_hunkData - LOBYTE(hunk_high.temp)) & 0xF) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2074, 0, "%s", "!(((psize_int)buf) & 15)");
    if (endBuf != (unsigned __int8*)(buf & 0xFFFFF000))
        Z_VirtualCommit((void*)(buf & 0xFFFFF000), (int)&endBuf[-(buf & 0xFFFFF000)]);
    track_temp_high_alloc(size, hunk_high.temp + hunk_low.temp, hunk_high.permanent, name);
    return buf;
}

void Hunk_ClearTempMemoryHigh()
{
    unsigned int commitSize; // [esp+4h] [ebp-8h]
    unsigned __int8* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2106, 0, "%s", "Sys_IsMainThread()");
    beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[s_hunkTotal - hunk_high.temp] & 0xFFFFF000);
    hunk_high.temp = hunk_high.permanent;
    commitSize = ((unsigned int)&s_hunkData[s_hunkTotal - hunk_high.permanent] & 0xFFFFF000) - (unsigned int)beginBuf;
    if (commitSize)
        Z_VirtualDecommit(beginBuf, commitSize);
    track_temp_high_clear(hunk_high.permanent);
}

unsigned __int8* __cdecl Hunk_AllocLow(unsigned int size, const char* name, int type)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2138, 0, "%s", "Sys_IsMainThread()");
    return Hunk_AllocLowAlign(size, 32, name, type);
}

unsigned __int8* __cdecl Hunk_AllocLowAlign(unsigned int size, int alignment, const char* name, int type)
{
    int old_permanent; // [esp+0h] [ebp-14h]
    unsigned __int8* buf; // [esp+4h] [ebp-10h]
    unsigned int commitSize; // [esp+Ch] [ebp-8h]
    unsigned __int8* beginBuf; // [esp+10h] [ebp-4h]
    int alignmenta; // [esp+20h] [ebp+Ch]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2185, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2186, 0, "%s", "s_hunkData");
    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2193, 0, "%s", "!(alignment & (alignment - 1))");
    if (alignment > 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2194, 0, "%s", "alignment <= HUNK_MAX_ALIGNEMT");
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryClear();
    old_permanent = hunk_low.permanent;
    beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000);
    hunk_low.permanent = ~alignmenta & (alignmenta + hunk_low.permanent);
    buf = &s_hunkData[hunk_low.permanent];
    if ((alignmenta & (unsigned int)&s_hunkData[hunk_low.permanent]) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2210, 0, "%s", "!(((psize_int)buf) & alignment)");
    hunk_low.permanent += size;
    hunk_low.temp = hunk_low.permanent;
    if (hunk_high.temp + hunk_low.permanent > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, &byte_8C08A8, size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    commitSize = ((unsigned int)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000) - (unsigned int)beginBuf;
    if (commitSize)
        Z_VirtualCommit(beginBuf, commitSize);
    track_hunk_allocLow(hunk_low.permanent - old_permanent, hunk_low.permanent, name, type);
    memset(buf, 0, size);
    return buf;
}

unsigned int* __cdecl Hunk_AllocateTempMemory(int size, const char* name)
{
    hunkHeader_t* hdr; // [esp+0h] [ebp-18h]
    unsigned __int8* buf; // [esp+4h] [ebp-14h]
    void* bufa; // [esp+4h] [ebp-14h]
    int prev_temp; // [esp+8h] [ebp-10h]
    unsigned int commitSize; // [esp+10h] [ebp-8h]
    unsigned __int8* beginBuf; // [esp+14h] [ebp-4h]
    int sizea; // [esp+20h] [ebp+8h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2263, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        return Z_Malloc(size, name, 10);
    sizea = size + 16;
    prev_temp = hunk_low.temp;
    beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = (hunk_low.temp + 15) & 0xFFFFFFF0;
    buf = &s_hunkData[hunk_low.temp];
    hunk_low.temp += sizea;
    if (hunk_high.temp + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(
            ERR_DROP,
            &byte_8C08F8,
            sizea,
            s_hunkTotal / 0x100000,
            hunk_low.temp / 0x100000,
            hunk_high.temp / 0x100000,
            hunk_high.temp + hunk_low.temp - s_hunkTotal);
    }
    hdr = (hunkHeader_t*)buf;
    bufa = buf + 16;
    if (((unsigned __int8)bufa & 0xF) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2303, 0, "%s", "!(((psize_int)buf) & 15)");
    commitSize = ((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000) - (unsigned int)beginBuf;
    if (commitSize)
        Z_VirtualCommit(beginBuf, commitSize);
    hdr->magic = -1991018350;
    hdr->size = hunk_low.temp - prev_temp;
    track_temp_alloc(hdr->size, hunk_high.temp + hunk_low.temp, hunk_low.permanent, name);
    hdr->name = name;
    return (unsigned int*)bufa;
}

void __cdecl Hunk_FreeTempMemory(char* buf)
{
    hunkHeader_t* hdr; // [esp+0h] [ebp-10h]
    unsigned __int8* endBuf; // [esp+4h] [ebp-Ch]
    unsigned __int8* beginBuf; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2339, 0, "%s", "Sys_IsMainThread()");
    if (s_hunkData)
    {
        if (!buf)
            MyAssertHandler(".\\universal\\com_memory.cpp", 2353, 0, "%s", "buf");
        hdr = (hunkHeader_t*)(buf - 16);
        if (*((unsigned int*)buf - 4) != -1991018350)
            Com_Error(ERR_FATAL, &byte_8C09B4);
        hdr->magic = -1991018349;
        if (hdr != (hunkHeader_t*)&s_hunkData[(hunk_low.temp - hdr->size + 15) & 0xFFFFFFF0])
            MyAssertHandler(
                ".\\universal\\com_memory.cpp",
                2362,
                0,
                "%s",
                "hdr == (void *)( s_hunkData + ((hunk_low.temp - hdr->size + 15) & ~15) )");
        endBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
        hunk_low.temp -= hdr->size;
        track_temp_free(hdr->size, hunk_low.permanent, hdr->name);
        beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
        if (endBuf != beginBuf)
            Z_VirtualDecommit(beginBuf, endBuf - beginBuf);
    }
    else
    {
        Z_Free(buf, 10);
    }
}

void Hunk_ClearTempMemory()
{
    unsigned __int8* endBuf; // [esp+0h] [ebp-Ch]
    unsigned __int8* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2398, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2399, 0, "%s", "s_hunkData");
    endBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = hunk_low.permanent;
    beginBuf = (unsigned __int8*)((unsigned int)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000);
    if (endBuf != beginBuf)
        Z_VirtualDecommit(beginBuf, endBuf - beginBuf);
    track_temp_clear(hunk_low.permanent);
}

void Hunk_CheckTempMemoryClear()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2430, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2431, 0, "%s", "s_hunkData");
    if (hunk_low.temp != hunk_low.permanent)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2432, 0, "%s", "hunk_low.temp == hunk_low.permanent");
}

void Hunk_CheckTempMemoryHighClear()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2443, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2444, 0, "%s", "s_hunkData");
    if (hunk_high.temp != hunk_high.permanent)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2445, 0, "%s", "hunk_high.temp == hunk_high.permanent");
}

int __cdecl Hunk_HideTempMemory()
{
    int mark; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2460, 0, "%s", "Sys_IsMainThread()");
    mark = hunk_low.permanent;
    hunk_low.permanent = hunk_low.temp;
    return mark;
}

void __cdecl Hunk_ShowTempMemory(int mark)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2476, 0, "%s", "Sys_IsMainThread()");
    Hunk_CheckTempMemoryClear();
    hunk_low.permanent = mark;
}

int __cdecl LargeLocalBegin(int size)
{
    int startPos; // [esp+0h] [ebp-4h]
    unsigned int sizea; // [esp+Ch] [ebp+8h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2534, 0, "%s", "Sys_IsMainThread()");
    sizea = LargeLocalRoundSize(size);
    startPos = g_largeLocalPos;
    g_largeLocalPos += sizea;
    if (g_largeLocalPos > 0x80000)
        MyAssertHandler(
            ".\\universal\\com_memory.cpp",
            2540,
            0,
            "%s",
            "g_largeLocalPos <= static_cast< int >( sizeof( g_largeLocalBuf ) )");
    return startPos;
}

unsigned int __cdecl LargeLocalRoundSize(int size)
{
    return (size + 3) & 0xFFFFFFFC;
}

void __cdecl LargeLocalEnd(int startPos)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2556, 0, "%s", "Sys_IsMainThread()");
    g_largeLocalPos = startPos;
}

unsigned __int8* __cdecl LargeLocalGetBuf(int startPos)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2620, 0, "%s", "Sys_IsMainThread()");
    return &g_largeLocalBuf[startPos];
}

void __thiscall LargeLocal::LargeLocal(LargeLocal* this, int sizeParam)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2648, 0, "%s", "Sys_IsMainThread()");
    this->startPos = LargeLocalBegin(sizeParam);
    this->size = sizeParam;
}

void __thiscall LargeLocal::~LargeLocal(LargeLocal* this)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2674, 0, "%s", "Sys_IsMainThread()");
    LargeLocalEnd(this->startPos);
}

unsigned __int8* __thiscall LargeLocal::GetBuf(LargeLocal* this)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2698, 0, "%s", "Sys_IsMainThread()");
    return LargeLocalGetBuf(this->startPos);
}

void __cdecl LargeLocalReset()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2715, 0, "%s", "Sys_IsMainThread()");
    g_largeLocalPos = 0;
}

void __cdecl Hunk_InitDebugMemory()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2743, 0, "%s", "Sys_IsMainThread()");
    if (g_debugUser)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2744, 0, "%s", "!g_debugUser");
    g_debugUser = Hunk_UserCreate((int)&clients[0].parseClients[238].attachTagIndex[5], "Hunk_InitDebugMemory", 0, 0, 0);
}

void __cdecl Hunk_ShutdownDebugMemory()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2758, 0, "%s", "Sys_IsMainThread()");
    if (!g_debugUser)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2759, 0, "%s", "g_debugUser");
    Hunk_UserDestroy(g_debugUser);
    g_debugUser = 0;
}

void __cdecl Hunk_ResetDebugMem()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2774, 0, "%s", "Sys_IsMainThread()");
    if (!g_debugUser)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2775, 0, "%s", "g_debugUser");
    Hunk_UserReset(g_debugUser);
}

int __cdecl Hunk_AllocDebugMem(unsigned int size)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2789, 0, "%s", "Sys_IsMainThread()");
    if (!g_debugUser)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2790, 0, "%s", "g_debugUser");
    return Hunk_UserAlloc(g_debugUser, size, 4);
}

void __cdecl Hunk_FreeDebugMem()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2804, 0, "%s", "Sys_IsMainThread()");
    if (!g_debugUser)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2805, 0, "%s", "g_debugUser");
}

HunkUser* __cdecl Hunk_UserCreate(int maxSize, const char* name, bool fixed, bool tempMem, int type)
{
    HunkUser* user; // [esp+0h] [ebp-4h]

    if (maxSize % 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2834, 0, "%s\n\t(maxSize) = %i", "(!(maxSize % (4*1024)))", maxSize);
    user = (HunkUser*)Z_VirtualReserve(maxSize);
    Z_VirtualCommit(user, 32);
    user->end = (int)user + maxSize;
    user->pos = (int)user->buf;
    if ((user->pos & 0x1F) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2848, 0, "%s\n\t(user->pos) = %i", "(!(user->pos & 31))", user->pos);
    user->maxSize = maxSize;
    user->current = user;
    user->fixed = fixed;
    user->name = name;
    user->tempMem = tempMem;
    user->type = type;
    if (user->next)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2855, 0, "%s", "!user->next");
    return user;
}

int __cdecl Hunk_UserAlloc(HunkUser* user, unsigned int size, int alignment)
{
    const char* v3; // eax
    int pos; // [esp+4h] [ebp-10h]
    void* posa; // [esp+4h] [ebp-10h]
    int result; // [esp+8h] [ebp-Ch]
    HunkUser* current; // [esp+Ch] [ebp-8h]
    HunkUser* newCurrent; // [esp+10h] [ebp-4h]
    int alignmenta; // [esp+24h] [ebp+10h]

    if (!user)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2873, 0, "%s", "user");
    if (size > user->maxSize - 32)
    {
        v3 = va("size: %d, maxSize: %d", size, user->maxSize - 32);
        MyAssertHandler(
            ".\\universal\\com_memory.cpp",
            2874,
            0,
            "%s\n\t%s",
            "static_cast< uint >( size ) <= user->maxSize - offsetof( HunkUser, buf )",
            v3);
    }
    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2875, 0, "%s", "!(alignment & (alignment - 1))");
    if (alignment > 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2876, 0, "%s", "alignment <= HUNK_MAX_ALIGNEMT");
    alignmenta = alignment - 1;
    for (current = user->current; ; current = newCurrent)
    {
        pos = current->pos;
        result = ~alignmenta & (alignmenta + pos);
        if ((signed int)(size + result) <= current->end)
            break;
        if (user->fixed)
            Com_Error(ERR_FATAL, "Hunk_UserAlloc: out of memory");
        newCurrent = Hunk_UserCreate(user->maxSize, user->name, 0, user->tempMem, user->type);
        user->current = newCurrent;
        current->next = newCurrent;
    }
    current->pos = size + (~alignmenta & (alignmenta + pos));
    posa = (void*)((pos + 4095) & 0xFFFFF000);
    if (posa != (void*)((current->pos + 4095) & 0xFFFFF000))
    {
        if (current->pos - (int)posa <= 0)
            MyAssertHandler(".\\universal\\com_memory.cpp", 2904, 0, "%s", "current->pos - pos > 0");
        Z_VirtualCommit(posa, current->pos - (unsigned int)posa);
    }
    return result;
}

int __cdecl Hunk_UserAllocAlignStrict(HunkUser* user, unsigned int size)
{
    return Hunk_UserAlloc(user, size, 1);
}

void __cdecl Hunk_UserSetPos(HunkUser* user, unsigned __int8* pos)
{
    if (!user->fixed)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2991, 0, "%s", "user->fixed");
    if (pos < user->buf)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2992, 0, "%s", "pos >= user->buf");
    if ((int)pos > user->pos)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2993, 0, "%s", "(psize_int)pos <= user->pos");
    user->pos = (int)pos;
}

void __cdecl Hunk_UserReset(HunkUser* user)
{
    void* pos; // [esp+0h] [ebp-4h]

    if (user->next)
    {
        Hunk_UserDestroy(user->next);
        user->current = user;
        user->next = 0;
    }
    pos = (void*)(((unsigned int)&user[114].name + 3) & 0xFFFFF000);
    if (pos != (void*)((user->pos + 4095) & 0xFFFFF000))
    {
        if (user->pos - (int)pos <= 0)
            MyAssertHandler(".\\universal\\com_memory.cpp", 3019, 0, "%s", "user->pos - pos > 0");
        Z_VirtualDecommit(pos, user->pos - (unsigned int)pos);
    }
    user->pos = (int)user->buf;
    memset(user->buf, 0, 0xFE0u);
}

void __cdecl Hunk_UserDestroy(HunkUser* user)
{
    HunkUser* current; // [esp+0h] [ebp-8h]
    HunkUser* newCurrent; // [esp+4h] [ebp-4h]

    for (current = user->next; current; current = newCurrent)
    {
        newCurrent = current->next;
        Z_VirtualFree(current);
    }
    Z_VirtualFree(user);
}

char* __cdecl Hunk_CopyString(HunkUser* user, const char* in)
{
    char v3; // [esp+3h] [ebp-21h]
    char* v4; // [esp+8h] [ebp-1Ch]
    const char* v5; // [esp+Ch] [ebp-18h]
    char* out; // [esp+20h] [ebp-4h]

    out = (char*)Hunk_UserAlloc(user, strlen(in) + 1, 1);
    v5 = in;
    v4 = out;
    do
    {
        v3 = *v5;
        *v4++ = *v5++;
    } while (v3);
    return out;
}

unsigned __int8* __cdecl Hunk_AllocXModelPrecache(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecache", 21);
}

unsigned __int8* __cdecl Hunk_AllocXModelPrecacheColl(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecacheColl", 27);
}

