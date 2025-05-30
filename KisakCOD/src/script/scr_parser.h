#pragma once
#include "scr_stringlist.h"


struct OpcodeLookup // sizeof=0x18
{
    const char *codePos;
    unsigned int sourcePosIndex;
    unsigned int sourcePosCount;
    int profileTime;
    int profileBuiltInTime;
    int profileUsage;
};

struct Scr_SourcePos_t // sizeof=0xC
{                                       // ...
    unsigned int bufferIndex;           // ...
    int lineNum;                        // ...
    unsigned int sourcePos;             // ...
};

struct SourceBufferInfo // sizeof=0x2C
{
    const char *codePos;
    char *buf;
    const char *sourceBuf;
    int len;
    int sortedIndex;
    bool archive;
    // padding byte
    // padding byte
    // padding byte
    int time;
    int avgTime;
    int maxTime;
    float totalTime;
    float totalBuiltIn;
};

struct SourceLookup // sizeof=0x8
{
    unsigned int sourcePos;
    int type;
};

struct SaveSourceBufferInfo // sizeof=0x8
{
    char *sourceBuf;
    int len;
};

struct scrParserGlob_t // sizeof=0x34
{                                       // ...
    OpcodeLookup *opcodeLookup;         // ...
    unsigned int opcodeLookupMaxLen;    // ...
    unsigned int opcodeLookupLen;       // ...
    SourceLookup *sourcePosLookup;      // ...
    unsigned int sourcePosLookupMaxLen; // ...
    unsigned int sourcePosLookupLen;    // ...
    unsigned int sourceBufferLookupMaxLen; // ...
    const unsigned __int8 *currentCodePos; // ...
    unsigned int currentSourcePosCount; // ...
    SaveSourceBufferInfo *saveSourceBufferLookup; // ...
    unsigned int saveSourceBufferLookupLen; // ...
    int delayedSourceIndex;             // ...
    int threadStartSourceIndex;         // ...
};

struct scrParserPub_t // sizeof=0x10
{                                       // ...
    SourceBufferInfo *sourceBufferLookup; // ...
    unsigned int sourceBufferLookupLen; // ...
    const char *scriptfilename;         // ...
    const char *sourceBuf;              // ...
};

void __cdecl TRACK_scr_parser();
void __cdecl Scr_InitOpcodeLookup();
void __cdecl Scr_ShutdownOpcodeLookup();
void __cdecl AddOpcodePos(unsigned int sourcePos, int type);
void __cdecl RemoveOpcodePos();
void __cdecl AddThreadStartOpcodePos(unsigned int sourcePos);
const char *__cdecl Scr_GetOpcodePosOfType(
    unsigned int bufferIndex,
    unsigned int startSourcePos,
    unsigned int endSourcePos,
    int type,
    unsigned int *sourcePos);
unsigned int __cdecl Scr_GetClosestSourcePosOfType(unsigned int bufferIndex, unsigned int sourcePos, int type);
unsigned int __cdecl Scr_GetPrevSourcePos(const char *codePos, unsigned int index);
OpcodeLookup *__cdecl Scr_GetPrevSourcePosOpcodeLookup(const char *codePos);
void __cdecl Scr_SelectScriptLine(unsigned int bufferIndex, int lineNum);
unsigned int __cdecl Scr_GetLineNum(unsigned int bufferIndex, unsigned int sourcePos);
unsigned int __cdecl Scr_GetLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine, int *col);
unsigned int __cdecl Scr_GetFunctionLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine);
int __cdecl Scr_GetSourcePosOfType(const char *codePos, int type, Scr_SourcePos_t *pos);
OpcodeLookup *__cdecl Scr_GetSourcePosOpcodeLookup(const char *codePos);
void __cdecl Scr_AddSourceBufferInternal(
    const char *extFilename,
    const char *codePos,
    char *sourceBuf,
    int len,
    bool doEolFixup,
    bool archive);
SourceBufferInfo *__cdecl Scr_GetNewSourceBuffer();
char *__cdecl Scr_AddSourceBuffer(const char *filename, char *extFilename, const char *codePos, bool archive);
char *__cdecl Scr_ReadFile(const char *filename, char *extFilename, const char *codePos, bool archive);
char *__cdecl Scr_ReadFile_FastFile(const char *filename, const char *extFilename, const char *codePos, bool archive);
unsigned int __cdecl Scr_GetSourcePos(
    unsigned int bufferIndex,
    unsigned int sourcePos,
    char *outBuf,
    unsigned int outBufLen);
unsigned int __cdecl Scr_GetLineInfo(const char *buf, unsigned int sourcePos, int *col, char *line);
void __cdecl Scr_CopyFormattedLine(char *line, const char *rawLine);
unsigned int __cdecl Scr_GetSourceBuffer(const char *codePos);
void __cdecl Scr_PrintPrevCodePos(int channel, char *codePos, unsigned int index);
void __cdecl Scr_PrintSourcePos(int channel, const char *filename, const char *buf, unsigned int sourcePos);
const char *__cdecl Scr_PrevCodePosFileName(char *codePos);
const char *__cdecl Scr_PrevCodePosFunctionName(char *codePos);
bool __cdecl Scr_PrevCodePosFileNameMatches(char *codePos, const char *fileName);
void __cdecl Scr_PrintPrevCodePosSpreadSheet(int channel, char *codePos, bool summary, bool functionSummary);
void __cdecl Scr_PrintSourcePosSpreadSheet(int channel, const char *filename, const char *buf, unsigned int sourcePos);
void __cdecl Scr_PrintFunctionPosSpreadSheet(
    int channel,
    const char *filename,
    const char *buf,
    unsigned int sourcePos);
unsigned int __cdecl Scr_GetFunctionInfo(const char *buf, unsigned int sourcePos, char *line);
void __cdecl Scr_PrintSourcePosSummary(int channel, const char *filename);
void __cdecl Scr_GetCodePos(const char *codePos, unsigned int index, char *outBuf, unsigned int outBufLen);
void __cdecl Scr_GetFileAndLine(const char *codePos, char **filename, int *linenum);
void __cdecl Scr_AddProfileTime(const char *codePos, int time, int builtInTime);
void __cdecl Scr_CalcScriptFileProfile();
bool __cdecl Scr_CompareScriptSourceProfileTimes(int index1, int index2);
void __cdecl Scr_CalcAnimscriptProfile(int *total, int *totalNonBuiltIn);
char __cdecl Scr_PrintProfileTimes(float minTime);
bool __cdecl Scr_CompareProfileTimes(const OpcodeLookup& opcodeLookup1, const OpcodeLookup& opcodeLookup2);
void CompileError(unsigned int sourcePos, const char *msg, ...);
scrStringDebugGlob_t *Scr_IgnoreLeaks();
void CompileError2(char *codePos, const char *msg, ...);
void __cdecl Scr_GetTextSourcePos(const char *buf, char *codePos, char *line);
void __cdecl RuntimeError(char *codePos, unsigned int index, const char *msg, const char *dialogMessage);
void __cdecl RuntimeErrorInternal(int channel, char *codePos, unsigned int index, const char *msg);



extern scrParserGlob_t scrParserGlob;
extern scrParserPub_t scrParserPub;
extern char g_EndPos;
extern bool g_loadedImpureScript;