#include "timing.h"

#include <Windows.h>
#include <qcommon/threads.h>

long double msecPerRawTimerTick;

double __cdecl SecondsPerTick()
{
    _LARGE_INTEGER tscStop; // [esp+20h] [ebp-30h]
    _LARGE_INTEGER qpcFrequency; // [esp+28h] [ebp-28h] BYREF
    _LARGE_INTEGER qpcStart; // [esp+30h] [ebp-20h] BYREF
    _LARGE_INTEGER tscStart; // [esp+38h] [ebp-18h]
    _LARGE_INTEGER qpcStop; // [esp+40h] [ebp-10h] BYREF
    double secPerTick; // [esp+48h] [ebp-8h]

    Win_SetThreadLock(THREAD_LOCK_ALL);
    Sleep(0);
    tscStart.QuadPart = 0;
    qpcStart.QuadPart = 0;
    qpcStop.QuadPart = 0;
    QueryPerformanceFrequency(&qpcFrequency);
    QueryPerformanceCounter(&qpcStart);
    tscStart.QuadPart = __rdtsc();
    QueryPerformanceCounter(&qpcStart);
    Sleep(0xFAu);
    tscStop.QuadPart = __rdtsc();
    QueryPerformanceCounter(&qpcStop);
    secPerTick = (double)(qpcStop.QuadPart - qpcStart.QuadPart)
        / ((double)(tscStop.QuadPart - tscStart.QuadPart)
            * (double)qpcFrequency.QuadPart);
    Win_SetThreadLock(THREAD_LOCK_NONE);
    return secPerTick;
}

void __cdecl InitTiming()
{
	msecPerRawTimerTick = SecondsPerTick() * 1000.0;
}