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
    uint16_t canonicalStrCount;
    bool developer;
    bool developer_script;
    bool evaluate;
    const char* error_message;
    int error_index;
    uint32_t time;
    uint32_t timeArrayId;
    uint32_t pauseArrayId;
    uint32_t levelId;
    uint32_t gameId;
    uint32_t animId;
    uint32_t freeEntList;
    uint32_t tempVariable;
    bool bInited;
    uint16_t savecount;
    uint32_t checksum;
    uint32_t entId;
    uint32_t entFieldName;
    HunkUser* programHunkUser;
    const char* programBuffer;
    const char* endScriptBuffer;
    uint16_t saveIdMap[32768];
    uint16_t saveIdMapRev[32768];
    bool bScriptProfile;
    float scriptProfileMinTime;
    bool bScriptProfileBuiltin;
    float scriptProfileBuiltinMinTime;
    uint32_t numScriptThreads;
    uint32_t numScriptValues;
    uint32_t numScriptObjects;
    const char* varUsagePos;
    int ext_threadcount;
    int totalObjectRefCount;
    volatile uint32_t totalVectorRefCount;
};
static_assert(sizeof(scrVarPub_t) == 0x2007C);

struct PrecacheEntry // sizeof=0x8
{                                       // ...
    uint16_t filename;
    bool include;
    // padding byte
    uint32_t sourcePos;
};
static_assert(sizeof(PrecacheEntry) == 0x8);

extern scrVarPub_t scrVarPub;
extern scrVarDebugPub_t scrVarDebugPubBuf;

bool Scr_IsInOpcodeMemory(char const* pos);
bool Scr_IsIdentifier(char const* token);

int Scr_GetFunctionHandle(char const*, char const*);
uint32_t SL_TransferToCanonicalString(uint32_t);
uint32_t SL_GetCanonicalString(char const*);
void Scr_BeginLoadScriptsRemote(void);
void Scr_BeginLoadAnimTrees(int);
int Scr_ScanFile(unsigned char*, int);
uint32_t Scr_LoadScriptInternal(char const*, struct PrecacheEntry*, int);
uint32_t Scr_LoadScript(char const*);
void Scr_PostCompileScripts(void);
void Scr_EndLoadScripts(void);
void Scr_PrecacheAnimTrees(void* (__cdecl*)(int), int);
void Scr_EndLoadAnimTrees(void);
void Scr_FreeScripts(unsigned char);
void Scr_BeginLoadScripts(void);


//int marker_scr_main      83043248     scr_main.obj
//int Scr_IsInScriptMemory(char const*);

extern scrVarDebugPub_t *scrVarDebugPub;