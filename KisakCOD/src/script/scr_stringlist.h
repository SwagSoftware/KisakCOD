#pragma once

#define HASH_STAT_FREE      0
#define HASH_STAT_MOVABLE   0x10000
#define HASH_STAT_HEAD      0x20000
#define HASH_STAT_MASK      0x30000

#define SL_MAX_STRING_INDEX 0x10000

union HashEntry_unnamed_type_u
{           
    HashEntry_unnamed_type_u()
    {
        prev = 0;
    }
    HashEntry_unnamed_type_u(unsigned int val)
    {
        prev = val;
    }
    operator unsigned int() const
    {
        return prev;
    }
    unsigned int prev;
    unsigned int str;
};

struct HashEntry
{               
    unsigned int status_next;
    HashEntry_unnamed_type_u u;
};

struct __declspec(align(128)) scrStringGlob_t
{                                       
    HashEntry hashTable[20000];         
    bool inited;                        
    HashEntry * nextFreeEntry;
};

 struct RefString
 {
     union
     {
         struct
         {
             unsigned __int32 refCount : 16;
             unsigned __int32 user : 8;
             unsigned __int32 byteLen : 8;
         };
         volatile unsigned int data;
     };
     char str[1];
 };

#define MT_NODE_SIZE 12
#define MT_SIZE 0xC0000

struct scrMemTreePub_t
{                     
    char *mt_buffer;  
};

struct scrStringDebugGlob_t
{
    volatile unsigned int refCount[65536];
    volatile unsigned int totalRefCount;
    int ignoreLeaks;
};

void SL_Init();
void SL_InitCheckLeaks();

void SL_Shutdown();
void SL_ShutdownSystem(unsigned int user);

void SL_TransferSystem(unsigned int from, unsigned int to);

void SL_BeginLoadScripts();
void SL_EndLoadScripts();

void __cdecl SL_AddUser(unsigned int stringValue, unsigned int user);
void SL_AddUserInternal(RefString* refStr, unsigned int user);

void SL_AddRefToString(unsigned int stringValue);

unsigned int SL_GetString_(const char* str, unsigned int user, int type);
HashEntry_unnamed_type_u SL_GetStringOfSize(const char* str, unsigned int user, unsigned int len, int type);
char* SL_ConvertToString(unsigned int stringValue);
RefString* GetRefString(unsigned int stringValue);
RefString* GetRefString(const char* str);

void SL_CheckExists(unsigned int stringValue);

unsigned int SL_GetStringForVector(const float* v);
unsigned int SL_GetStringForInt(int i);
unsigned int SL_GetStringForFloat(float f);
unsigned int SL_GetString(const char* str, unsigned int user);
unsigned int SL_GetLowercaseString_(const char* str, unsigned int user, int type);
unsigned int SL_GetLowercaseString(const char* str, unsigned int user);

void __cdecl SL_TransferRefToUser(unsigned int stringValue, unsigned int user);

int SL_GetRefStringLen(RefString* refString);
int SL_GetStringLen(unsigned int stringValue);

HashEntry_unnamed_type_u SL_FindLowercaseString(const char* str);

const char* SL_DebugConvertToString(unsigned int stringValue);
unsigned int SL_ConvertFromString(const char* str);

unsigned int SL_FindString(const char* str);
void SL_RemoveRefToString(unsigned int stringValue);
void SL_RemoveRefToStringOfSize(unsigned int stringValue, unsigned int len);

int SL_IsLowercaseString(unsigned int stringValue);

void __cdecl Scr_SetString(unsigned __int16 *to, unsigned int from);

HashEntry_unnamed_type_u __cdecl SL_ConvertToLowercase(unsigned int stringValue, unsigned int user, int type);

HashEntry_unnamed_type_u __cdecl Scr_CreateCanonicalFilename(const char *filename);

HashEntry_unnamed_type_u __cdecl Scr_AllocString(char *s, int sys);