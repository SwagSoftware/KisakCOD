#include "r_texturemem.h"
#include <universal/assertive.h>
#include "r_init.h"

#define INITGUID 
#include <ddraw.h>

unsigned int s_maxReportedTexMem;

unsigned int __cdecl R_VideoMemoryForDevice(_GUID *lpGUID)
{
    _DDSCAPS2 caps; // [esp+0h] [ebp-20h] BYREF
    HRESULT hr; // [esp+10h] [ebp-10h]
    IDirectDraw7 *dd; // [esp+14h] [ebp-Ch] BYREF
    DWORD total; // [esp+18h] [ebp-8h] BYREF
    DWORD free; // [esp+1Ch] [ebp-4h] BYREF

    hr = DirectDrawCreateEx(lpGUID, (LPVOID *)&dd, IID_IDirectDraw7, 0);
    if (hr < 0)
        return 0;
    memset(&caps.dwCaps2, 0, 12);
    caps.dwCaps = 0x10000000;
    hr = dd->GetAvailableVidMem(&caps, &total, &free);
    dd->Release();
    if (hr >= 0)
        return total;
    else
        return 0;
}

int __stdcall R_DDEnumCallback(
    _GUID *lpGUID,
    char *lpDriverDescription,
    char *lpDriverName,
    unsigned int *lpContext,
    HMONITOR__ *hm)
{
    unsigned int total; // [esp+0h] [ebp-4h]

    if (hm)
        return 1;
    total = R_VideoMemoryForDevice(lpGUID);
    if (*lpContext < total)
        *lpContext = total;
    return 1;
}

unsigned int __cdecl R_DrasticVideoMemoryForDevice(_GUID *lpGUID)
{
    HMODULE ModuleHandleA; // eax
    _DDSCAPS2 caps; // [esp+0h] [ebp-24h] BYREF
    HRESULT hr; // [esp+10h] [ebp-14h]
    IDirectDraw7 *dd; // [esp+14h] [ebp-10h] BYREF
    DWORD total; // [esp+18h] [ebp-Ch] BYREF
    HWND__ *hwndDummy; // [esp+1Ch] [ebp-8h]
    DWORD free; // [esp+20h] [ebp-4h] BYREF

    hr = DirectDrawCreateEx(lpGUID, (LPVOID *)&dd, IID_IDirectDraw7, 0);
    if (hr < 0)
        return 0;
    ModuleHandleA = GetModuleHandleA(0);
    hwndDummy = CreateWindowExA(0, "static", "dummy", 0, 0, 0, 1, 1, 0, 0, ModuleHandleA, 0);
    if (hwndDummy)
    {
        hr = dd->SetCooperativeLevel(hwndDummy, 17u);
        if (hr >= 0)
        {
            memset(&caps.dwCaps2, 0, 12);
            caps.dwCaps = 0x4000;
            //hr = ((int(__thiscall *)(IDirectDraw7 *, IDirectDraw7 *, _DDSCAPS2 *, unsigned int *, unsigned int *))dd->GetAvailableVidMem)(
            //    dd,
            //    dd,
            //    &caps,
            //    &total,
            //    &free);
            hr = dd->GetAvailableVidMem(&caps, &total, &free);
            DestroyWindow(hwndDummy);
            dd->Release();
            if (hr >= 0)
                return total;
            else
                return 0;
        }
        else
        {
            DestroyWindow(hwndDummy);
            dd->Release();
            return 0;
        }
    }
    else
    {
        //((void(__thiscall *)(IDirectDraw7 *, IDirectDraw7 *))dd->Release)(dd, dd);
        dd->Release();
        return 0;
    }
}

int __stdcall R_DDEnumDrasticCallback(
    _GUID *lpGUID,
    char *lpDriverDescription,
    char *lpDriverName,
    unsigned int *lpContext,
    HMONITOR__ *hm)
{
    unsigned int total; // [esp+0h] [ebp-4h]

    total = R_DrasticVideoMemoryForDevice(lpGUID);
    if (*lpContext < total)
        *lpContext = total;
    return 1;
}

unsigned int __cdecl R_VideoMemory()
{
    unsigned int total; // [esp+0h] [ebp-8h] BYREF
    unsigned int size; // [esp+4h] [ebp-4h]

    total = R_VideoMemoryForDevice(0);
    if (!total)
    {
        DirectDrawEnumerateExA((LPDDENUMCALLBACKEXA)R_DDEnumCallback, &total, 0);
        if (!total)
        {
            total = R_DrasticVideoMemoryForDevice(0);
            if (!total)
            {
                DirectDrawEnumerateExA((LPDDENUMCALLBACKEXA)R_DDEnumDrasticCallback, &total, 0);
                if (!total)
                    return 0;
            }
        }
    }
    total = ((total - 1) >> 20) + 1;
    for (size = 1; size < total; size *= 2)
        ;
    if (size - total > 0x20)
        size >>= 1;
    return size;
}

unsigned int __cdecl R_AvailableTextureMemory()
{
    unsigned int currentTexMem; // [esp+0h] [ebp-4h]

    currentTexMem = R_DetectCurrentTextureMemory();
    if (s_maxReportedTexMem >= currentTexMem)
        Com_Printf(8, "Using previously reported texture memory size of %i MB.\n", s_maxReportedTexMem);
    else
        s_maxReportedTexMem = currentTexMem;
    return s_maxReportedTexMem;
}

unsigned int __cdecl R_DetectCurrentTextureMemory()
{
    unsigned int texMemInMegs; // [esp+0h] [ebp-Ch]
    unsigned int vidMemInMegs; // [esp+8h] [ebp-4h]

    iassert( dx.device );
    vidMemInMegs = R_VideoMemory();
    texMemInMegs = dx.device->GetAvailableTextureMem() >> 20;
    if (vidMemInMegs)
    {
        Com_Printf(
            8,
            "DirectX reports %i MB of video memory and %i MB of available texture memory.\n",
            vidMemInMegs,
            texMemInMegs);
        if (vidMemInMegs >= texMemInMegs)
        {
            return texMemInMegs;
        }
        else
        {
            Com_Printf(8, "Using video memory size to cap used texture memory at %i MB.\n", vidMemInMegs - 16);
            return vidMemInMegs - 16;
        }
    }
    else
    {
        Com_Printf(
            8,
            "DirectX reports %i MB of available texture memory, but wouldn't tell available video memory.\n",
            texMemInMegs);
        return texMemInMegs;
    }
}