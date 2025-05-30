#pragma once
#include "r_material.h"
#include "rb_backend.h"
#include "fxprimitives.h"

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

int __cdecl R_FXNonDependentOrSpotLightPending(void* args);
bool __cdecl R_FXSpotLightPending();
bool __cdecl R_FXNonDependentPending();
int __cdecl R_EndFenceBusy(void* args);
void __cdecl TRACK_r_workercmds();
void __cdecl R_WaitWorkerCmdsOfType(int type);
void __cdecl R_NotifyWorkerCmdType(int type);
int __cdecl R_WorkerCmdsFinished();
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
void __cdecl R_ProcessWorkerCmdsWithTimeout(int(__cdecl *timeout)(), int forever);

static int(__cdecl *g_cmdOutputBusy[17])(void *) =
{
  NULL,
  NULL,
  &R_FXNonDependentOrSpotLightPending,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &R_EndFenceBusy,
  &R_EndFenceBusy,
  &R_EndFenceBusy,
  &R_EndFenceBusy
}; // idb