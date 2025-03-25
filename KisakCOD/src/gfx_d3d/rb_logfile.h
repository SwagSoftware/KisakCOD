#pragma once
#include "r_rendercmds.h"

struct StencilLogBits // sizeof=0x18
{                                       // ...
    const char *description;            // ...
    int enableMask;                     // ...
    int passShift;                      // ...
    int failShift;                      // ...
    int zfailShift;                     // ...
    int funcShift;                      // ...
};

void __cdecl RB_UpdateLogging();
int RB_CloseLogFile();
void RB_OpenLogFile();
bool __cdecl RB_IsLogging();
void __cdecl RB_LogPrint(const char *text);
const char *__cdecl RB_LogTechniqueType(MaterialTechniqueType techType);
void __cdecl RB_LogPrintState_0(int stateBits0, int changedBits0);
void __cdecl RB_LogBlend(const char *format, unsigned int blend);
void __cdecl RB_LogBlendOp(const char *format, unsigned int blendOp);
void __cdecl RB_LogFromTable(
    const char *keyName,
    int stateBits,
    int changedBits,
    int bitMask,
    char bitShift,
    const StateBitsTable *table,
    int tableCount);
void __cdecl RB_LogPrintState_1(int stateBits1, int changedBits1);
void __cdecl RB_LogBool(
    const char *keyName,
    int stateBits,
    int changedBits,
    int bitMask,
    const char *trueName,
    const char *falseName);
void __cdecl RB_LogStencilState(int stateBits1, int changedBits1, const StencilLogBits *desc);

extern const dvar_s *r_logFile;