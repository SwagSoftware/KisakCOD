#include <d3d9.h>

#include <qcommon/qcommon.h>

#include "win_local.h"
#include <universal/timing.h>

void Sys_DetectVideoCard(int descLimit, char* description)
{
    _D3DADAPTER_IDENTIFIER9 id;
    vassert(descLimit  == sizeof(id.Description), "descLimit = %d", descLimit);

    strcpy(description, "Unknown video card");
    IDirect3D9 *d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d9)
    {
        if (d3d9->GetAdapterIdentifier(0, 0, &id) >= 0)
            strcpy_s(description, descLimit - 1, id.Description);
        d3d9->Release();
    }
}

unsigned int __cdecl Sys_AddApicIdIfUnique(
    unsigned int apicId,
    unsigned int *existingApicId,
    unsigned int existingCount)
{
    unsigned int existingIter; // [esp+0h] [ebp-4h]

    for (existingIter = 0; existingIter < existingCount; ++existingIter)
    {
        if (existingIter)
            MyAssertHandler(
                ".\\win32\\win_configure.cpp",
                261,
                0,
                "existingIter doesn't index ARRAY_COUNT( existingApicId )\n\t%i not in [0, %i)",
                existingIter,
                1);
        if (existingApicId[existingIter] == apicId)
            return existingCount;
    }
    existingApicId[existingCount] = apicId;
    return existingCount + 1;
}

void __cdecl Sys_GetPhysicalCpuCount(SysInfo* sysInfo)
{
    bool v1; // cf
    DWORD v8; // eax
    _DWORD v14[5]; // [esp+0h] [ebp-E8h] BYREF
    int v15; // [esp+14h] [ebp-D4h]
    unsigned __int8 v16; // [esp+1Ah] [ebp-CEh]
    unsigned __int8 v17; // [esp+1Bh] [ebp-CDh]
    char* cpuVendor; // [esp+1Ch] [ebp-CCh]
    const char* v19; // [esp+20h] [ebp-C8h]
    int v20; // [esp+28h] [ebp-C0h]
    int v21; // [esp+2Ch] [ebp-BCh]
    unsigned int regEdx; // [esp+30h] [ebp-B8h]
    unsigned int regEax; // [esp+34h] [ebp-B4h]
    unsigned int regEbx; // [esp+38h] [ebp-B0h]
    void* process; // [esp+3Ch] [ebp-ACh]
    unsigned int apicId; // [esp+40h] [ebp-A8h]
    unsigned int logicalPerPhysical; // [esp+44h] [ebp-A4h]
    DWORD_PTR systemAffinityMask; // [esp+48h] [ebp-A0h] BYREF
    unsigned int logicalIdMask; // [esp+4Ch] [ebp-9Ch]
    DWORD_PTR processAffinityMask; // [esp+50h] [ebp-98h] BYREF
    unsigned int testAffinityMask; // [esp+54h] [ebp-94h]
    unsigned int existingApicId[32]; // [esp+58h] [ebp-90h] BYREF
    _DWORD* v33; // [esp+D8h] [ebp-10h]
    int v34; // [esp+E4h] [ebp-4h]

    v33 = v14;
    sysInfo->physicalCpuCount = sysInfo->logicalCpuCount;
    v19 = "GenuineIntel";
    cpuVendor = sysInfo->cpuVendor;
    while (1)
    {
        v17 = *cpuVendor;
        v1 = v17 < (unsigned int)*v19;
        if (v17 != *v19)
            break;
        if (!v17)
            goto LABEL_6;
        v16 = cpuVendor[1];
        v1 = v16 < (unsigned int)v19[1];
        if (v16 != v19[1])
            break;
        cpuVendor += 2;
        v19 += 2;
        if (!v16)
        {
        LABEL_6:
            v15 = 0;
            goto LABEL_8;
        }
    }
    v15 = -v1 - (v1 - 1);
LABEL_8:
    v14[4] = v15;
    if (!v15)
    {
        int cpuinfo[4]{};

        // TODO: ecx??
        __cpuidex(cpuinfo, 1, 0);
        regEax = cpuinfo[0];
        regEbx = cpuinfo[1];
        regEdx = cpuinfo[2];

        v34 = -1;
        if ((regEax & 0xF00) == 0xF00 || (regEax & 0xF00000) != 0)
        {
            logicalPerPhysical = BYTE2(regEbx);
            if (BYTE2(regEbx))
            {
                if (logicalPerPhysical != 255)
                {
                    if (!_BitScanReverse(&v8, logicalPerPhysical))
                        v8 = 63;// `CountLeadingZeros'::`2': : notFound;
                    v21 = v8 ^ 0x1F;
                    v20 = 1 << (32 - (v8 ^ 0x1F));
                    logicalIdMask = v20 - 1;
                    sysInfo->physicalCpuCount = 0;
                    process = GetCurrentProcess();
                    GetProcessAffinityMask(process, &processAffinityMask, &systemAffinityMask);
                    if (!processAffinityMask)
                        MyAssertHandler(".\\win32\\win_configure.cpp", 320, 0, "%s", "processAffinityMask");
                    if (!systemAffinityMask)
                        MyAssertHandler(".\\win32\\win_configure.cpp", 321, 0, "%s", "systemAffinityMask");
                    if ((processAffinityMask & ~systemAffinityMask) != 0)
                        MyAssertHandler(".\\win32\\win_configure.cpp", 322, 0, "%s", "!(processAffinityMask & ~systemAffinityMask)");
                    for (testAffinityMask = 1; testAffinityMask && testAffinityMask <= processAffinityMask; testAffinityMask *= 2)
                    {
                        if (SetProcessAffinityMask(process, testAffinityMask))
                        {
                            Sleep(0);

                            // TODO: ecx??
                            __cpuidex(cpuinfo, 1, 0);
                            regEbx = cpuinfo[1];

                            apicId = ~logicalIdMask & HIBYTE(regEbx);
                        }
                        else
                        {
                            apicId = -1;
                        }
                        sysInfo->physicalCpuCount = Sys_AddApicIdIfUnique(apicId, existingApicId, sysInfo->physicalCpuCount);
                    }
                    SetProcessAffinityMask(process, processAffinityMask);
                    Sleep(0);
                    if (sysInfo->physicalCpuCount < 1 || sysInfo->physicalCpuCount > sysInfo->logicalCpuCount)
                        MyAssertHandler(
                            ".\\win32\\win_configure.cpp",
                            344,
                            0,
                            "sysInfo->physicalCpuCount not in [1, sysInfo->logicalCpuCount]\n\t%i not in [%i, %i]",
                            sysInfo->physicalCpuCount,
                            1,
                            sysInfo->logicalCpuCount);
                }
            }
        }
    }
}

long double __cdecl Sys_BenchmarkGHz()
{
    unsigned int i; // ecx
    unsigned __int64 v1; // kr00_8
    int holdrand; // [esp+10h] [ebp-68h]
    float k; // [esp+2Ch] [ebp-4Ch]
    uint64 start; // [esp+30h] [ebp-48h]
    int priority; // [esp+44h] [ebp-34h]
    unsigned __int64 minTime; // [esp+48h] [ebp-30h]
    unsigned int attempt; // [esp+54h] [ebp-24h]
    float x; // [esp+68h] [ebp-10h]
    float xa; // [esp+68h] [ebp-10h]
    float y; // [esp+6Ch] [ebp-Ch]
    float ya; // [esp+6Ch] [ebp-Ch]
    HANDLE thread; // [esp+70h] [ebp-8h]

    k = 2.5999999f;
    thread = GetCurrentThread();
    priority = GetThreadPriority(thread);
    SetThreadPriority(thread, 15);
    minTime = -1;
    for (attempt = 0; attempt < 0x3E8; ++attempt)
    {
        Sleep(0);
        start = __rdtsc();
        holdrand = 0;
        x = 0.25;
        y = 0.75;
        for (i = 0; i < 0x3E8; ++i)
        {
            xa = (1.0 - x) * x * k + x;
            ya = (1.0 - y) * y * k + y;
            x = (1.0 - xa) * xa * k + xa;
            y = (1.0 - ya) * ya * k + ya;
            if ((i & 1) != 0)
                holdrand = 0x343FD * (0x343FD * (0x343FD * holdrand + 0x269EC3) + 0x269EC3) + 0x269EC3;
        }
        v1 = __rdtsc() - start;
        if (minTime > v1)
            minTime = v1;
    }
    SetThreadPriority(thread, priority);
    return 0.1010328 / ((double)minTime * msecPerRawTimerTick);
}

void Sys_SetAutoConfigureGHz(SysInfo* sysInfo)
{
    double multiCpuFactor;

    Sys_GetPhysicalCpuCount(sysInfo);
    vassert(sysInfo->physicalCpuCount >= 1, "sysInfo->physicalCpuCount = %d\n", sysInfo->physicalCpuCount);

    if (sysInfo->physicalCpuCount == 1)
    {
        multiCpuFactor = 1.0;
    }
    else if (sysInfo->physicalCpuCount == 2)
    {
        multiCpuFactor = 1.75;
    }
    else
    {
        multiCpuFactor = 2.0;
    }

    sysInfo->configureGHz = Sys_BenchmarkGHz() * multiCpuFactor;
}

void __cdecl Sys_DetectCpuVendorAndName(char *vendor, char *name)
{
    strcpy(vendor, "KisakTech");
    strcpy(name, "KSK-2900x 5.0khz (No AMD PSP or Intel ME)");
}
#if 0 // KISAKTODO gotta do this ourselves
void __cdecl Sys_DetectCpuVendorAndName(char* vendor, char* name)
{
    int _EAX; // eax
    unsigned int _EDX; // edx
    unsigned int _ECX; // ecx
    unsigned int _EBX; // ebx
    unsigned int _EAX; // eax
    unsigned int _EAX; // eax
    bool v12; // cf
    bool v13; // cf
    int _EAX; // eax
    unsigned int _EAX; // eax
    unsigned int _EDX; // edx
    unsigned int _ECX; // ecx
    unsigned int _EBX; // ebx
    int _EAX; // eax
    unsigned int _EAX; // eax
    unsigned int _EDX; // edx
    unsigned int _ECX; // ecx
    unsigned int _EBX; // ebx
    int _EAX; // eax
    unsigned int _EAX; // eax
    unsigned int _EDX; // edx
    unsigned int _ECX; // ecx
    unsigned int _EBX; // ebx
    _DWORD v29[5]; // [esp+0h] [ebp-90h] BYREF
    int v30; // [esp+14h] [ebp-7Ch]
    unsigned __int8 v31; // [esp+1Ah] [ebp-76h]
    unsigned __int8 v32; // [esp+1Bh] [ebp-75h]
    char* v33; // [esp+1Ch] [ebp-74h]
    const char* v34; // [esp+20h] [ebp-70h]
    int v35; // [esp+24h] [ebp-6Ch]
    int v36; // [esp+28h] [ebp-68h]
    unsigned __int8 v37; // [esp+2Eh] [ebp-62h]
    unsigned __int8 v38; // [esp+2Fh] [ebp-61h]
    char* v39; // [esp+30h] [ebp-60h]
    const char* v40; // [esp+34h] [ebp-5Ch]
    Sys_DetectCpuVendorAndName::__l2::<unnamed_type_cpuid_desc> cpuid_desc; // [esp+38h] [ebp-58h] BYREF
    Sys_DetectCpuVendorAndName::__l2::<unnamed_type_cpuid_vendor> cpuid_vendor; // [esp+6Ch] [ebp-24h] BYREF
    unsigned int maxCpuidArg; // [esp+7Ch] [ebp-14h]
    _DWORD* v44; // [esp+80h] [ebp-10h]
    int v45; // [esp+8Ch] [ebp-4h]

    v44 = v29;
    v45 = 0;
    __asm { pushaw }
    _EAX = 0;
    __asm { cpuid }
    cpuid_vendor.reg.ebx = _EBX;
    *(_QWORD*)&cpuid_vendor.name[4] = __PAIR64__(_ECX, _EDX);
    _EAX = 0x80000000;
    __asm { cpuid }
    maxCpuidArg = _EAX;
    __asm { popaw }
    Sys_CopyCpuidString(vendor, (const char*)&cpuid_vendor, 0xCu);
    if (maxCpuidArg >= 0x80000004)
    {
        __asm { pushaw }
        _EAX = 0x80000002;
        __asm { cpuid }
        *(_QWORD*)&cpuid_desc.s.reg0.eax = __PAIR64__(_EBX, _EAX);
        *(_QWORD*)&cpuid_desc.string[8] = __PAIR64__(_EDX, _ECX);
        _EAX = 0x80000003;
        __asm { cpuid }
        *(_QWORD*)&cpuid_desc.string[16] = __PAIR64__(_EBX, _EAX);
        *(_QWORD*)&cpuid_desc.string[24] = __PAIR64__(_EDX, _ECX);
        _EAX = 0x80000004;
        __asm { cpuid }
        *(_QWORD*)&cpuid_desc.string[32] = __PAIR64__(_EBX, _EAX);
        *(_QWORD*)&cpuid_desc.string[40] = __PAIR64__(_EDX, _ECX);
        __asm { popaw }
        Sys_CopyCpuidString(name, (const char*)&cpuid_desc, 0x30u);
    }
    else
    {
        v40 = "GenuineIntel";
        v39 = vendor;
        while (1)
        {
            v38 = *v39;
            v12 = v38 < (unsigned int)*v40;
            if (v38 != *v40)
                break;
            if (!v38)
                goto LABEL_7;
            v37 = v39[1];
            v12 = v37 < (unsigned int)v40[1];
            if (v37 != v40[1])
                break;
            v39 += 2;
            v40 += 2;
            if (!v37)
            {
            LABEL_7:
                v36 = 0;
                goto LABEL_9;
            }
        }
        v36 = -v12 - (v12 - 1);
    LABEL_9:
        v35 = v36;
        if (v36)
        {
            v34 = "AuthenticAMD";
            v33 = vendor;
            while (1)
            {
                v32 = *v33;
                v13 = v32 < (unsigned int)*v34;
                if (v32 != *v34)
                    break;
                if (!v32)
                    goto LABEL_16;
                v31 = v33[1];
                v13 = v31 < (unsigned int)v34[1];
                if (v31 != v34[1])
                    break;
                v33 += 2;
                v34 += 2;
                if (!v31)
                {
                LABEL_16:
                    v30 = 0;
                    goto LABEL_18;
                }
            }
            v30 = -v13 - (v13 - 1);
        LABEL_18:
            v29[4] = v30;
            if (v30)
            {
                *(_DWORD*)name = *(_DWORD*)aUnkn_0;
                *((_DWORD*)name + 1) = 544110447;
                *((_DWORD*)name + 2) = (char*)&loc_555042 + 1;
            }
            else
            {
                *(_DWORD*)name = *(_DWORD*)aUnkn_2;
                *((_DWORD*)name + 1) = 544110447;
                *((_DWORD*)name + 2) = 541347137;
                *((_DWORD*)name + 3) = (char*)&loc_555042 + 1;
            }
        }
        else
        {
            strcpy(name, "Unknown Intel CPU");
        }
    }
}
#endif