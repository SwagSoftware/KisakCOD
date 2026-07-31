#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif

// MFC precompiled header (static MFC; CMAKE_MFC_FLAG 1).
// afxwin.h MUST be the first include in every editor TU — it pulls in <windows.h>
// with MFC's required settings and hard-errors ("WINDOWS.H already included...")
// if <windows.h> was pulled first. Do NOT define WIN32_LEAN_AND_MEAN here.
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

// ---- DXSDK June 2010 vs modern Windows SDK shim -------------------------------
// The DXSDK include dir (target include) shadows the Windows SDK's dxgitype.h.
// Modern <wincodec.h> (pulled in by the MFC headers below) references three
// DXGI_JPEG_* types that exist only in the modern dxgitype.h, so every TU that
// includes this header fails with C2061 on a full rebuild. Define them up front
// (layouts verbatim from the Windows SDK). unsigned char == BYTE; <windows.h>
// must not be included before afxwin.h, so BYTE itself is unavailable here.
// Safe: the June 2010 dxgitype.h always wins the include search for this target,
// so the modern definitions are never also in scope.
#define KISAK_RADIANT_DXGI_JPEG_SHIM 1
typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE
{
    unsigned char CodeCounts[12];
    unsigned char CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;
typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE
{
    unsigned char CodeCounts[16];
    unsigned char CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;
typedef struct DXGI_JPEG_QUANTIZATION_TABLE
{
    unsigned char Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;
// --------------------------------------------------------------------------------

// C4005 (macro redefinition): MFC's afxrendertarget.h includes <d2d1.h>, which the
// DXSDK include dir shadows (same mechanism as the DXGI shim above). The June 2010
// D2DErr.h then redefines every D2DERR_* the modern winerror.h already defined —
// same values, different token spelling (MAKE_D2DHR vs _HRESULT_TYPEDEF_), hence
// hundreds of warnings per TU. Scoped to the MFC includes only.
#pragma warning(push)
#pragma warning(disable: 4005)
#include <afxwin.h>     // MFC core and standard components
#include <afxext.h>     // MFC extensions (CFrameWnd helpers, control bars)
#include <afxcmn.h>     // MFC support for Windows Common Controls
#include <afxmt.h>      // MFC synchronization
#include <afxtempl.h>   // MFC container templates
#pragma warning(pop)

// Standard C/C++
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// D3D9 (renderer bridge in later phases)
#include <d3d9.h>
#include <d3dx9.h>

// GDI+ (uses std::min/std::max)
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>

// Resource IDs
#include "res/resource.h"

// Editor object model (plane_t, winding_t, vec3_t, brush_t, face_t, qeglobals_t, …).
// GtkRadiant 1.6 stdafx.h also includes qe3.h as the universal editor type header.
// Must come AFTER afxwin.h (needs HWND/HINSTANCE) and resource.h (ICON IDs).
#include "qe3.h"
