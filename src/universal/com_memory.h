#pragma once

#include <script/scr_stringlist.h>
#include "q_shared.h"
#include <qcommon/mem_track.h>

#define HUNK_MAX_ALIGNEMT 4096

struct TempMemInfo // sizeof=0x28
{                                       // ...
    int permanent;
    int high;
    int highExtra;
    int hunkSize;
    int low;
    mem_track_t data;                   // ...
};

void __cdecl Hunk_AddAsset(XAssetHeader header, _DWORD *data);

void Com_TouchMemory();

uint8_t* __cdecl Hunk_AllocXAnimPrecache(uint32_t size);
uint8_t* __cdecl Hunk_AllocPhysPresetPrecache(uint32_t size);
void* __cdecl Hunk_AllocXAnimClient(int size);
uint8_t* __cdecl Hunk_AllocXAnimServer(uint32_t size);

//void __cdecl TRACK_com_memory();

// LWSS: Note that the Z_ prefix comes from the fact that it uses the "Zone" memory pool.
// There are a few memory pools of fixed size that allocations come from.
void* __cdecl Z_VirtualReserve(int size);
void __cdecl Z_VirtualDecommitInternal(void* ptr, int size);
void __cdecl Z_VirtualFreeInternal(void* ptr);
void* __cdecl Z_TryVirtualAllocInternal(int size);
bool __cdecl Z_TryVirtualCommitInternal(void* ptr, int size);
void __cdecl Z_VirtualCommitInternal(void* ptr, int size);
void __cdecl Z_VirtualFree(void* ptr);
void __cdecl Z_VirtualDecommit(void* ptr, int size);
char* __cdecl Z_TryVirtualAlloc(int size, const char* name, int type);
char* __cdecl Z_VirtualAlloc(int size, const char* name, int type);
void __cdecl Z_VirtualCommit(void* ptr, int size);


// LWSS: COD4 notably removes the memtag_t (Doom3BFG has these as well)
// Instead they use a per-file memory tracking system that links to qcommon\\mem_track.cpp (Anything starting with track_)
void* Z_Malloc(int size, const char* name, int type);
void  Z_Free(void* ptr, int type);

char *__cdecl Z_MallocGarbage(int size, const char *name, int type);

const char* CopyString(const char* in);
void __cdecl ReplaceString(const char** str, const char* in);
void __cdecl FreeString(const char* str);

void Com_InitHunkMemory();
void __cdecl Com_Meminfo_f();

struct HunkUser // sizeof=0x24
{
    HunkUser* current;
    HunkUser* next;
    int maxSize;
    int end;
    int pos;
    const char* name;
    bool fixed;
    bool tempMem;
    // padding byte
    // padding byte
    int type;
    uint8_t buf[1];
    // padding byte
    // padding byte
    // padding byte
};

// KISAKTODO: Move to proper spot?
struct fileData_s // sizeof=0xC
{
    void* data;
    fileData_s* next;
    uint8_t type;
    char name[1];
    // padding byte
    // padding byte
};

void Hunk_ClearData();
void __cdecl Hunk_ClearDataFor(fileData_s** pFileData, uint8_t* low, uint8_t* high);
void __cdecl Hunk_ClearToMarkLow(int mark);
void Hunk_Clear();
int __cdecl Hunk_Used();
uint8_t* __cdecl Hunk_Alloc(uint32_t size, const char* name, int type);
uint8_t* __cdecl Hunk_AllocAlign(uint32_t size, int alignment, const char* name, int type);
uint32_t __cdecl Hunk_AllocateTempMemoryHigh(int size, const char* name);
void Hunk_ClearTempMemoryHigh();
uint8_t* __cdecl Hunk_AllocLow(uint32_t size, const char* name, int type);
uint8_t* __cdecl Hunk_AllocLowAlign(uint32_t size, int alignment, const char* name, int type);
uint32_t* __cdecl Hunk_AllocateTempMemory(int size, const char* name);
void __cdecl Hunk_FreeTempMemory(char* buf);
void Hunk_ClearTempMemory();
void Hunk_CheckTempMemoryClear();
void Hunk_CheckTempMemoryHighClear();
int __cdecl Hunk_HideTempMemory();
void __cdecl Hunk_ShowTempMemory(int mark);
void __cdecl Hunk_InitDebugMemory();
void __cdecl Hunk_ShutdownDebugMemory();
void __cdecl Hunk_ResetDebugMem();
void* Hunk_AllocDebugMem(uint32_t size);
inline void *Hunk_AllocDebugMem(uint32_t size, const char *why) // why is rarely used and unused in this project
{
    return Hunk_AllocDebugMem(size);
}

void __cdecl Hunk_FreeDebugMem(void* ptr = NULL);
HunkUser* __cdecl Hunk_UserCreate(int maxSize, const char* name, bool fixed, bool tempMem, int type);
void* Hunk_UserAlloc(HunkUser* user, uint32_t size, int alignment);
void* Hunk_UserAllocAlignStrict(HunkUser* user, uint32_t size);
void __cdecl Hunk_UserSetPos(HunkUser* user, uint8_t* pos);
void __cdecl Hunk_UserReset(HunkUser* user);
void __cdecl Hunk_UserDestroy(HunkUser* user);
char* __cdecl Hunk_CopyString(HunkUser* user, const char* in);
uint8_t* __cdecl Hunk_AllocXModelPrecache(uint32_t size);
uint8_t* __cdecl Hunk_AllocXModelPrecacheColl(uint32_t size);
void* __cdecl Hunk_FindDataForFile(int type, const char* name);
void* __cdecl Hunk_FindDataForFileInternal(int type, const char* name, int hash);
bool __cdecl Hunk_DataOnHunk(uint8_t* data);
char* __cdecl Hunk_SetDataForFile(int type, const char* name, void* data, void* (__cdecl* alloc)(int));
void __cdecl Hunk_AddData(int type, void* data, void* (__cdecl* alloc)(int));
int Hunk_SetMarkLow();

char *__cdecl TempMalloc(uint32_t len);
void __cdecl TempMemorySetPos(char *pos);
void __cdecl TempMemoryReset(HunkUser *user);
bool __cdecl TempInfoSort(TempMemInfo *info1, TempMemInfo *info2);
char *__cdecl TempMallocAlignStrict(uint32_t len);
void __cdecl TempMemoryReset(HunkUser *user);


class LargeLocal
{
public:
    LargeLocal(int sizeParam);
    ~LargeLocal();

    uint8_t* GetBuf();

    int startPos;
    int size;
};

int __cdecl LargeLocalBegin(int size);
void __cdecl LargeLocalEnd(int startPos);
uint8_t* __cdecl LargeLocalGetBuf(int startPos);
void __cdecl LargeLocalReset();
uint32_t __cdecl LargeLocalRoundSize(int size);


extern unsigned char *s_hunkData;
extern uint8_t *s_origHunkData;
extern int s_hunkTotal;