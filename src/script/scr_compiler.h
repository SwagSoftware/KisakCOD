#pragma once

#include "scr_debugger.h"

enum : __int32
{
    SOURCE_TYPE_NONE = 0,
    SOURCE_TYPE_BREAKPOINT = 0x1,
    SOURCE_TYPE_CALL = 0x2,
    SOURCE_TYPE_THREAD_START = 0x4,
    SOURCE_TYPE_BUILTIN_CALL = 0x8,
    SOURCE_TYPE_NOTIFY = 0x10,
};
enum : __int32
{
    SCR_DEV_NO = 0x0,
    SCR_DEV_YES = 0x1,
    SCR_DEV_IGNORE = 0x2,
    SCR_DEV_EVALUATE = 0x3,
};

enum : __int32
{
    SCR_ABORT_NONE = 0x0,
    SCR_ABORT_CONTINUE = 0x1,
    SCR_ABORT_BREAK = 0x2,
    SCR_ABORT_RETURN = 0x3,
    SCR_ABORT_MAX = 0x3,
};

struct CaseStatementInfo // sizeof=0x10
{
    unsigned int name;
    const char *codePos;
    unsigned int sourcePos;
    CaseStatementInfo *next;
};
static_assert(sizeof(CaseStatementInfo) == 0x10);

struct BreakStatementInfo // sizeof=0xC
{
    char *codePos;
    const char *nextCodePos;
    BreakStatementInfo *next;
};
static_assert(sizeof(BreakStatementInfo) == 0xC);

struct ContinueStatementInfo // sizeof=0xC
{
    char *codePos;
    const char *nextCodePos;
    ContinueStatementInfo *next;
};
static_assert(sizeof(ContinueStatementInfo) == 0xC);

struct VariableCompileValue // sizeof=0xC
{                                       // ...
    VariableValue value;                // ...
    sval_u sourcePos;
};
static_assert(sizeof(VariableCompileValue) == 0xC);

#define VALUE_STACK_SIZE 32

struct scrCompileGlob_t // sizeof=0x1D8
{                                       // ...
    unsigned __int8 *codePos;           // ...
    unsigned __int8 *prevOpcodePos;     // ...
    unsigned int fileId;                // ...
    unsigned int threadId;              // ...
    int cumulOffset;                    // ...
    int maxOffset;                      // ...
    int maxCallOffset;                  // ...
    bool bConstRefCount;                // ...
    bool in_developer_thread;           // ...
    // padding byte
    // padding byte
    unsigned int developer_thread_sourcePos; // ...
    bool firstThread[2];                // ...
    // padding byte
    // padding byte
    CaseStatementInfo *currentCaseStatement; // ...
    bool bCanBreak;                     // ...
    // padding byte
    // padding byte
    // padding byte
    BreakStatementInfo *currentBreakStatement; // ...
    bool bCanContinue;                  // ...
    // padding byte
    // padding byte
    // padding byte
    ContinueStatementInfo *currentContinueStatement; // ...
    scr_block_s **breakChildBlocks;     // ...
    int *breakChildCount;               // ...
    scr_block_s *breakBlock;            // ...
    scr_block_s **continueChildBlocks;  // ...
    int *continueChildCount;            // ...
    bool forceNotCreate;                // ...
    // padding byte
    // padding byte
    // padding byte
    struct PrecacheEntry *precachescriptList;  // ...
    VariableCompileValue value_start[VALUE_STACK_SIZE]; // ...
};
static_assert(sizeof(scrCompileGlob_t) == 0x1D8);

#define SCR_FUNC_TABLE_SIZE 1024

struct scrCompilePub_t
{
    int value_count;
    int far_function_count;
    unsigned int loadedscripts;
    unsigned int scripts;
    unsigned int builtinFunc;
    unsigned int builtinMeth;
    short canonicalStrings[65536];
    const char *in_ptr;
    const char *parseBuf;
    bool script_loading;
    bool allowedBreakpoint;
    int developer_statement;
    unsigned char *opcodePos;
    unsigned int programLen;
    int func_table_size;
    int func_table[SCR_FUNC_TABLE_SIZE];
};

void __cdecl Scr_CompileStatement(sval_u parseData);
void __cdecl ScriptCompile(
    sval_u val,
    unsigned int fileId,
    unsigned int scriptId,
    struct PrecacheEntry *entries,
    int entriesCount);

extern scrCompilePub_t scrCompilePub;
extern scrCompileGlob_t scrCompileGlob;