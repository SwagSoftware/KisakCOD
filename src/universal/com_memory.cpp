#include "com_memory.h"
#include "assertive.h"

#include "script/scr_stringlist.h"

#include <string.h>
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/threads.h>

#include <qcommon/mem_track.h>
#include <win32/win_local.h>
#include <win32/win_net.h>
#include <database/database.h>
#include "com_files.h"
#include <qcommon/cmd.h>
#include <gfx_d3d/r_dvars.h>
#include "physicalmemory.h"

#include <cstdint>

HunkUser *g_user;
fileData_s *com_hunkData;

static HunkUser* g_debugUser;
static int32_t g_largeLocalPos;
static unsigned char g_largeLocalBuf[0x80000];

struct hunkUsed_t // sizeof=0x8
{                                       // ...
	int32_t permanent;                      // ...
	int32_t temp;                           // ...
};
struct hunkHeader_t // sizeof=0x10
{
	uint32_t magic;
	int32_t size;
	const char* name;
	int32_t dummy;
};

static hunkUsed_t hunk_high;
static hunkUsed_t hunk_low;

unsigned char* s_hunkData;
uint8_t *s_origHunkData;
int32_t s_hunkTotal;

void __cdecl Hunk_AddAsset(XAssetHeader header, _DWORD *data)
{
    if (!data)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1731, 0, "%s", "data");
    if (*data >= data[1])
        MyAssertHandler(".\\universal\\com_memory.cpp", 1735, 0, "%s", "assetList->assetCount < assetList->maxCount");
    *(XAssetHeader *)(data[2] + 4 * (*data)++) = header;
}

void Com_TouchMemory()
{
    int32_t sum; // [esp+4h] [ebp-10h]
    DWORD start; // [esp+8h] [ebp-Ch]
    DWORD end; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]
    int32_t ia; // [esp+10h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1218, 0, "%s", "Sys_IsMainThread()");
    start = Sys_Milliseconds();
    sum = 0;
    for (i = 0; i < hunk_low.permanent >> 2; i += 64)
        sum += *(_DWORD *)&s_hunkData[4 * i];
    for (ia = (s_hunkTotal - hunk_high.permanent) >> 2; ia < hunk_high.permanent >> 2; ia += 64)
        sum += *(_DWORD *)&s_hunkData[4 * ia];
    end = Sys_Milliseconds();
    Com_Printf(16, "Com_TouchMemory: %i msec. Using sum: %d\n", end - start, sum);
}

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
const char* CopyString(const char* in)
{
	uint32_t out;

	iassert(in);

	out = SL_GetString_(in, 0, 21);
	return SL_ConvertToString(out);
}

void FreeString(const char* str)
{
	iassert(str);

	uint32_t out = SL_FindString(str);

	iassert(out);

	SL_RemoveRefToString(out);
}


uint8_t* __cdecl Hunk_AllocXAnimPrecache(uint32_t size)
{
    return Hunk_AllocAlign(size, 4, "XAnimPrecache", 11);
}

uint8_t* __cdecl Hunk_AllocPhysPresetPrecache(uint32_t size)
{
    return Hunk_Alloc(size, "PhysPresetPrecache", 0);
}

uint8_t* __cdecl Hunk_AllocXAnimClient(uint32_t size)
{
    return Hunk_Alloc(size, "Hunk_AllocXAnimClient", 11);
}


uint8_t* __cdecl Hunk_AllocXAnimServer(uint32_t size)
{
    return Hunk_AllocLow(size, "Hunk_AllocXAnimServer", 11);
}

//void __cdecl TRACK_com_memory()
//{
//    track_static_alloc_internal(com_fileDataHashTable, 4096, "com_fileDataHashTable", 10);
//    track_static_alloc_internal(g_largeLocalBuf, 0x80000, "g_largeLocalBuf", 10);
//}

void* __cdecl Z_VirtualReserve(int32_t size)
{
    void* buf; // [esp+0h] [ebp-4h]

    if (size <= 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 150, 0, "%s\n\t(size) = %i", "(size > 0)", size);
    buf = VirtualAlloc(0, size, 0x2000u, 4u);
    if (!buf)
        MyAssertHandler(".\\universal\\com_memory.cpp", 208, 0, "%s", "buf");
    return buf;
}

void __cdecl Z_VirtualDecommitInternal(void* ptr, int32_t size)
{
    if (size < 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 325, 0, "%s\n\t(size) = %i", "(size >= 0)", size);
    VirtualFree(ptr, size, 0x4000u);
}

void __cdecl Z_VirtualFreeInternal(void* ptr)
{
    VirtualFree(ptr, 0, 0x8000u);
}

void* __cdecl Z_TryVirtualAllocInternal(int32_t size)
{
    void* ptr; // [esp+0h] [ebp-4h]

    ptr = Z_VirtualReserve(size);
    if (Z_TryVirtualCommitInternal(ptr, size))
        return ptr;
    Z_VirtualFree(ptr);
    return 0;
}

bool __cdecl Z_TryVirtualCommitInternal(void* ptr, int32_t size)
{
    iassert(size >= 0);
    return VirtualAlloc(ptr, size, 0x1000u, 4u) != 0;
}

void __cdecl Z_VirtualCommitInternal(void* ptr, int32_t size)
{
    if (!Z_TryVirtualCommitInternal(ptr, size))
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 426);
}

void __cdecl Z_VirtualFree(void* ptr)
{
    Z_VirtualFreeInternal(ptr);
}

void __cdecl Z_VirtualDecommit(void* ptr, int32_t size)
{
    Z_VirtualDecommitInternal(ptr, size);
}

char* __cdecl Z_TryVirtualAlloc(int32_t size, const char* name, int32_t type)
{
    char* buf; // [esp+0h] [ebp-4h]

    buf = (char*)Z_TryVirtualAllocInternal(size);
    if (buf)
        track_z_commit((size + 4095) & 0xFFFFF000, type);
    return buf;
}

char* __cdecl Z_VirtualAlloc(int32_t size, const char* name, int32_t type)
{
    char* buf; // [esp+0h] [ebp-4h]

    buf = Z_TryVirtualAlloc(size, name, type);
    if (!buf)
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 716);
    return buf;
}

void __cdecl Z_VirtualCommit(void* ptr, int32_t size)
{
    Z_VirtualCommitInternal(ptr, size);
}

const char* __cdecl CopyString(char* in)
{
    iassert(in);

    return SL_ConvertToString(SL_GetString_(in, 0, 21));
}

void __cdecl ReplaceString(const char** str, const char* in)
{
    const char* newStr; // [esp+0h] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\universal\\com_memory.cpp", 845, 0, "%s", "str");
    if (!in)
        MyAssertHandler(".\\universal\\com_memory.cpp", 846, 0, "%s", "in");
    newStr = CopyString(in);
    if (*str)
        FreeString(*str);
    *str = newStr;
}

cmd_function_s Com_Meminfo_f_VAR;
cmd_function_s Com_TempMeminfo_f_VAR;

void __cdecl Com_TempMeminfo_f()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1194, 0, "%s", "Sys_IsMainThread()");
    //track_PrintTempInfo();
    Com_Printf(0, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void Com_InitHunkMemory()
{
    iassert(Sys_IsMainThread());
    iassert(!s_hunkData);

    if (FS_LoadStack())
        Com_Error(ERR_FATAL, "Hunk initialization failed. File system load stack not zero");

    if (!IsFastFileLoad())
    {
#ifdef KISAK_PURE
        s_hunkTotal = 0xA000000;
#else
        s_hunkTotal = 0x20000000; // LWSS: MOAR! !
#endif
    }
    if (IsFastFileLoad())
    {
        s_hunkTotal = 0xA00000;
    }
    R_ReflectionProbeRegisterDvars();
    if (r_reflectionProbeGenerate->current.enabled)
        s_hunkTotal = 0x20000000;
    s_hunkData = (unsigned char*)Z_VirtualReserve(s_hunkTotal);
    if (!s_hunkData)
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 1318);
    s_origHunkData = s_hunkData;
    track_set_hunk_size(s_hunkTotal);
    Hunk_Clear();
    Cmd_AddCommandInternal("meminfo", Com_Meminfo_f, &Com_Meminfo_f_VAR);
    Cmd_AddCommandInternal("tempmeminfo", Com_TempMeminfo_f, &Com_TempMeminfo_f_VAR);
}

void __cdecl Com_Meminfo_f()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1140, 0, "%s", "Sys_IsMainThread()");
    track_PrintInfo();
    if (IsFastFileLoad())
        PMem_DumpMemStats();
    Com_Printf(0, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void* __cdecl Hunk_FindDataForFile(int32_t type, const char* name)
{
    int32_t hash; // [esp+0h] [ebp-4h]

    hash = FS_HashFileName(name, 1024);
    return Hunk_FindDataForFileInternal(type, name, hash);
}

void* __cdecl Hunk_FindDataForFileInternal(int32_t type, const char* name, int32_t hash)
{
    fileData_s* searchFileData; // [esp+0h] [ebp-4h]

    for (searchFileData = com_fileDataHashTable[hash]; searchFileData; searchFileData = searchFileData->next)
    {
        if (searchFileData->type == type && !I_stricmp(searchFileData->name, name))
            return searchFileData->data;
    }
    return 0;
}

bool __cdecl Hunk_DataOnHunk(uint8_t* data)
{
#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1458, 0, "%s", "s_hunkData");
    return data >= s_hunkData && data < &s_hunkData[s_hunkTotal];
}

char* __cdecl Hunk_SetDataForFile(int32_t type, const char* name, void* data, void* (__cdecl* alloc)(int))
{
    char v5; // [esp+3h] [ebp-25h]
    char* v6; // [esp+8h] [ebp-20h]
    const char* v7; // [esp+Ch] [ebp-1Ch]
    int32_t hash; // [esp+20h] [ebp-8h]
    fileData_s* fileData; // [esp+24h] [ebp-4h]

#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    hash = FS_HashFileName(name, 1024);
    if (Hunk_FindDataForFileInternal(type, name, hash))
        MyAssertHandler(".\\universal\\com_memory.cpp", 1483, 0, "%s", "!Hunk_FindDataForFileInternal( type, name, hash )");
    fileData = (fileData_s*)alloc(strlen(name) + 10);
    if (!Hunk_DataOnHunk((uint8_t*)fileData))
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

void __cdecl Hunk_AddData(int32_t type, void* data, void* (__cdecl* alloc)(int))
{
    fileData_s* fileData; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1511, 0, "%s", "Sys_IsMainThread()");
    fileData = (fileData_s*)alloc(9);
    if (!Hunk_DataOnHunk((uint8_t*)fileData))
        MyAssertHandler(".\\universal\\com_memory.cpp", 1516, 0, "%s", "Hunk_DataOnHunk( fileData )");
    fileData->data = data;
    fileData->type = type;
    if (type != fileData->type)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1521, 0, "%s", "type == fileData->type");
    fileData->next = com_hunkData;
    com_hunkData = fileData;
}

void Hunk_ClearData()
{
    uint8_t* low; // [esp+0h] [ebp-10h]
    uint32_t hash; // [esp+4h] [ebp-Ch]
    uint8_t* high; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1619, 0, "%s", "Sys_IsMainThread()");
    low = &s_hunkData[hunk_low.permanent];
    high = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    for (hash = 0; hash < 0x400; ++hash)
        Hunk_ClearDataFor(&com_fileDataHashTable[hash], low, high);
    Hunk_ClearDataFor(&com_hunkData, low, high);
}

void __cdecl Hunk_ClearDataFor(fileData_s** pFileData, uint8_t* low, uint8_t* high)
{
    uint8_t type; // [esp+0h] [ebp-Ch]
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

void __cdecl Hunk_ClearToMarkLow(int32_t mark)
{
    uint8_t* endBuf; // [esp+0h] [ebp-Ch]
    uint8_t* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1849, 0, "%s", "Sys_IsMainThread()");
    Hunk_CheckTempMemoryClear();
    endBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = mark;
    hunk_low.permanent = mark;
    Hunk_ClearData();
    beginBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
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

int32_t __cdecl Hunk_Used()
{
    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1912, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    return hunk_high.permanent + hunk_low.permanent;
}

uint8_t* __cdecl Hunk_Alloc(uint32_t size, const char* name, int32_t type)
{
#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    return Hunk_AllocAlign(size, 32, name, type);
}

uint8_t* __cdecl Hunk_AllocAlign(uint32_t size, int32_t alignment, const char* name, int32_t type)
{
    int32_t old_permanent; // [esp+0h] [ebp-14h]
    uint8_t* buf; // [esp+4h] [ebp-10h]
    uint8_t* endBuf; // [esp+8h] [ebp-Ch]
    int32_t alignmenta; // [esp+20h] [ebp+Ch]

#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1976, 0, "%s", "s_hunkData");
    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1983, 0, "%s", "!(alignment & (alignment - 1))");
    if (alignment > 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1984, 0, "%s", "alignment <= HUNK_MAX_ALIGNEMT");
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryHighClear();
    old_permanent = hunk_high.permanent;
    endBuf = (uint8_t*)((uint32_t)&s_hunkData[s_hunkTotal - hunk_high.permanent] & 0xFFFFF000);
    hunk_high.permanent += size;
    hunk_high.permanent = ~alignmenta & (alignmenta + hunk_high.permanent);
    hunk_high.temp = hunk_high.permanent;
    if (hunk_high.permanent + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocAlign failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    if ((alignmenta & (uint32_t)buf) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2011, 0, "%s", "!(((psize_int)buf) & alignment)");
    if (endBuf != (uint8_t*)((uint32_t)buf & 0xFFFFF000))
        Z_VirtualCommit((void*)((uint32_t)buf & 0xFFFFF000), (int)&endBuf[-(int)((uint32_t)buf & 0xFFFFF000)]); // KISAKTODO: sus int32_t cast
    track_hunk_alloc(hunk_high.permanent - old_permanent, hunk_high.temp, name, type);
    memset(buf, 0, size);
    return buf;
}

uint32_t __cdecl Hunk_AllocateTempMemoryHigh(int32_t size, const char* name)
{
    uint32_t buf; // [esp+0h] [ebp-10h]
    uint8_t* endBuf; // [esp+4h] [ebp-Ch]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2050, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2051, 0, "%s", "s_hunkData");
    endBuf = (uint8_t*)((uint32_t)&s_hunkData[s_hunkTotal - hunk_high.temp] & 0xFFFFF000);
    hunk_high.temp += size;
    hunk_high.temp = (hunk_high.temp + 15) & 0xFFFFFFF0;
    if (hunk_high.temp + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocateTempMemoryHigh: failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = (uint32_t)&s_hunkData[s_hunkTotal - hunk_high.temp];
    if ((((_BYTE)s_hunkTotal + (_BYTE)s_hunkData - LOBYTE(hunk_high.temp)) & 0xF) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2074, 0, "%s", "!(((psize_int)buf) & 15)");
    if (endBuf != (uint8_t*)(buf & 0xFFFFF000))
        Z_VirtualCommit((void*)(buf & 0xFFFFF000), (int)&endBuf[-(int)(buf & 0xFFFFF000)]); // KISAKTODO: sus int32_t cast
    track_temp_high_alloc(size, hunk_high.temp + hunk_low.temp, hunk_high.permanent, name);
    return buf;
}

void Hunk_ClearTempMemoryHigh()
{
    uint32_t commitSize; // [esp+4h] [ebp-8h]
    uint8_t* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2106, 0, "%s", "Sys_IsMainThread()");
    beginBuf = (uint8_t*)((uint32_t)&s_hunkData[s_hunkTotal - hunk_high.temp] & 0xFFFFF000);
    hunk_high.temp = hunk_high.permanent;
    commitSize = ((uint32_t)&s_hunkData[s_hunkTotal - hunk_high.permanent] & 0xFFFFF000) - (uint32_t)beginBuf;
    if (commitSize)
        Z_VirtualDecommit(beginBuf, commitSize);
    track_temp_high_clear(hunk_high.permanent);
}

uint8_t* __cdecl Hunk_AllocLow(uint32_t size, const char* name, int32_t type)
{
#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    return Hunk_AllocLowAlign(size, 32, name, type);
}

uint8_t* __cdecl Hunk_AllocLowAlign(uint32_t size, int32_t alignment, const char* name, int32_t type)
{
    int32_t old_permanent; // [esp+0h] [ebp-14h]
    uint8_t* buf; // [esp+4h] [ebp-10h]
    uint32_t commitSize; // [esp+Ch] [ebp-8h]
    uint8_t* beginBuf; // [esp+10h] [ebp-4h]
    int32_t alignmenta; // [esp+20h] [ebp+Ch]

#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2186, 0, "%s", "s_hunkData");
    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2193, 0, "%s", "!(alignment & (alignment - 1))");
    if (alignment > 4096)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2194, 0, "%s", "alignment <= HUNK_MAX_ALIGNEMT");
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryClear();
    old_permanent = hunk_low.permanent;
    beginBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000);
    hunk_low.permanent = ~alignmenta & (alignmenta + hunk_low.permanent);
    buf = &s_hunkData[hunk_low.permanent];
    if ((alignmenta & (uint32_t)&s_hunkData[hunk_low.permanent]) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2210, 0, "%s", "!(((psize_int)buf) & alignment)");
    hunk_low.permanent += size;
    hunk_low.temp = hunk_low.permanent;
    if (hunk_high.temp + hunk_low.permanent > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocLowAlign failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    commitSize = ((uint32_t)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000) - (uint32_t)beginBuf;
    if (commitSize)
        Z_VirtualCommit(beginBuf, commitSize);
    track_hunk_allocLow(hunk_low.permanent - old_permanent, hunk_low.permanent, name, type);
    memset(buf, 0, size);
    return buf;
}

uint32_t* __cdecl Hunk_AllocateTempMemory(int32_t size, const char* name)
{
    hunkHeader_t* hdr; // [esp+0h] [ebp-18h]
    uint8_t* buf; // [esp+4h] [ebp-14h]
    void* bufa; // [esp+4h] [ebp-14h]
    int32_t prev_temp; // [esp+8h] [ebp-10h]
    uint32_t commitSize; // [esp+10h] [ebp-8h]
    uint8_t* beginBuf; // [esp+14h] [ebp-4h]
    int32_t sizea; // [esp+20h] [ebp+8h]


#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    if (!s_hunkData)
        return (uint32_t*)Z_Malloc(size, name, 10);
    sizea = size + 16;
    prev_temp = hunk_low.temp;
    beginBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = (hunk_low.temp + 15) & 0xFFFFFFF0;
    buf = &s_hunkData[hunk_low.temp];
    hunk_low.temp += sizea;
    if (hunk_high.temp + hunk_low.temp > s_hunkTotal)
    {
        track_PrintAllInfo();
        Com_Error(
            ERR_DROP,
            "Hunk_AllocateTempMemory: failed on %i bytes (total %i MB, low %i MB, high %i MB), needs %i more hunk bytes",
            sizea,
            s_hunkTotal / 0x100000,
            hunk_low.temp / 0x100000,
            hunk_high.temp / 0x100000,
            hunk_high.temp + hunk_low.temp - s_hunkTotal);
    }
    hdr = (hunkHeader_t*)buf;
    bufa = buf + 16;
    if (((uint8_t)bufa & 0xF) != 0)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2303, 0, "%s", "!(((psize_int)buf) & 15)");
    commitSize = ((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000) - (uint32_t)beginBuf;
    if (commitSize)
        Z_VirtualCommit(beginBuf, commitSize);
    hdr->magic = -1991018350;
    hdr->size = hunk_low.temp - prev_temp;
    track_temp_alloc(hdr->size, hunk_high.temp + hunk_low.temp, hunk_low.permanent, name);
    hdr->name = name;
    return (uint32_t*)bufa;
}

void __cdecl Hunk_FreeTempMemory(char* buf)
{
    hunkHeader_t* hdr; // [esp+0h] [ebp-10h]
    uint8_t* endBuf; // [esp+4h] [ebp-Ch]
    uint8_t* beginBuf; // [esp+Ch] [ebp-4h]

#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif

    if (s_hunkData)
    {
        if (!buf)
            MyAssertHandler(".\\universal\\com_memory.cpp", 2353, 0, "%s", "buf");
        hdr = (hunkHeader_t*)(buf - 16);
        if (*((uint32_t*)buf - 4) != 0x89537892)
            Com_Error(ERR_FATAL, "Hunk_FreeTempMemory: bad magic");
        hdr->magic = -1991018349;
        if (hdr != (hunkHeader_t*)&s_hunkData[(hunk_low.temp - hdr->size + 15) & 0xFFFFFFF0])
            MyAssertHandler(
                ".\\universal\\com_memory.cpp",
                2362,
                0,
                "%s",
                "hdr == (void *)( s_hunkData + ((hunk_low.temp - hdr->size + 15) & ~15) )");
        endBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
        hunk_low.temp -= hdr->size;
        track_temp_free(hdr->size, hunk_low.permanent, hdr->name);
        beginBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
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
    uint8_t* endBuf; // [esp+0h] [ebp-Ch]
    uint8_t* beginBuf; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2398, 0, "%s", "Sys_IsMainThread()");
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2399, 0, "%s", "s_hunkData");
    endBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.temp + 4095] & 0xFFFFF000);
    hunk_low.temp = hunk_low.permanent;
    beginBuf = (uint8_t*)((uint32_t)&s_hunkData[hunk_low.permanent + 4095] & 0xFFFFF000);
    if (endBuf != beginBuf)
        Z_VirtualDecommit(beginBuf, endBuf - beginBuf);
    //track_temp_clear(hunk_low.permanent);
}

void Hunk_CheckTempMemoryClear()
{
#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    if (!s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2431, 0, "%s", "s_hunkData");
    if (hunk_low.temp != hunk_low.permanent)
        MyAssertHandler(".\\universal\\com_memory.cpp", 2432, 0, "%s", "hunk_low.temp == hunk_low.permanent");
}

void Hunk_CheckTempMemoryHighClear()
{
#ifdef KISAK_MP
    iassert(Sys_IsMainThread());
#endif
    iassert(s_hunkData);
    iassert(hunk_high.temp == hunk_high.permanent);
}

int32_t __cdecl Hunk_HideTempMemory()
{
    int32_t mark; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2460, 0, "%s", "Sys_IsMainThread()");
    mark = hunk_low.permanent;
    hunk_low.permanent = hunk_low.temp;
    return mark;
}

void __cdecl Hunk_ShowTempMemory(int32_t mark)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2476, 0, "%s", "Sys_IsMainThread()");
    Hunk_CheckTempMemoryClear();
    hunk_low.permanent = mark;
}

int32_t __cdecl LargeLocalBegin(int32_t size)
{
    int32_t startPos; // [esp+0h] [ebp-4h]
    uint32_t sizea; // [esp+Ch] [ebp+8h]

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
            "g_largeLocalPos <= static_cast< int32_t >( sizeof( g_largeLocalBuf ) )");
    return startPos;
}

uint32_t __cdecl LargeLocalRoundSize(int32_t size)
{
    return (size + 3) & 0xFFFFFFFC;
}

void __cdecl LargeLocalEnd(int32_t startPos)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2556, 0, "%s", "Sys_IsMainThread()");
    g_largeLocalPos = startPos;
}

uint8_t* __cdecl LargeLocalGetBuf(int32_t startPos)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2620, 0, "%s", "Sys_IsMainThread()");
    return &g_largeLocalBuf[startPos];
}

LargeLocal::LargeLocal(int32_t sizeParam)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2648, 0, "%s", "Sys_IsMainThread()");

    this->startPos = LargeLocalBegin(sizeParam);
    this->size = sizeParam;
}

LargeLocal::~LargeLocal()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 2674, 0, "%s", "Sys_IsMainThread()");
    LargeLocalEnd(this->startPos);
}

uint8_t* LargeLocal::GetBuf()
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
    g_debugUser = Hunk_UserCreate(0x1000000, "Hunk_InitDebugMemory", 0, 0, 0);
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

void* Hunk_AllocDebugMem(uint32_t size)
{
    iassert(Sys_IsMainThread());
    iassert(g_debugUser);

    return Hunk_UserAlloc(g_debugUser, size, 4);
}

void __cdecl Hunk_FreeDebugMem(void* ptr)
{
    iassert(Sys_IsMainThread());
    iassert(g_debugUser);
}

HunkUser* __cdecl Hunk_UserCreate(int32_t maxSize, const char* name, bool fixed, bool tempMem, int32_t type)
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

void* Hunk_UserAlloc(HunkUser* user, uint32_t size, int32_t alignment)
{
    const char* v3; // eax
    int32_t pos; // [esp+4h] [ebp-10h]
    int32_t result; // [esp+8h] [ebp-Ch]
    HunkUser* current; // [esp+Ch] [ebp-8h]
    HunkUser* newCurrent; // [esp+10h] [ebp-4h]

    iassert(user);
    iassert(static_cast<uint>(size) <= user->maxSize - offsetof(HunkUser, buf));
    iassert(!(alignment & (alignment - 1)));
    iassert(alignment <= HUNK_MAX_ALIGNEMT);

    alignment = alignment - 1;

    for (current = user->current; ; current = newCurrent)
    {
        pos = current->pos;
        result = ~alignment & (alignment + pos);
        if ((signed int)(size + result) <= current->end)
            break;
        if (user->fixed)
            Com_Error(ERR_FATAL, "Hunk_UserAlloc: out of memory");
        newCurrent = Hunk_UserCreate(user->maxSize, user->name, 0, user->tempMem, user->type);
        user->current = newCurrent;
        current->next = newCurrent;
    }

    current->pos = size + (~alignment & (alignment + pos));
    pos = ((pos + 4095) & 0xFFFFF000);

    if (pos != ((current->pos + 4095) & 0xFFFFF000))
    {
        iassert(current->pos - pos > 0);
        Z_VirtualCommit((void*)pos, current->pos - (uint32_t)pos);
    }

    return (void*)result;
}

void* Hunk_UserAllocAlignStrict(HunkUser* user, uint32_t size)
{
    return Hunk_UserAlloc(user, size, 1);
}

void __cdecl Hunk_UserSetPos(HunkUser* user, uint8_t* pos)
{
    iassert(user->fixed);
    iassert(pos >= user->buf);
    iassert((uintptr_t)pos <= user->pos); // (psize_int)pos <= user->pos

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
    pos = (void*)(((uint32_t)&user[114].name + 3) & 0xFFFFF000);
    if (pos != (void*)((user->pos + 4095) & 0xFFFFF000))
    {
        if (user->pos - (int)pos <= 0)
            MyAssertHandler(".\\universal\\com_memory.cpp", 3019, 0, "%s", "user->pos - pos > 0");
        Z_VirtualDecommit(pos, user->pos - (uint32_t)pos);
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

uint8_t* __cdecl Hunk_AllocXModelPrecache(uint32_t size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecache", 21);
}

uint8_t* __cdecl Hunk_AllocXModelPrecacheColl(uint32_t size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecacheColl", 27);
}

int Hunk_SetMarkLow()
{
    iassert(Sys_IsMainThread());
    Hunk_CheckTempMemoryClear();
    return hunk_low.permanent;
}











static char* __cdecl Z_TryMallocGarbage(int32_t size, const char* name, int32_t type)
{
    char* buf; // [esp+0h] [ebp-4h]

    buf = (char*)malloc(size);
    // LWSS: remove this +32. Not needed
    //buf = (char*)malloc(size + 32);
    //if (buf)
    //{
    //    buf += 32;
    //    track_z_alloc(size + 72, name, type, buf, 0, 32); // KISAKMEMTRACK
    //}
    return buf;
}

static uint32_t* __cdecl Z_TryMalloc(int32_t size, const char* name, int32_t type)
{
    uint32_t* buf; // [esp+0h] [ebp-4h]

    buf = (uint32_t*)Z_TryMallocGarbage(size, name, type);
    if (buf)
        Com_Memset(buf, 0, size);
    return buf;
}

static void __cdecl Z_MallocFailed(int32_t size)
{
    Com_PrintError(16, "Failed to Z_Malloc %i bytes\n", size);
    Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 593);
}

void* Z_Malloc(int32_t size, const char* name, int32_t type)
{
    uint32_t* buf; // [esp+0h] [ebp-4h]

    buf = Z_TryMalloc(size, name, type);

    //if (!buf)
    //    Z_MallocFailed(size + 32);

    return buf;
}

void __cdecl Z_Free(void *ptr, int32_t type)
{
    if (ptr)
    {
       // track_z_free(type, ptr, 32);
       //free((char*)ptr - 32);
       free(ptr);
    }
}


char *__cdecl Z_MallocGarbage(int32_t size, const char *name, int32_t type)
{
    char *buf; // [esp+0h] [ebp-4h]

    buf = Z_TryMallocGarbage(size, name, type);
    //if (!buf)
    //    Z_MallocFailed(size + 32);
    return buf;
}

char *__cdecl TempMalloc(uint32_t len)
{
    return (char*)Hunk_UserAlloc(g_user, len, 1);
}

void __cdecl TempMemorySetPos(char *pos)
{
    Hunk_UserSetPos(g_user, (unsigned char*)pos);
}

void __cdecl TempMemoryReset(HunkUser *user)
{
    g_user = user;
}

char *__cdecl TempMallocAlignStrict(uint32_t len)
{
    return (char *)Hunk_UserAllocAlignStrict(g_user, len);
}

bool __cdecl TempInfoSort(TempMemInfo *info1, TempMemInfo *info2)
{
    if (!info1->data.type && info2->data.type)
        return 1;
    if (info1->data.type && !info2->data.type)
        return 0;
    if (info1->highExtra < info2->highExtra)
        return 1;
    if (info1->highExtra <= info2->highExtra)
        return info1->high < info2->high;
    return 0;
}