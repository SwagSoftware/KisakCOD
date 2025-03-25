#pragma once
#include <cgame/cg_local.h>
#include "r_material.h"
#include "rb_backend.h"
#include "fxprimitives.h"

struct r_double_index_t // sizeof=0x4
{                                       // ...
    unsigned __int16 value[2];          // ...
};

struct WorkerCmds // sizeof=0x80
{                                       // ...
    volatile int startPos;
    volatile int endPos;
    volatile int syncedEndPos;
    volatile int inSize;                // ...
    volatile int outSize;               // ...
    unsigned int dataSize;              // ...
    unsigned __int8 *buf;               // ...
    int bufSize;                        // ...
    int bufCount;
    unsigned int pad[23];
};

bool __cdecl R_FXNonDependentOrSpotLightPending();
bool __cdecl R_FXSpotLightPending();
bool __cdecl R_FXNonDependentPending();
bool __cdecl R_EndFenceBusy();
void __cdecl TRACK_r_workercmds();
void __cdecl R_WaitWorkerCmdsOfType(int type);
void __cdecl R_NotifyWorkerCmdType(int type);
bool __cdecl R_WorkerCmdsFinished();
void __cdecl R_ProcessWorkerCmds();
int __cdecl R_ProcessWorkerCmd(int type);
void __cdecl R_ProcessWorkerCmdInternal(int type, FxCmd *data);
void R_InitWorkerThreads();
int R_InitWorkerCmds();
int R_InitWorkerCmdsPos();
void __cdecl  R_WorkerThread();
void __cdecl R_AddWorkerCmd(int type, unsigned __int8 *data);
void __cdecl R_UpdateActiveWorkerThreads();
void __cdecl R_WaitFrontendWorkerCmds();
int __cdecl R_FinishedWorkerCmds();
void __cdecl R_WaitWorkerCmds();
