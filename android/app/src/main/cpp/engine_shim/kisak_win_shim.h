// Minimal Windows-compatibility surface so the decompiled engine sources can
// compile with the Android NDK. Only the types and macros actually referenced
// by the ported translation units belong here.
#pragma once

#include <cstdint>
#include <cstddef>

#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using UINT = unsigned int;
using BOOL = int32_t;
using LONG = int32_t;
using ULONG = uint32_t;
using HRESULT = int32_t;
using LPVOID = void*;
using HANDLE = void*;
using HWND = void*;
using HINSTANCE = void*;
using HMODULE = void*;
using LPCSTR = const char*;
using LPSTR = char*;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
