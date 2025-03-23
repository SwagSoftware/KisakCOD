#include "../win32/win_local.h"

#include "../universal/assertive.h"
#include <qcommon/qcommon.h>
#include <qcommon/threads.h>
#include <direct.h>



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

void Sys_LockWrite(FastCriticalSection* critSect)
{
    while (1)
    {
        if (critSect->readCount == 0)
        {
            if (InterlockedIncrement(&critSect->writeCount) == 1 && critSect->readCount == 0)
            {
                break;
            }
            InterlockedDecrement(&critSect->writeCount);
        }
        NET_Sleep(0);
    }
}

void Sys_UnlockWrite(FastCriticalSection* critSect)
{
    iassert(critSect->writeCount > 0);
    InterlockedDecrement(&critSect->writeCount);
}

static int threadId[7];
static HANDLE threadHandle[7];

static unsigned int s_affinityMaskForProcess;
static unsigned int s_cpuCount;
static unsigned int s_affinityMaskForCpu[4];

unsigned int Win_InitThreads()
{
    HANDLE CurrentProcess;
    unsigned long result; 
    unsigned long cpuCount; 
    DWORD_PTR systemAffinityMask;
    unsigned long cpuOffset; 
    unsigned long threadAffinityMask; 
    unsigned long affinityMaskBits[33];
    DWORD_PTR processAffinityMask; 

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

static void Sys_SetValue(int valueIndex, void* data)
{
    g_threadValues[valueIndex][0] = data;
}

void Com_InitThreadData(int threadContext)
{
    Sys_SetValue(1, &va_info[threadContext]);
    Sys_SetValue(2, g_com_error[threadContext]);
    Sys_SetValue(3, &g_traceThreadInfo[threadContext]);
}

void __cdecl Sys_Mkdir(const char *path)
{
    _mkdir(path);
}

char cwd[256];
char *__cdecl Sys_Cwd()
{
    _getcwd(cwd, 255);
    cwd[255] = 0;
    return cwd;
}

const char *__cdecl Sys_DefaultCDPath()
{
    return "";
}

char exePath[256];
char *__cdecl Sys_DefaultInstallPath()
{
    char *v0; // eax
    unsigned int len; // [esp+0h] [ebp-8h]
    HINSTANCE__ *hinst; // [esp+4h] [ebp-4h]

    if (!exePath[0])
    {
        if (IsDebuggerPresent())
        {
            v0 = Sys_Cwd();
            I_strncpyz(exePath, v0, 256);
        }
        else
        {
            hinst = GetModuleHandleA(0);
            len = GetModuleFileNameA(hinst, exePath, 0x100u);
            if (len == 256)
                len = 255;
            while (len && exePath[len] != 92 && exePath[len] != 47 && exePath[len] != 58)
                --len;
            exePath[len] = 0;
        }
    }
    return exePath;
}