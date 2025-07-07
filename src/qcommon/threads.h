#pragma once

#include <Windows.h> // literally just for some of the extern types at the bottom

enum ThreadOwner : __int32
{                                       // ...
    THREAD_OWNER_NONE = 0x0,
    THREAD_OWNER_DATABASE = 0x1,
    THREAD_OWNER_CINEMATICS = 0x2,
};

unsigned int __cdecl Sys_GetCpuCount();
void __cdecl Sys_InitMainThread();
unsigned int __cdecl Sys_GetCurrentThreadId();
void __cdecl Sys_InitThread(int threadContext);
char __cdecl Sys_SpawnRenderThread(void(__cdecl* function)(unsigned int));
void __cdecl Sys_CreateEvent(bool manualReset, bool initialState, void** event);
void __cdecl Sys_CreateThread(void(__cdecl* function)(unsigned int), unsigned int threadContext);
void __cdecl SetThreadName(unsigned int threadId, const char* threadName);
unsigned int __stdcall Sys_ThreadMain(int parameter);
char __cdecl Sys_SpawnDatabaseThread(void(__cdecl* function)(unsigned int));
void __cdecl Sys_SuspendDatabaseThread(ThreadOwner owner);
void __cdecl Sys_ResetEvent(void** event);
void __cdecl Sys_ResumeDatabaseThread(ThreadOwner owner);
void __cdecl Sys_SetEvent(void** event);
bool __cdecl Sys_HaveSuspendedDatabaseThread(ThreadOwner owner);
void __cdecl Sys_WaitDatabaseThread();
void __cdecl Sys_WaitForSingleObject(void** event);
bool __cdecl Sys_SpawnWorkerThread(void(__cdecl* function)(unsigned int), unsigned int threadIndex);
void __cdecl Sys_SuspendThread(unsigned int threadContext);
void __cdecl Sys_ResumeThread(unsigned int threadContext);
void *__cdecl Sys_RendererSleep();
int __cdecl Sys_RendererReady();
void __cdecl Sys_RenderCompleted();
void __cdecl Sys_FrontEndSleep();
bool __cdecl Sys_WaitForSingleObjectTimeout(void** event, unsigned int msec);
void __cdecl Sys_WakeRenderer(void* data);
void __cdecl Sys_NotifyRenderer();
void __cdecl Sys_DatabaseCompleted();
void __cdecl Sys_WaitStartDatabase();
bool __cdecl Sys_IsDatabaseReady();
void __cdecl Sys_SyncDatabase();
void __cdecl Sys_WakeDatabase();
void __cdecl Sys_NotifyDatabase();
void __cdecl Sys_DatabaseCompleted2();
bool __cdecl Sys_IsDatabaseReady2();
void __cdecl Sys_WakeDatabase2();
bool __cdecl Sys_FinishRenderer();
int __cdecl Sys_IsMainThreadReady();
void __cdecl Sys_EndLoadThreadPriorities();
void __cdecl Sys_WaitForMainThread();
void __cdecl Sys_StopRenderer();
void __cdecl Sys_StartRenderer();
bool __cdecl Sys_IsRenderThread();
bool __cdecl Sys_IsDatabaseThread();
bool __cdecl Sys_IsMainThread();
void __cdecl Sys_SetValue(int valueIndex, void* data);
void* __cdecl Sys_GetValue(int valueIndex);
void __cdecl Sys_WaitForWorkerCmd();
void __cdecl Sys_SetWorkerCmdEvent();
void __cdecl Sys_ResetWorkerCmdEvent();
int __cdecl Sys_WaitBackendEvent();
void __cdecl Sys_SetUpdateSpotLightEffectEvent();
void __cdecl Sys_ResetUpdateSpotLightEffectEvent();
void __cdecl Sys_WaitUpdateNonDependentEffectsCompleted();
void __cdecl Sys_SetUpdateNonDependentEffectsEvent();
void __cdecl Sys_ResetUpdateNonDependentEffectsEvent();
void __cdecl Sys_SuspendOtherThreads();
void __cdecl Sys_ReleaseThreadOwnership();
char __cdecl Sys_SpawnCinematicsThread(void(__cdecl* function)(unsigned int));
bool __cdecl Sys_WaitForCinematicsThreadOutstandingRequestEventTimeout(unsigned int timeoutMsec);
void __cdecl Sys_SetCinematicsThreadOutstandingRequestEvent();
void __cdecl Sys_ResetCinematicsThreadOutstandingRequestEvent();
bool __cdecl Sys_WaitForCinematicsHostOutstandingRequestEventTimeout(unsigned int timeoutMsec);
void __cdecl Sys_SetCinematicsHostOutstandingRequestEvent();
void __cdecl Sys_ResetCinematicsHostOutstandingRequestEvent();

int __cdecl Sys_IsRendererReady();
void __cdecl Sys_BeginLoadThreadPriorities();

#ifdef KISAK_SP
int Sys_WaitStartServer(unsigned int timeout);
void Sys_InitServerEvents();
void Sys_ClientMessageReceived();
void Sys_ClearClientMessage();
int Sys_SpawnServerThread(void(*function)(unsigned int));
void Sys_WaitClientMessageReceived();
void Sys_ServerSnapshotCompleted();
bool Sys_WaitServerSnapshot();
void Sys_AllowSendClientMessages();
void Sys_DisallowSendClientMessages();
int Sys_CanSendClientMessages();
void Sys_ServerCompleted();
int Sys_ServerTimeout();
void Sys_WakeServer();
void Sys_SleepServer();
bool Sys_WaitServer();
void Sys_Sleep(unsigned int msec);
void Sys_SetServerTimeout(int timeout);
#endif


enum WinThreadLock : __int32
{                                       // ...
    THREAD_LOCK_NONE = 0x0,
    THREAD_LOCK_MINIMAL = 0x1,
    THREAD_LOCK_ALL = 0x2,
};
void __cdecl Win_SetThreadLock(WinThreadLock threadLock);
WinThreadLock __cdecl Win_GetThreadLock();
void Win_UpdateThreadLock();


extern void *g_threadValues[7][4];
extern DWORD threadId[7];
extern HANDLE threadHandle[7];
extern unsigned int s_affinityMaskForProcess;
extern unsigned int s_cpuCount;
extern unsigned int s_affinityMaskForCpu[4];