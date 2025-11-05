#pragma once
#include "r_material.h"
#include "rb_backend.h"
#include "fxprimitives.h"

enum WorkerCmdType : __int32
{
    WRKCMD_FIRST_FRONTEND = 0x0,
    WRKCMD_UPDATE_FX_SPOT_LIGHT = 0x0,
    WRKCMD_UPDATE_FX_NON_DEPENDENT = 0x1,
    WRKCMD_UPDATE_FX_REMAINING = 0x2,
    WRKCMD_DPVS_CELL_STATIC = 0x3,
    WRKCMD_DPVS_CELL_SCENE_ENT = 0x4,
    WRKCMD_DPVS_CELL_DYN_MODEL = 0x5,
    WRKCMD_DPVS_CELL_DYN_BRUSH = 0x6,
    WRKCMD_DPVS_ENTITY = 0x7,
    WRKCMD_ADD_SCENE_ENT = 0x8,
    WRKCMD_SPOT_SHADOW_ENT = 0x9,
    WRKCMD_SHADOW_COOKIE = 0xA,
    WRKCMD_BOUNDS_ENT_DELAYED = 0xB,
    WRKCMD_SKIN_ENT_DELAYED = 0xC,
    WRKCMD_GENERATE_FX_VERTS = 0xD,
    WRKCMD_GENERATE_MARK_VERTS = 0xE,
    WRKCMD_SKIN_CACHED_STATICMODEL = 0xF,
    WRKCMD_SKIN_XMODEL = 0x10,
    WRKCMD_COUNT = 0x11,
};
inline WorkerCmdType &operator++(WorkerCmdType &e) {
    e = static_cast<WorkerCmdType>(static_cast<int>(e) + 1);
    return e;
}
inline WorkerCmdType &operator++(WorkerCmdType &e, int i)
{
    ++e;
    return e;
}

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
void __cdecl R_WaitWorkerCmdsOfType(WorkerCmdType type);
void __cdecl R_NotifyWorkerCmdType(WorkerCmdType type);
int __cdecl R_WorkerCmdsFinished();
void __cdecl R_ProcessWorkerCmds();
int __cdecl R_ProcessWorkerCmd(WorkerCmdType type);
void __cdecl R_ProcessWorkerCmdInternal(WorkerCmdType type, void *data);
void R_InitWorkerThreads();
int R_InitWorkerCmds();
int R_InitWorkerCmdsPos();
void __cdecl  R_WorkerThread();
void __cdecl R_AddWorkerCmd(WorkerCmdType type, unsigned __int8 *data);
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
