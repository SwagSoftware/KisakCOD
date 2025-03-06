#include "../win32/win_local.h"

#include "../universal/assertive.h"

_RTL_CRITICAL_SECTION s_criticalSections[CRITSECT_COUNT];

void Sys_InitializeCriticalSections()
{
	for (int critSect = 0; critSect < CRITSECT_COUNT; critSect++) {
		InitializeCriticalSection(&s_criticalSections[critSect]);
	}
}

void Sys_EnterCriticalSection(int critSect)
{
	iassert(critSect >= 0 && critSect < CRITSECT_COUNT);
	EnterCriticalSection(&s_criticalSections[critSect]);
}

void Sys_LeaveCriticalSection(int critSect)
{
	iassert(critSect >= 0 && critSect < CRITSECT_COUNT);
	LeaveCriticalSection(&s_criticalSections[critSect]);
}

static int threadId[7];
static HANDLE threadHandle[7];

static unsigned int s_affinityMaskForProcess;
static unsigned int s_cpuCount;
static unsigned int s_affinityMaskForCpu[4];

static DWORD Sys_GetCurrentThreadId()
{
	return GetCurrentThreadId();
}

static unsigned int Win_InitThreads()
{
    HANDLE CurrentProcess;
    unsigned int result; 
    unsigned int cpuCount; 
    unsigned int systemAffinityMask; 
    unsigned int cpuOffset; 
    unsigned int threadAffinityMask; 
    unsigned int affinityMaskBits[33];
    unsigned int processAffinityMask; 

    CurrentProcess = GetCurrentProcess();
    result = GetProcessAffinityMask(CurrentProcess, &processAffinityMask, &systemAffinityMask);
    s_affinityMaskForProcess = processAffinityMask;
    cpuCount = 0;
    for (threadAffinityMask = 1; (processAffinityMask & -threadAffinityMask) != 0; threadAffinityMask *= 2)
    {
        if ((processAffinityMask & threadAffinityMask) != 0)
        {
            result = cpuCount;
            affinityMaskBits[cpuCount++] = threadAffinityMask;
            if (cpuCount == 32)
                break;
        }
        result = 2 * threadAffinityMask;
    }
    if (cpuCount > 1)
    {
        s_cpuCount = cpuCount;
        s_affinityMaskForCpu[0] = affinityMaskBits[0];
        result = affinityMaskBits[cpuCount - 1];
        s_affinityMaskForCpu[1] = result;
        if (cpuCount != 2)
        {
            if (cpuCount == 3)
            {
                s_affinityMaskForCpu[2] = affinityMaskBits[1];
            }
            else if (cpuCount == 4)
            {
                s_affinityMaskForCpu[2] = affinityMaskBits[1];
                result = affinityMaskBits[2];
                s_affinityMaskForCpu[3] = affinityMaskBits[2];
            }
            else
            {
                cpuOffset = (cpuCount - 2) / 3;
                iassert(1 + cpuOffset < (cpuCount - 1) - cpuOffset);
                s_affinityMaskForCpu[2] = affinityMaskBits[cpuOffset + 1];
                result = affinityMaskBits[cpuCount - 1 - cpuOffset];
                s_affinityMaskForCpu[3] = result;
                s_cpuCount = 4;
            }
        }
    }
    else
    {
        s_cpuCount = 1;
        s_affinityMaskForCpu[0] = -1;
    }
    return result;
}

// *(_DWORD *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4)
static void* g_threadValues[7][4];
static int g_com_error[7][16];

struct va_info_t
{                       
    char va_string[2][1024];
    int index;
};
static va_info_t va_info[7];

struct TraceCheckCount
{                                    
    int global;                      
    int *partitions;                 
};
struct TraceThreadInfo
{                                       
    TraceCheckCount checkcount;         
    struct cbrush_t *box_brush;                
    struct cmodel_t *box_model;                
};

static TraceThreadInfo g_traceThreadInfo[7];

static void Sys_SetValue(int valueIndex, void* data)
{
    g_threadValues[valueIndex][0] = data;
}

static void Com_InitThreadData(int threadContext)
{
    Sys_SetValue(1, &va_info[threadContext]);
    Sys_SetValue(2, g_com_error[threadContext]);
    Sys_SetValue(3, &g_traceThreadInfo[threadContext]);
}

void Sys_InitMainThread()
{
	threadId[0] = Sys_GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &threadHandle[0], 0, 0, 2);
	Win_InitThreads();
    Com_InitThreadData(0);
}