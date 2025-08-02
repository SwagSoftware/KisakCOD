#include "threads.h"

#include <Windows.h>

#include <universal/assertive.h>

#include <client_mp/client_mp.h>
#include <gfx_d3d/rb_drawprofile.h>
#include <gfx_d3d/r_init.h>
#include <win32/win_local.h>

unsigned int Win_InitThreads();


// NOTE(mrsteyk): keep in mind this is 4 elements long.
static thread_local void **g_threadLocals;

//static int threadId[7];
//static HANDLE threadHandle[7];
//static unsigned int s_affinityMaskForProcess;
//static unsigned int s_cpuCount;
//static unsigned int s_affinityMaskForCpu[4];

#ifdef KISAK_SP
int isDoingDatabaseInit;

void *wakeServerEvent;
void *serverCompletedEvent;
void *allowSendClientMessagesEvent;
void *serverSnapshotEvent;
void *clientMessageReceived;
void *g_saveHistoryEvent;
void *g_saveHistoryDoneEvent;

volatile int g_timeout;
#endif

typedef void (*ThreadFuncFn)(unsigned int);
static ThreadFuncFn threadFunc[7];

void *g_threadValues[7][4];
DWORD threadId[7];
HANDLE threadHandle[7];
unsigned int s_affinityMaskForProcess;
unsigned int s_cpuCount;
unsigned int s_affinityMaskForCpu[4];

static int g_databaseThreadOwner;

static volatile PVOID smpData;

static volatile unsigned int renderPausedCount;

static WinThreadLock s_threadLock;

static void *renderPausedEvent;
static void *renderCompletedEvent;
static void *noThreadOwnershipEvent;
static void *rendererRunningEvent;
static void *backendEvent[2];
static void *ackendEvent;
static void *updateSpotLightEffectEvent;
static void *updateEffectsEvent;

static const char* s_threadNames[7] =
{
    "Main",
    "Backend",
    "Worker0",
    "Worker1",
    "Cinematic",
    "Titleserver",
    "Database",
};
unsigned int __cdecl Sys_GetCpuCount()
{
    return s_cpuCount;
}

void __cdecl Sys_EndLoadThreadPriorities()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\threads.cpp", 700, 0, "%s", "Sys_IsMainThread()");
    SetThreadPriority(threadHandle[0], 0);
}

int __cdecl Sys_IsRendererReady()
{
    return Sys_WaitForSingleObjectTimeout(&renderCompletedEvent, 0);
}

void __cdecl Sys_InitMainThread()
{
    HANDLE process; // [esp+0h] [ebp-8h]
    HANDLE pseudoHandle; // [esp+4h] [ebp-4h]

    threadId[0] = Sys_GetCurrentThreadId();
    process = GetCurrentProcess();
    pseudoHandle = GetCurrentThread();
    DuplicateHandle(process, pseudoHandle, process, threadHandle, 0, 0, 2u);
    Win_InitThreads();
    //*(unsigned int*)(*((unsigned int*)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4) = g_threadValues;
    g_threadLocals = g_threadValues[0];
    Com_InitThreadData(0);
}

unsigned int __cdecl Sys_GetCurrentThreadId()
{
    return GetCurrentThreadId();
}

void __cdecl Sys_InitThread(int threadContext)
{
    //*(unsigned int*)(*((unsigned int*)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4) = g_threadValues[threadContext];
    g_threadLocals = g_threadValues[threadContext];
    Com_InitThreadData(threadContext);
    Profile_InitContext(threadContext);
}

char __cdecl Sys_SpawnRenderThread(void(__cdecl* function)(unsigned int))
{
    Sys_CreateEvent(0, 0, &renderPausedEvent);
    Sys_CreateEvent(1, 1, &renderCompletedEvent);
    Sys_CreateEvent(1, 0, &noThreadOwnershipEvent);
    Sys_CreateEvent(1, 1, &rendererRunningEvent);
    Sys_CreateEvent(0, 0, &backendEvent[1]);
    Sys_CreateEvent(1, 0, backendEvent);
    Sys_CreateEvent(1, 1, &updateSpotLightEffectEvent);
    Sys_CreateEvent(1, 1, &updateEffectsEvent);

    Sys_CreateThread(function, 1u);

    if (!threadHandle[1])
        return 0;

    Sys_ResumeThread(1);

    return 1;
}

void __cdecl Sys_CreateEvent(bool manualReset, bool initialState, void** event)
{
    *event = CreateEventA(0, manualReset, initialState, 0);
}

void __cdecl Sys_CreateThread(void(__cdecl* function)(unsigned int), unsigned int threadContext)
{
    if (threadFunc[threadContext])
        MyAssertHandler(".\\qcommon\\threads.cpp", 805, 0, "%s", "threadFunc[threadContext] == NULL");
    if (threadContext >= 7)
        MyAssertHandler(".\\qcommon\\threads.cpp", 806, 0, "%s", "threadContext < THREAD_CONTEXT_COUNT");
    threadFunc[threadContext] = function;
    threadHandle[threadContext] = CreateThread(
        0,
        0,
        (LPTHREAD_START_ROUTINE)Sys_ThreadMain,
        (LPVOID)threadContext,
        4u,
        &threadId[threadContext]);
    SetThreadName(threadId[threadContext], s_threadNames[threadContext]);
}

#define MS_VC_EXCEPTION 0x406d1388
struct tagTHREADNAME_INFO // sizeof=0x10
{                                     
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.          
};

// https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads?view=vs-2022&tabs=csharp
void __cdecl SetThreadName(unsigned int threadId, const char* threadName)
{
    tagTHREADNAME_INFO info; // [esp+10h] [ebp-28h] BYREF
    //CPPEH_RECORD ms_exc; // [esp+20h] [ebp-18h]

    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = threadId;
    info.dwFlags = 0;
    
#ifdef TRACY_ENABLE
    TracyCSetThreadName(threadName);
#endif

    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

unsigned int __stdcall Sys_ThreadMain(int parameter)
{
    if ((unsigned int)parameter >= 7)
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            787,
            0,
            "threadContext doesn't index THREAD_CONTEXT_COUNT\n\t%i not in [0, %i)",
            parameter,
            7);
    if (!threadFunc[parameter])
        MyAssertHandler(".\\qcommon\\threads.cpp", 788, 0, "%s", "threadFunc[threadContext]");
    SetThreadName(0xFFFFFFFF, s_threadNames[parameter]);
    Sys_InitThread(parameter);
    threadFunc[parameter](parameter);
    return 0;
}

static void* wakeDatabaseEvent;
static void* databaseCompletedEvent;
static void* databaseCompletedEvent2;
static void* resumedDatabaseEvent;

bool dediRenderHack = false;

char __cdecl Sys_SpawnDatabaseThread(void(__cdecl* function)(unsigned int))
{
    Sys_CreateEvent(0, 0, &wakeDatabaseEvent);
    Sys_CreateEvent(1, 1, &databaseCompletedEvent);
    Sys_CreateEvent(1, 1, &databaseCompletedEvent2);
    Sys_CreateEvent(1, 1, &resumedDatabaseEvent);

    Sys_CreateThread(function, 6);

    if (!threadHandle[6])
        return 0;

    SetThreadPriority(threadHandle[6], s_cpuCount > 1 ? 1 : -1);
    Sys_ResumeThread(6);

    return 1;
}

void __cdecl Sys_SuspendDatabaseThread(ThreadOwner owner)
{
    if (owner == THREAD_OWNER_NONE)
        MyAssertHandler(".\\qcommon\\threads.cpp", 1060, 0, "%s", "owner != THREAD_OWNER_NONE");

    if (g_databaseThreadOwner)
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1061,
            0,
            "%s\n\t(g_databaseThreadOwner) = %i",
            "(g_databaseThreadOwner == THREAD_OWNER_NONE)",
            g_databaseThreadOwner);

    g_databaseThreadOwner = owner;
    Sys_ResetEvent(&resumedDatabaseEvent);
}

void __cdecl Sys_ResetEvent(void** event)
{
    ResetEvent(*event);
}

void __cdecl Sys_ResumeDatabaseThread(ThreadOwner owner)
{
    if (owner == THREAD_OWNER_NONE)
        MyAssertHandler(".\\qcommon\\threads.cpp", 1072, 0, "%s", "owner != THREAD_OWNER_NONE");
    if (g_databaseThreadOwner != owner)
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1073,
            0,
            "g_databaseThreadOwner == owner\n\t%i, %i",
            g_databaseThreadOwner,
            owner);
    g_databaseThreadOwner = THREAD_OWNER_NONE;
    Sys_SetEvent(&resumedDatabaseEvent);
}

void __cdecl Sys_SetEvent(void** event)
{
    SetEvent(*event);
}

bool __cdecl Sys_HaveSuspendedDatabaseThread(ThreadOwner owner)
{
    return g_databaseThreadOwner == owner;
}

void __cdecl Sys_WaitDatabaseThread()
{
    Sys_WaitForSingleObject(&resumedDatabaseEvent);
}

void __cdecl Sys_WaitForSingleObject(void** event)
{
    unsigned int result; // [esp+0h] [ebp-4h]

    result = WaitForSingleObject(*event, 0xFFFFFFFF);
    iassert(result == ((((unsigned int)0x00000000L)) + 0));
}

bool __cdecl Sys_SpawnWorkerThread(void(__cdecl* function)(unsigned int), unsigned int threadIndex)
{
    unsigned int threadContext; // [esp+0h] [ebp-4h]

    threadContext = threadIndex + 2;
    if (threadHandle[threadIndex + 2])
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1113,
            0,
            "%s\n\t(threadContext) = %i",
            "(!threadHandle[threadContext])",
            threadContext);
    Sys_CreateThread(function, threadContext);
    return threadHandle[threadContext] != 0;
}

void __cdecl Sys_SuspendThread(unsigned int threadContext)
{
    if (!threadHandle[threadContext])
        MyAssertHandler(".\\qcommon\\threads.cpp", 1136, 0, "%s", "threadHandle[threadContext]");
    SuspendThread(threadHandle[threadContext]);
}

void __cdecl Sys_ResumeThread(unsigned int threadContext)
{
    if (!threadHandle[threadContext])
        MyAssertHandler(".\\qcommon\\threads.cpp", 1148, 0, "%s", "threadHandle[threadContext]");
    ResumeThread(threadHandle[threadContext]);
}

void *__cdecl Sys_RendererSleep()
{
    return InterlockedExchangePointer(&smpData, nullptr);
}

int __cdecl Sys_RendererReady()
{
    return smpData != 0;
}

void __cdecl Sys_RenderCompleted()
{
    Sys_SetEvent(&renderCompletedEvent);
    Sys_SetWorkerCmdEvent();
}

void __cdecl Sys_FrontEndSleep()
{
    int newCount; // [esp+0h] [ebp-4h]

    KISAK_NULLSUB();
    if (!Sys_WaitForSingleObjectTimeout(&noThreadOwnershipEvent, 0))
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1206,
            0,
            "%s",
            "Sys_WaitForSingleObjectTimeout( &noThreadOwnershipEvent, 0 )");
    Sys_WaitForSingleObject(&rendererRunningEvent);
    Sys_ResetEvent(&noThreadOwnershipEvent);
    Sys_SetEvent(&backendEvent[1]);
    newCount = InterlockedDecrement(&renderPausedCount);
    if (newCount != -1 && newCount)
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1212,
            0,
            "%s\n\t(newCount) = %i",
            "((newCount == -1) || (newCount == 0))",
            newCount);
    Sys_WaitForSingleObject(&renderPausedEvent);
}

bool __cdecl Sys_WaitForSingleObjectTimeout(void** event, unsigned int msec)
{
    if (msec == -1)
        MyAssertHandler(".\\qcommon\\threads.cpp", 234, 0, "%s", "msec != INFINITE");
    return WaitForSingleObject(*event, msec) == 0;
}

void __cdecl Sys_WakeRenderer(void* data)
{
    Sys_ResetEvent(&renderCompletedEvent);
    const void *old = InterlockedExchangePointer(&smpData, data);
    vassert(!old, "old = %p", old);
    KISAK_NULLSUB();
    Sys_SetEvent(&backendEvent[1]);
    Sys_SetWorkerCmdEvent();
}

void __cdecl Sys_NotifyRenderer()
{
    if (!backendEvent[1])
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1266,
            0,
            "%s",
            "Sys_IsEventInitialized( backendEvent[BACKEND_EVENT_GENERIC] )");
    Sys_SetEvent(&backendEvent[1]);
}

void __cdecl Sys_DatabaseCompleted()
{
#ifdef KISAK_SP 
    Sys_EnterCriticalSection(CRITSECT_START_SERVER);
    isDoingDatabaseInit = 1;
    Sys_LeaveCriticalSection(CRITSECT_START_SERVER);
    Sys_WaitForSingleObject(&serverCompletedEvent);
#endif
    Sys_SetEvent(&databaseCompletedEvent);
}

void __cdecl Sys_WaitStartDatabase()
{
    Sys_WaitForSingleObject(&wakeDatabaseEvent);
}

bool __cdecl Sys_IsDatabaseReady()
{
    return Sys_WaitForSingleObjectTimeout(&databaseCompletedEvent, 0);
}

void __cdecl Sys_SyncDatabase()
{
    Sys_WaitForSingleObject(&databaseCompletedEvent);
}

void __cdecl Sys_WakeDatabase()
{
    Sys_ResetEvent(&databaseCompletedEvent);
}

void __cdecl Sys_NotifyDatabase()
{
    Sys_SetEvent(&wakeDatabaseEvent);
}

void __cdecl Sys_DatabaseCompleted2()
{
#ifdef KISAK_SP
    Sys_EnterCriticalSection(CRITSECT_START_SERVER);
    isDoingDatabaseInit = 0;
    Sys_LeaveCriticalSection(CRITSECT_START_SERVER);
#endif
    Sys_SetEvent(&databaseCompletedEvent2);
}

bool __cdecl Sys_IsDatabaseReady2()
{
    return Sys_WaitForSingleObjectTimeout(&databaseCompletedEvent2, 0);
}

void __cdecl Sys_WakeDatabase2()
{
    Sys_ResetEvent(&databaseCompletedEvent2);
}

bool __cdecl Sys_FinishRenderer()
{
    KISAK_NULLSUB();
    return !Sys_WaitForSingleObjectTimeout(&noThreadOwnershipEvent, 0);
}

int __cdecl Sys_IsMainThreadReady()
{
    return Sys_WaitForSingleObjectTimeout(&noThreadOwnershipEvent, 0);
}

void __cdecl Sys_WaitForMainThread()
{
    Sys_WaitForSingleObject(&noThreadOwnershipEvent);
}

void __cdecl Sys_StopRenderer()
{
    unsigned int newCount; // [esp+0h] [ebp-4h]

    newCount = InterlockedIncrement(&renderPausedCount);
    if (newCount > 1)
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            1734,
            0,
            "%s\n\t(newCount) = %i",
            "((newCount == 0) || (newCount == 1))",
            newCount);
    Sys_ResetEvent(&rendererRunningEvent);
    Sys_SetEvent(&renderPausedEvent);
}

void __cdecl Sys_StartRenderer()
{
    Sys_SetEvent(&rendererRunningEvent);
}

bool __cdecl Sys_IsRenderThread()
{
    return Sys_GetCurrentThreadId() == threadId[1];
}

bool __cdecl Sys_IsDatabaseThread()
{
    return Sys_GetCurrentThreadId() == threadId[6];
}

bool __cdecl Sys_IsMainThread()
{
    return Sys_GetCurrentThreadId() == threadId[0];
}

void __cdecl Sys_SetValue(int valueIndex, void* data)
{
    //*(unsigned int*)(*(unsigned int*)(*((unsigned int*)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4) + 4 * valueIndex) = data;
    g_threadLocals[valueIndex] = data;
}

void* __cdecl Sys_GetValue(int valueIndex)
{
    //return *(void**)(*(unsigned int*)(*((unsigned int*)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4) + 4 * valueIndex);
    return g_threadLocals[valueIndex];
}

void __cdecl Sys_WaitForWorkerCmd()
{
    KISAK_NULLSUB();
    Sys_WaitForSingleObjectTimeout(backendEvent, 1u);
}

void __cdecl Sys_SetWorkerCmdEvent()
{
    Sys_SetEvent(backendEvent);
}

void __cdecl Sys_ResetWorkerCmdEvent()
{
    Sys_ResetEvent(backendEvent);
}

int __cdecl Sys_WaitBackendEvent()
{
    return Sys_WaitForSingleObjectTimeout(&backendEvent[1], 0);
}

void __cdecl Sys_SetUpdateSpotLightEffectEvent()
{
    Sys_SetEvent(&updateSpotLightEffectEvent);
}

void __cdecl Sys_ResetUpdateSpotLightEffectEvent()
{
    Sys_ResetEvent(&updateSpotLightEffectEvent);
}

void __cdecl Sys_WaitUpdateNonDependentEffectsCompleted()
{
    KISAK_NULLSUB();
    Sys_WaitForSingleObject(&updateEffectsEvent);
}

void __cdecl Sys_SetUpdateNonDependentEffectsEvent()
{
    Sys_SetEvent(&updateEffectsEvent);
}

void __cdecl Sys_ResetUpdateNonDependentEffectsEvent()
{
    Sys_ResetEvent(&updateEffectsEvent);
}

void __cdecl Sys_SuspendOtherThreads()
{
    unsigned int threadIndex; // [esp+0h] [ebp-8h]
    unsigned int currentThreadId; // [esp+4h] [ebp-4h]

    currentThreadId = Sys_GetCurrentThreadId();
    for (threadIndex = 0; threadIndex < 7; ++threadIndex)
    {
        if (threadHandle[threadIndex] && threadId[threadIndex] && threadId[threadIndex] != currentThreadId)
            SuspendThread(threadHandle[threadIndex]);
    }
}

void __cdecl Sys_ReleaseThreadOwnership()
{
    if (Sys_WaitForSingleObjectTimeout(&noThreadOwnershipEvent, 0))
        MyAssertHandler(
            ".\\qcommon\\threads.cpp",
            2000,
            0,
            "%s",
            "!Sys_WaitForSingleObjectTimeout( &noThreadOwnershipEvent, 0 )");
    Sys_SetEvent(&noThreadOwnershipEvent);
}

static void* g_cinematicsThreadOutstandingRequestEvent;
static void* g_cinematicsHostOutstandingRequestEvent;


char __cdecl Sys_SpawnCinematicsThread(void(__cdecl* function)(unsigned int))
{
    Sys_CreateEvent(1, 1, &g_cinematicsThreadOutstandingRequestEvent);
    Sys_CreateEvent(1, 0, &g_cinematicsHostOutstandingRequestEvent);
    Sys_CreateThread(function, 4u);
    if (!threadHandle[4])
        return 0;
    Sys_ResumeThread(4);
    return 1;
}

bool __cdecl Sys_WaitForCinematicsThreadOutstandingRequestEventTimeout(unsigned int timeoutMsec)
{
    return Sys_WaitForSingleObjectTimeout(&g_cinematicsThreadOutstandingRequestEvent, timeoutMsec);
}

void __cdecl Sys_SetCinematicsThreadOutstandingRequestEvent()
{
    Sys_SetEvent(&g_cinematicsThreadOutstandingRequestEvent);
}

void __cdecl Sys_ResetCinematicsThreadOutstandingRequestEvent()
{
    Sys_ResetEvent(&g_cinematicsThreadOutstandingRequestEvent);
}

bool __cdecl Sys_WaitForCinematicsHostOutstandingRequestEventTimeout(unsigned int timeoutMsec)
{
    return Sys_WaitForSingleObjectTimeout(&g_cinematicsHostOutstandingRequestEvent, timeoutMsec);
}

void __cdecl Sys_SetCinematicsHostOutstandingRequestEvent()
{
    Sys_SetEvent(&g_cinematicsHostOutstandingRequestEvent);
}

void __cdecl Sys_ResetCinematicsHostOutstandingRequestEvent()
{
    Sys_ResetEvent(&g_cinematicsHostOutstandingRequestEvent);
}

void __cdecl Win_SetThreadLock(WinThreadLock threadLock)
{
    if (s_cpuCount != 1 && threadLock != s_threadLock)
    {
        s_threadLock = threadLock;
        if (s_cpuCount < 2)
            MyAssertHandler(".\\qcommon\\threads.cpp", 2419, 1, "s_cpuCount >= 2\n\t%i, %i", s_cpuCount, 2);
        if (threadLock)
            SetThreadAffinityMask(threadHandle[0], s_affinityMaskForCpu[0]);
        else
            SetThreadAffinityMask(threadHandle[0], s_affinityMaskForProcess);
        if (threadLock)
            SetThreadAffinityMask(threadHandle[1], s_affinityMaskForCpu[1]);
        else
            SetThreadAffinityMask(threadHandle[1], s_affinityMaskForProcess);
        if (threadLock == THREAD_LOCK_ALL)
            SetThreadAffinityMask(threadHandle[6], s_affinityMaskForCpu[2 - (s_cpuCount < 3)]);
        else
            SetThreadAffinityMask(threadHandle[6], s_affinityMaskForProcess);
        if (threadLock == THREAD_LOCK_ALL)
            SetThreadAffinityMask(threadHandle[4], s_affinityMaskForCpu[s_cpuCount - 1]);
        else
            SetThreadAffinityMask(threadHandle[4], s_affinityMaskForProcess);
        if (s_cpuCount >= 3)
        {
            if (threadLock == THREAD_LOCK_ALL)
                SetThreadAffinityMask(threadHandle[2], s_affinityMaskForCpu[2]);
            else
                SetThreadAffinityMask(threadHandle[2], s_affinityMaskForProcess);
        }
        if (s_cpuCount >= 4)
        {
            if (threadLock == THREAD_LOCK_ALL)
                SetThreadAffinityMask(threadHandle[3], s_affinityMaskForCpu[3]);
            else
                SetThreadAffinityMask(threadHandle[3], s_affinityMaskForProcess);
        }
    }
}

WinThreadLock __cdecl Win_GetThreadLock()
{
    return s_threadLock;
}

void Win_UpdateThreadLock()
{
    if (s_cpuCount == 1)
    {
        s_threadLock = THREAD_LOCK_ALL;
    }
    else if (RB_IsUsingAnyProfile())
    {
        Win_SetThreadLock(THREAD_LOCK_ALL);
    }
    else
    {
        WinThreadLock threadLock = (WinThreadLock)sys_lockThreads->current.integer;
        if (threadLock == THREAD_LOCK_NONE && R_IsUsingAdaptiveGpuSync())
            threadLock = THREAD_LOCK_MINIMAL;
        Win_SetThreadLock(threadLock);
    }
}

void __cdecl Sys_BeginLoadThreadPriorities()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\threads.cpp", 715, 0, "%s", "Sys_IsMainThread()");
    SetThreadPriority(threadHandle[0], -1);
}

#ifdef KISAK_SP
int Sys_WaitStartServer(unsigned int timeout)
{
    int v2; // r3
    int v3; // r30

    Sys_EnterCriticalSection(CRITSECT_START_SERVER);
    v2 = Sys_WaitForSingleObjectTimeout(&wakeServerEvent, timeout);
    v3 = v2;
    if (isDoingDatabaseInit)
    {
        v3 = 0;
    }
    else if (v2)
    {
        ResetEvent(serverCompletedEvent);
    }
    Sys_LeaveCriticalSection(CRITSECT_START_SERVER);
    return v3;
}

void Sys_InitServerEvents()
{
    ResetEvent(wakeServerEvent);
    ResetEvent(serverCompletedEvent);
    SetEvent(allowSendClientMessagesEvent);
    ResetEvent(serverSnapshotEvent);
    SetEvent(clientMessageReceived);
    g_timeout = 0;
}

void Sys_ClientMessageReceived()
{
    SetEvent(clientMessageReceived);
}
void Sys_ClearClientMessage()
{
    ResetEvent(clientMessageReceived);
}
int Sys_SpawnServerThread(void(*function)(unsigned int))
{
    int result; // r3

    wakeServerEvent = CreateEventA(0, 1, 0, 0);
    serverCompletedEvent = CreateEventA(0, 1, 0, 0);
    allowSendClientMessagesEvent = CreateEventA(0, 1, 0, 0);
    serverSnapshotEvent = CreateEventA(0, 0, 0, 0);
    clientMessageReceived = CreateEventA(0, 1, 1, 0);
    Sys_CreateThread(function, 5);
    result = (int)threadHandle[5];

    if (threadHandle[5])
    {
        //XSetThreadProcessor(threadHandle[5], 3u);
        Sys_ResumeThread(5);
        return 1;
    }

    return result;
}
void Sys_WaitClientMessageReceived()
{
    unsigned int v0; // r8

    Profile_Begin(424);
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "wait receive msg");
    Sys_WaitForSingleObject(&clientMessageReceived);
    //PIXEndNamedEvent();
    Profile_EndInternal(0);
}
void Sys_ServerSnapshotCompleted()
{
    SetEvent(serverSnapshotEvent);
}
bool Sys_WaitServerSnapshot()
{
    bool v0; // r31

    Profile_Begin(423);
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "wait snapshot");
    v0 = WaitForSingleObject(serverSnapshotEvent, 1) == 0;
    //PIXEndNamedEvent();
    Profile_EndInternal(0);
    return v0;
}
void Sys_AllowSendClientMessages()
{
    SetEvent(allowSendClientMessagesEvent);
}
void Sys_DisallowSendClientMessages()
{
    ResetEvent(allowSendClientMessagesEvent);
}
int Sys_CanSendClientMessages()
{
    return WaitForSingleObject(allowSendClientMessagesEvent, 0) == 0;
}
void Sys_ServerCompleted()
{
    SetEvent(serverCompletedEvent);
}
int Sys_ServerTimeout()
{
    int time = g_timeout;

    if (!time)
    {
        return 1;
    }

    int timeMS = Sys_Milliseconds();
    if (timeMS - g_timeout >= 0)
    {
        int nextTimeout = timeMS - (int)(-50.0f / com_timescaleValue);

        // shitty atomic looping that is emulated from XBox360
        while (true)
        {
            int current = g_timeout;
            if (current != time)
                break;

            // linux spergs, use: __sync_bool_compare_and_swap()
            int oldVal = InterlockedCompareExchange((volatile unsigned int*)&g_timeout, nextTimeout, current);
            if (oldVal == current)
            {
                return 1;
            }
        }

        // timeout modified by someone else
        return 1;
    }


    return 0;
}
void Sys_WakeServer()
{
    SetEvent(wakeServerEvent);
}
bool Sys_WaitServer()
{
    bool v0; // r31

    Profile_Begin(422);
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "wait server");
    v0 = WaitForSingleObject(serverCompletedEvent, 1u) == 0;
    //PIXEndNamedEvent();
    Profile_EndInternal(0);
    return v0;
}
void Sys_SleepServer()
{
    bool v0; // r30

    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "sleep server");
    v0 = WaitForSingleObject(wakeServerEvent, 0) == 0;
    //PIXEndNamedEvent();
    if (v0)
    {
        Sys_EnterCriticalSection(CRITSECT_START_SERVER);
        ResetEvent(wakeServerEvent);
        Sys_LeaveCriticalSection(CRITSECT_START_SERVER);
    }
}
void Sys_Sleep(unsigned int msec)
{
    Sleep(msec);
}
void Sys_SetServerTimeout(int timeout)
{
    int timeMS; // r3

    iassert(timeout >= 0);
    iassert(com_timescaleValue);

    if (timeout)
    {
        //a12 = (int)(float)((float)__SPAIR64__(&a12, timeout) / (float)v13);
        int val = (int)((float)timeout / com_timescaleValue);
        timeMS = Sys_Milliseconds();
        if (g_timeout && timeMS - g_timeout < 0 && g_timeout - (timeMS + val) <= 0)
        {
            //PIXSetMarker(0xFFFFFFFF, "ignore server timeout: %d", a12);
        }
        else
        {
            g_timeout = timeMS + val;
            //PIXSetMarker(0xFFFFFFFF, "server timeout: %d", a12);
        }
    }
    else
    {
        g_timeout = 0;
        //PIXSetMarker(0xFFFFFFFF, "server timeout");
    }
}

bool Sys_WaitForSaveHistoryDone()
{
    return WaitForSingleObject(g_saveHistoryDoneEvent, 0x7D0u) == 0;
}

int Sys_SpawnServerDemoThread(void(*function)(unsigned int))
{
    int result; // r3

    g_saveHistoryEvent = CreateEventA(0, 0, 0, 0);
    g_saveHistoryDoneEvent = CreateEventA(0, 0, 0, 0);
    Sys_CreateThread(function, 0xBu);
    result = (int)threadHandle[11];
    if (threadHandle[11])
    {
        //XSetThreadProcessor(threadHandle[11], 2u);
        Sys_ResumeThread(0xBu);
        return 1;
    }
    return result;
}

void Sys_SetSaveHistoryEvent()
{
    SetEvent(g_saveHistoryEvent);
}
#endif