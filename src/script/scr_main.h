#pragma once

#include <universal/q_shared.h>

#include <universal/com_memory.h>
#include "scr_variable.h"

static const char *var_typename[] =
{
    "undefined",
    "object",
    "string",
    "localized string",
    "vector",
    "float",
    "int",
    "codepos",
    "precodepos",
    "function",
    "stack",
    "animation",
    "developer codepos",
    "include codepos",
    "thread",
    "thread",
    "thread",
    "thread",
    "struct",
    "removed entity",
    "entity",
    "array",
    "removed thread",
};

struct scrVarPub_t // sizeof=0x2007C
{
    char* fieldBuffer;
    unsigned __int16 canonicalStrCount;
    bool developer;
    bool developer_script;
    bool evaluate;
    const char* error_message;
    int error_index;
    unsigned int time;
    unsigned int timeArrayId;
    unsigned int pauseArrayId;
    unsigned int levelId;
    unsigned int gameId;
    unsigned int animId;
    unsigned int freeEntList;
    unsigned int tempVariable;
    bool bInited;
    unsigned __int16 savecount;
    unsigned int checksum;
    unsigned int entId;
    unsigned int entFieldName;
    HunkUser* programHunkUser;
    const char* programBuffer;
    const char* endScriptBuffer;
    unsigned __int16 saveIdMap[32768];
    unsigned __int16 saveIdMapRev[32768];
    bool bScriptProfile;
    float scriptProfileMinTime;
    bool bScriptProfileBuiltin;
    float scriptProfileBuiltinMinTime;
    unsigned int numScriptThreads;
    unsigned int numScriptValues;
    unsigned int numScriptObjects;
    const char* varUsagePos;
    int ext_threadcount;
    int totalObjectRefCount;
    volatile unsigned int totalVectorRefCount;
};

struct PrecacheEntry // sizeof=0x8
{                                       // ...
    unsigned __int16 filename;
    bool include;
    // padding byte
    unsigned int sourcePos;
};

extern scrVarPub_t scrVarPub;
extern scrVarDebugPub_t scrVarDebugPubBuf;

bool Scr_IsInOpcodeMemory(char const* pos);
bool Scr_IsIdentifier(char const* token);

int Scr_GetFunctionHandle(char const*, char const*);
unsigned int SL_TransferToCanonicalString(unsigned int);
unsigned int SL_GetCanonicalString(char const*);
void Scr_BeginLoadScriptsRemote(void);
void Scr_BeginLoadAnimTrees(int);
int Scr_ScanFile(unsigned char*, int);
unsigned int Scr_LoadScriptInternal(char const*, struct PrecacheEntry*, int);
unsigned int Scr_LoadScript(char const*);
void Scr_PostCompileScripts(void);
void Scr_EndLoadScripts(void);
void Scr_PrecacheAnimTrees(void* (__cdecl*)(int), int);
void Scr_EndLoadAnimTrees(void);
void Scr_FreeScripts(unsigned char);
void Scr_BeginLoadScripts(void);


//int marker_scr_main      83043248     scr_main.obj
//int Scr_IsInScriptMemory(char const*);

extern scrVarDebugPub_t *scrVarDebugPub;