#include "profile.h"
#include <qcommon/qcommon.h>
#include <qcommon/threads.h>
#include "timing.h"

#include <Windows.h>

ProfileScript profileScript;
int g_profileStack[256];
int prof_parity[2];
ProfileStack g_prof_stack[7];

const dvar_t *profile;
const dvar_t *profile_thread;
const dvar_t *profile_rowcount;

int prof_enumSystems[432] =
{
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
}; // idb



const char *prof_pageNames[36] =
{
  "",
  "self",
  "total",
  "avgself",
  "avgtotal",
  "max",
  "maxself",
  "main",
  "updateScreen",
  "thread",
  "generic",
  "generic2",
  "probes",
  "probe",
  "gprobe",
  "frontend",
  "renderer",
  "renderer2",
  "renderer3",
  "cinematics",
  "phys",
  "shadowcookie",
  "water",
  "snd",
  "xanim",
  "xanim2",
  "script",
  "fx",
  "prediction",
  "trace",
  "cgtrace",
  "server",
  "trigger",
  "dynent",
  "aimassist",
  NULL
}; // idb

const char *g_profile_thread_values[9] =
{
  "main",
  "backend",
  "worker0",
  "worker1",
  "cinematic",
  "titleserver",
  "database",
  "all",
  NULL
}; // idb

void Profile_Init()
{
    const char *v0; // eax
    unsigned int profPageIter; // [esp+14h] [ebp-4h]

    for (profPageIter = 0; s_profileArrays[profPageIter].name; ++profPageIter)
    {
        if (!prof_pageNames[profPageIter + 7])
            MyAssertHandler(
                ".\\universal\\profile.cpp",
                234,
                0,
                "%s\n\t(s_profileArrays[profPageIter].name) = %s",
                "(prof_pageNames[profPageIter + PROFPAGE_SPECIAL_COUNT])",
                s_profileArrays[profPageIter].name);
        if (strcmp(s_profileArrays[profPageIter].name, prof_pageNames[profPageIter + 7]))
        {
            v0 = va("%s != %s", s_profileArrays[profPageIter].name, prof_pageNames[profPageIter + 7]);
            MyAssertHandler(
                ".\\universal\\profile.cpp",
                235,
                0,
                "%s\n\t%s",
                "!strcmp( s_profileArrays[profPageIter].name, prof_pageNames[profPageIter + PROFPAGE_SPECIAL_COUNT] )",
                v0);
        }
    }
    profile = Dvar_RegisterEnum("profile", prof_pageNames, 0, 0, "Type of profiling");
    profile_thread = Dvar_RegisterEnum("profile_thread", g_profile_thread_values, 7, 0, "Thread being profiled");
    profile_rowcount = Dvar_RegisterInt("profile_rowcount", 20, (DvarLimits)0x2800000000LL, 0, "Profile row count");
}

void __cdecl Profile_Unguard(int id)
{
    const char *v1; // eax
    ProfileStack *prof_stack; // [esp+0h] [ebp-4h]

    prof_stack = (ProfileStack *)Sys_GetValue(0);
    if (prof_stack->prof_guardpos <= 0)
        MyAssertHandler(".\\universal\\profile.cpp", 516, 0, "%s", "prof_stack->prof_guardpos > 0");
    if (prof_stack->prof_guardstack[--prof_stack->prof_guardpos].id != id)
        MyAssertHandler(
            ".\\universal\\profile.cpp",
            520,
            0,
            "%s",
            "prof_stack->prof_guardstack[prof_stack->prof_guardpos].id == id");
    if (prof_stack->prof_guardstack[prof_stack->prof_guardpos].ppStack != prof_stack->prof_ppStack)
    {
        v1 = Profile_MissingEnd();
        MyAssertHandler(
            ".\\universal\\profile.cpp",
            521,
            0,
            "%s\n\t(Profile_MissingEnd()) = %s",
            "(prof_stack->prof_guardstack[prof_stack->prof_guardpos].ppStack == prof_stack->prof_ppStack)",
            v1);
    }
}

void __cdecl Profile_SetTotal(int index, int total)
{
    ProfileStack* prof_stack = (ProfileStack*)Sys_GetValue(0);
    prof_stack->prof_array[index].write.total.value[0] = total;
}

void Profile_ResetScriptCounters()
{
    int profileIndex; // [esp+10h] [ebp-4h]

    for (profileIndex = 0; profileIndex < 40; ++profileIndex)
    {
        profileScript.totalTime[profileIndex] = profileScript.totalTime[3 * profileIndex - 118];
        profileScript.avgTime[profileIndex] = (profileScript.totalTime[profileIndex]
            + 4 * profileScript.avgTime[profileIndex])
            / 5;
        if (profileScript.maxTime[profileIndex] < profileScript.totalTime[profileIndex])
            profileScript.maxTime[profileIndex] = profileScript.totalTime[profileIndex];
        profileScript.cumulative[profileIndex] = (double)profileScript.totalTime[profileIndex]
            * *((float *)Sys_GetValue(0) + 20782)
                + (float)profileScript.cumulative[profileIndex];
    }
    memset((unsigned __int8 *)&profileScript, 0, 0x1E0u);
}

void __cdecl Profile_ResetCounters(int system)
{
    ProfileStack *prof_stack; // [esp+30h] [ebp-Ch]
    int profileContext; // [esp+34h] [ebp-8h]
    unsigned int profileStackPos; // [esp+38h] [ebp-4h]

    prof_stack = (ProfileStack *)Sys_GetValue(0);
    profileStackPos = 0;
    while (prof_stack->prof_ppStack != prof_stack->prof_pStack)
    {
        if (profileStackPos >= 0x100)
            MyAssertHandler(
                ".\\universal\\profile.cpp",
                416,
                0,
                "profileStackPos doesn't index ARRAY_COUNT( g_profileStack )\n\t%i not in [0, %i)",
                profileStackPos,
                256);
        g_profileStack[profileStackPos++] = Profile_EndInternal(0);
    }
    for (profileContext = 0; profileContext < 7; ++profileContext)
        Profile_ResetCountersForContext(profileContext, system);
    while (profileStackPos)
        Profile_Begin(g_profileStack[--profileStackPos]);
    ++prof_parity[system];
}

void __cdecl Profile_Recover(int id)
{
    ProfileStack *prof_stack; // [esp+0h] [ebp-4h]

    prof_stack = (ProfileStack *)Sys_GetValue(0);
    if (prof_stack->prof_guardpos <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../universal/profile.h",
            355,
            0,
            "%s",
            "prof_stack->prof_guardpos > 0");
    while (prof_stack->prof_guardpos > 0)
    {
        if (prof_stack->prof_guardstack[--prof_stack->prof_guardpos].id == id)
        {
            while (prof_stack->prof_ppStack != prof_stack->prof_guardstack[prof_stack->prof_guardpos].ppStack)
            {
                if (**(_DWORD **)prof_stack->prof_ppStack >= 3u)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\renderer\\../universal/profile.h",
                        364,
                        0,
                        "(*prof_stack->prof_ppStack)->write.nesting doesn't index ARRAY_COUNT( (*prof_stack->prof_ppStack)->write.sta"
                        "rt )\n"
                        "\t%i not in [0, %i)",
                        **(_DWORD **)prof_stack->prof_ppStack,
                        3);
                --**(_DWORD **)prof_stack->prof_ppStack--;
            }
            return;
        }
    }
    if (!alwaysfails)
        MyAssertHandler("c:\\trees\\cod3\\src\\renderer\\../universal/profile.h", 372, 0, "Profile_Recover: id not found");
}

void __cdecl Profile_InitContext(int profileContext)
{
    ProfileStack *prof_stack; // [esp+50h] [ebp-18h]
    ProfileAtom uiStart; // [esp+54h] [ebp-14h]
    ProfileAtom uiStop; // [esp+58h] [ebp-10h]
    int atomType; // [esp+5Ch] [ebp-Ch]
    ProfileAtom uiTotal; // [esp+60h] [ebp-8h]
    int i; // [esp+64h] [ebp-4h]

    LARGE_INTEGER qpc;

    prof_stack = Profile_GetStackForContext(profileContext);
    Sys_SetValue(0, prof_stack);
    prof_stack->prof_pStack[0] = &prof_stack->prof_root;
    prof_stack->prof_ppStack = prof_stack->prof_pStack;
    prof_stack->prof_overhead_internal.value[0] = 0;
    prof_stack->prof_overhead_external.value[0] = 0;
    prof_stack->prof_timescale = 0.0;
    prof_stack->prof_guardpos = 0;
    Profile_Begin(0);
    Profile_EndInternal(0);
    prof_stack->prof_array[0].write.total.value[0] = 0;
    prof_stack->prof_root.write.total.value[0] = 0;
    prof_stack->prof_root.write.child.value[0] = 0;
    uiTotal.value[0] = 0;
    for (i = 0; i < 1000; ++i)
    {
        QueryPerformanceCounter(&qpc);
        uiStart.value[0] = qpc.QuadPart;
        Profile_Begin(0);
        Profile_EndInternal(0);
        QueryPerformanceCounter(&qpc);
        uiStop.value[0] = qpc.QuadPart;
        for (atomType = 0; atomType < 1; ++atomType)
            uiTotal.value[atomType] += uiStop.value[atomType] - uiStart.value[atomType];
    }
    prof_stack->prof_overhead_internal.value[0] = prof_stack->prof_array[0].write.total.value[0] / 0x3E8;
    prof_stack->prof_overhead_external.value[0] = uiTotal.value[0] / 0x3E8;
    // NOTE(mrsteyk): unused?
    //prof_stack->prof_timescale = ((double)1LL - (double)0LL) * msecPerRawTimerTick;
    prof_stack->prof_timescale = qpc2msec;
    for (i = 0; i < 432; ++i)
        prof_stack->prof_array[i].write.nesting = -1;
}

void __cdecl Profile_Guard(int id)
{
    int i; // [esp+0h] [ebp-8h]
    ProfileStack *prof_stack; // [esp+4h] [ebp-4h]

    prof_stack = (ProfileStack *)Sys_GetValue(0);
    if (prof_stack->prof_guardpos
        && (profile_t **)prof_stack->prof_overhead_external.value[2 * prof_stack->prof_guardpos] > prof_stack->prof_ppStack)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../universal/profile.h",
            331,
            0,
            "%s",
            "prof_stack->prof_guardpos == 0 || prof_stack->prof_guardstack[prof_stack->prof_guardpos - 1].ppStack <= prof_stack->prof_ppStack");
    }
    if (prof_stack->prof_guardpos >= 0x20u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../universal/profile.h",
            332,
            0,
            "prof_stack->prof_guardpos doesn't index ARRAY_COUNT( prof_stack->prof_guardstack )\n\t%i not in [0, %i)",
            prof_stack->prof_guardpos,
            32);
    for (i = 0; i < prof_stack->prof_guardpos; ++i)
    {
        if (prof_stack->prof_guardstack[i].id == id)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../universal/profile.h",
                339,
                0,
                "%s",
                "prof_stack->prof_guardstack[i].id != id");
    }
    prof_stack->prof_guardstack[prof_stack->prof_guardpos].id = id;
    prof_stack->prof_guardstack[prof_stack->prof_guardpos++].ppStack = prof_stack->prof_ppStack;
}

ProfileStack *__cdecl Profile_GetStackForContext(int profileContext)
{
    return &g_prof_stack[profileContext];
}

ProfileScript *__cdecl Profile_GetScript()
{
    return &profileScript;
}

int __cdecl Profile_GetEnumParity(unsigned int profEnum)
{
    if (profEnum >= 0x1B0)
        MyAssertHandler(
            ".\\universal\\profile.cpp",
            474,
            0,
            "profEnum doesn't index ARRAY_COUNT( prof_enumSystems )\n\t%i not in [0, %i)",
            profEnum,
            432);
    return prof_parity[prof_enumSystems[profEnum]];
}

int __cdecl Profile_GetDisplayThread()
{
    if (!profile_thread)
        MyAssertHandler(".\\universal\\profile.cpp", 668, 0, "%s", "profile_thread");
    return profile_thread->current.integer;
}

void __cdecl Profile_EndScripts(unsigned int profileFlags)
{
    int profileIndex; // [esp+10h] [ebp-4h]

    if (!profileFlags)
        MyAssertHandler(".\\universal\\profile.cpp", 593, 0, "%s", "profileFlags");
    profileIndex = 0;
    do
    {
        if ((profileFlags & 1) != 0)
        {
            Profile_EndScript(profileIndex);
        LABEL_10:
            profileFlags >>= 1;
            ++profileIndex;
            continue;
        }
        if ((profileFlags & 0xE) != 0)
            goto LABEL_10;
        profileFlags >>= 4;
        profileIndex += 4;
        if (!(_BYTE)profileFlags)
        {
            profileFlags >>= 8;
            profileIndex += 8;
        }
    } while (profileFlags);
}

void __cdecl Profile_EndScript(int profileIndex)
{
    ProfileScriptWritable *write; // [esp+8h] [ebp-8h]
    unsigned int endTime; // [esp+Ch] [ebp-4h]

    endTime = __rdtsc();
    if (profileIndex >= 40)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\script\\../universal/profile.h",
            316,
            0,
            "%s",
            "profileIndex < PROF_SCRIPT_COUNT");
    write = &profileScript.write[profileIndex];
    if (!--write->refCount)
        write->totalTime += endTime - write->startTime;
}

int __cdecl Profile_EndInternal(long double *duration)
{
    // KISAKTODO: Profiler
    return 0;

    LARGE_INTEGER qpc;

    ProfileAtom end;
    QueryPerformanceCounter(&qpc);
    end.value[0] = qpc.QuadPart;

    ProfileStack* prof_stack = (ProfileStack*)Sys_GetValue(0);
    profile_t* p = *prof_stack->prof_ppStack;

    iassert(p->write.nesting < 3);
    iassert(p->write.nesting >= 0);

    auto overhead = prof_stack->prof_overhead_internal.value[0];
    auto start = p->write.start[p->write.nesting].value[0];
    
    // Self time.
    ProfileAtom deltaa;
    deltaa.value[0] = end.value[0] - overhead - start;
    if ((deltaa.value[0] & 0x80000000) != 0)
        deltaa.value[0] = 0;

    --p->write.nesting;
    p->write.total.value[0] += deltaa.value[0];
    ++p->write.hits;

    // Add self time to total parent's time?
    profile_t* parent = *--prof_stack->prof_ppStack;
    parent->write.child.value[0] += deltaa.value[0];
    // Substract overhead because 2000's
    if (parent->write.total.value[0] < prof_stack->prof_overhead_external.value[0])
        parent->write.total.value[0] = 0;
    else
        parent->write.total.value[0] -= prof_stack->prof_overhead_external.value[0];

    if (duration)
        *duration = deltaa.value[0] * qpc2msec;

    return p - prof_stack->prof_array;
}

void __cdecl Profile_BeginScripts(unsigned int profileFlags)
{
    // KISAKTODO: profiler
    int profileIndex; // [esp+Ch] [ebp-4h]

    if (!profileFlags)
        MyAssertHandler(".\\universal\\profile.cpp", 557, 0, "%s", "profileFlags");
    profileIndex = 0;
    do
    {
        if ((profileFlags & 1) != 0)
        {
            Profile_BeginScript(profileIndex);
        LABEL_10:
            profileFlags >>= 1;
            ++profileIndex;
            continue;
        }
        if ((profileFlags & 0xE) != 0)
            goto LABEL_10;
        profileFlags >>= 4;
        profileIndex += 4;
        if (!(_BYTE)profileFlags)
        {
            profileFlags >>= 8;
            profileIndex += 8;
        }
    } while (profileFlags);
}

void __cdecl Profile_BeginScript(int profileIndex)
{
    // KISAKTODO: profiler
    ProfileScriptWritable* write; // [esp+8h] [ebp-4h]

    if (profileIndex >= 40)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\script\\../universal/profile.h",
            299,
            0,
            "%s",
            "profileIndex < PROF_SCRIPT_COUNT");
    write = &profileScript.write[profileIndex];
    if (!write->refCount)
        write->startTime = __rdtsc();
    ++write->refCount;
}

void __cdecl Profile_Begin(int index)
{
    // KISAKTODO: Profiler

    return;
    LARGE_INTEGER qpc;

    ProfileStack* prof_stack = (ProfileStack*)Sys_GetValue(0);
    profile_t* p = prof_stack->prof_array + index;

    // NOTE(mrsteyk): this will assert when something bad happens either way
    p->write.nesting++;
    iassert(p->write.nesting < 3);

    *++prof_stack->prof_ppStack = p;

    QueryPerformanceCounter(&qpc);
    p->write.start[p->write.nesting].value[0] = qpc.QuadPart;
}

int __cdecl Profile_AddScriptName(char *profileName)
{
    char *name; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 0x28; ++i)
    {
        name = profileScript.profileScriptNames[i];
        if (!I_strnicmp(profileName, name, 19))
            return i;
        if (!*name)
        {
            I_strncpyz(name, profileName, 20);
            profileScript.avgTime[i] = 0;
            profileScript.maxTime[i] = 0;
            profileScript.cumulative[i] = 0.0;
            return i;
        }
    }
    return -1;
}

void __cdecl Profile_ResetCountersForContext(int profileContext, int system)
{
    int atomType; // [esp+8h] [ebp-Ch]
    profile_t *prof_array; // [esp+Ch] [ebp-8h]
    unsigned int i; // [esp+10h] [ebp-4h]

    prof_array = Profile_GetStackForContext(profileContext)->prof_array;
    for (i = 0; i < 0x1B0; ++i)
    {
        if (prof_enumSystems[i] == system)
        {
            prof_array[i].read.hits = prof_array[i].write.hits;
            prof_array[i].read.total.value[0] = prof_array[i].write.total.value[0];
            for (atomType = 0; atomType < 1; ++atomType)
            {
                if (prof_array[i].write.total.value[atomType] < prof_array[i].write.child.value[atomType])
                    prof_array[i].read.self.value[atomType] = 0;
                else
                    prof_array[i].read.self.value[atomType] = prof_array[i].write.total.value[atomType]
                    - prof_array[i].write.child.value[atomType];
            }
            prof_array[i].write.hits = 0;
            prof_array[i].write.total.value[0] = 0;
            prof_array[i].write.child.value[0] = 0;
        }
    }
}

const char *__cdecl Profile_MissingEnd()
{
    ProfileStack *prof_stack; // [esp+0h] [ebp-Ch]
    char *msg; // [esp+4h] [ebp-8h]
    unsigned int profId; // [esp+8h] [ebp-4h]

    prof_stack = (ProfileStack *)Sys_GetValue(0);
    msg = 0;
    for (profId = 0; profId < 0x1B0; ++profId)
    {
        if (prof_stack->prof_array[profId].write.nesting >= 0)
        {
            if (msg)
                msg = va("%s, %s", msg, prof_enumNames[profId]);
            else
                msg = (char *)prof_enumNames[profId];
        }
    }
    if (msg)
        return va("probes missing PROF_END: %s", msg);
    else
        return "no probes missing PROF_END detected";
}