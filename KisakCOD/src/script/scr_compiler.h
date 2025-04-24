#pragma once

#include "scr_debugger.h"

struct CaseStatementInfo // sizeof=0x10
{
    unsigned int name;
    const char *codePos;
    unsigned int sourcePos;
    CaseStatementInfo *next;
};
struct BreakStatementInfo // sizeof=0xC
{
    char *codePos;
    const char *nextCodePos;
    BreakStatementInfo *next;
};
struct ContinueStatementInfo // sizeof=0xC
{
    char *codePos;
    const char *nextCodePos;
    ContinueStatementInfo *next;
};
struct VariableCompileValue // sizeof=0xC
{                                       // ...
    VariableValue value;                // ...
    sval_u sourcePos;
};
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
    PrecacheEntry *precachescriptList;  // ...
    VariableCompileValue value_start[32]; // ...
};

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
    int func_table[1024];
};

void __cdecl Scr_CompileStatement(sval_u parseData);

extern scrCompilePub_t scrCompilePub;
extern scrCompileGlob_t scrCompileGlob;