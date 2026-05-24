#pragma once
#include <cstdint>

#define HASH_STAT_FREE      0
#define HASH_STAT_MOVABLE   0x10000
#define HASH_STAT_HEAD      0x20000
#define HASH_STAT_MASK      0x30000

#define SL_MAX_STRING_INDEX 0x10000

#define STRINGLIST_SIZE 20'000

union HashEntry_unnamed_type_u
{           
    uint32_t prev;
    uint32_t str;
};

struct HashEntry
{               
    uint32_t status_next;
    HashEntry_unnamed_type_u u;
};

struct __declspec(align(128)) scrStringGlob_t
{                                       
    HashEntry hashTable[20000];         
    bool inited;                        
    HashEntry *nextFreeEntry;
};

 struct RefString
 {
     union
     {
         struct
         {
             unsigned __int32 refCount : 16;
             unsigned __int32 user : 8;
             unsigned __int32 byteLen : 8; // includes null terminator
         };
         volatile uint32_t data;
     };
     char str[1];
 };

 struct RefVector
 {
     union
     {
         struct
         {
             uint32_t refCount : 16;
             uint32_t user : 8;
             uint32_t byteLen : 8;
         };
         volatile int head;
     };
     float vec[3];
 };

//#define MT_NODE_SIZE 12
#define MT_NODE_SIZE (sizeof(MemoryNode))
#define MT_SIZE 0xC0000

struct scrMemTreePub_t
{                     
    char *mt_buffer;  //     scrMemTreePub.mt_buffer = (char*)&scrMemTreeGlob.nodes;
};

struct scrStringDebugGlob_t
{
    volatile uint32_t refCount[65536];
    volatile uint32_t totalRefCount;
    int ignoreLeaks;
};

void SL_Init();
void SL_InitCheckLeaks();

void SL_Shutdown();
void SL_ShutdownSystem(uint32_t user);

void SL_TransferSystem(uint32_t from, uint32_t to);

void SL_BeginLoadScripts();
void SL_EndLoadScripts();

void __cdecl SL_AddUser(uint32_t stringValue, uint32_t user);
void SL_AddUserInternal(RefString* refStr, uint32_t user);

void SL_AddRefToString(uint32_t stringValue);

uint32_t SL_GetString_(const char* str, uint32_t user, int type);
uint32_t SL_GetStringOfSize(const char* str, uint32_t user, uint32_t len, int type);
const char* SL_ConvertToString(uint32_t stringValue);
const char *SL_ConvertToStringSafe(uint32_t stringValue);
RefString* GetRefString(uint32_t stringValue);
RefString* GetRefString(const char* str);

void SL_CheckExists(uint32_t stringValue);

uint32_t SL_GetStringForVector(const float* v);
uint32_t SL_GetStringForInt(int i);
uint32_t SL_GetStringForFloat(float f);
uint32_t SL_GetString(const char* str, uint32_t user);
uint32_t SL_GetLowercaseString_(const char* str, uint32_t user, int type);
uint32_t SL_GetLowercaseString(const char* str, uint32_t user);

void __cdecl SL_TransferRefToUser(uint32_t stringValue, uint32_t user);

int SL_GetRefStringLen(RefString* refString);
int SL_GetStringLen(uint32_t stringValue);

uint32_t SL_FindLowercaseString(const char* str);

const char* SL_DebugConvertToString(uint32_t stringValue);
uint32_t SL_ConvertFromString(const char* str);

uint32_t SL_FindString(const char* str);
void SL_RemoveRefToString(uint32_t stringValue);
void SL_RemoveRefToStringOfSize(uint32_t stringValue, uint32_t len);

int SL_IsLowercaseString(uint32_t stringValue);

void __cdecl Scr_SetString(uint16_t *to, uint32_t from);

uint32_t __cdecl SL_ConvertToLowercase(uint32_t stringValue, uint32_t user, int type);

uint32_t __cdecl Scr_CreateCanonicalFilename(const char *filename);

void Scr_SetStringFromCharString(uint16_t *to, const char *from);
uint32_t SL_GetUser(uint32_t stringValue);

uint32_t __cdecl Scr_AllocString(char *s, int sys);