//#include "../qcommon/exe_headers.h"

//#include "../client/client.h
#include "win_configure.h"
#include "win_local.h"
#include "win_localize.h"
#include "win_net.h"
#include "win_net_debug.h"

//#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

#include <Windows.h>
#include <tlhelp32.h>

#include <client/client.h>

#include <qcommon/qcommon.h>
#include <qcommon/cmd.h>
#include <qcommon/threads.h>
#include <qcommon/mem_track.h>

#include <universal/com_memory.h>
#include <universal/q_parse.h>
#include <universal/timing.h>

#include <gfx_d3d/r_init.h>

//#include "../qcommon/stringed_ingame.h"

char sys_cmdline[1024];
char sys_exitCmdLine[1024];

HWND g_splashWnd;

sysEvent_t eventQue[0x100];

int eventHead;
int eventTail;

SysInfo sys_info;

bool shouldQuitOnError;

cmd_function_s Sys_In_Restart_f_VAR;
cmd_function_s Sys_Net_Restart_f_VAR;
cmd_function_s Sys_Listen_f_VAR;

static char sys_processSemaphoreFile[0x20];

static void PrintWorkingDir()
{
	char cwd[260];

	_getcwd(cwd, 256);
	Com_Printf(16, "Working directory: %s\n", cwd);
}

static void Win_RegisterClass()
{
	tagWNDCLASSEXA wce{};

	wce.cbSize = sizeof(wce);
	wce.lpfnWndProc = MainWndProc;
	wce.hInstance = g_wv.hInstance;
	wce.hIcon = LoadIconA(g_wv.hInstance, (LPCSTR)1);
	wce.hCursor = LoadCursorA(0, (LPCSTR)0x7F00); // KISAKTODO figure resources out
	wce.hbrBackground = CreateSolidBrush(0);
	wce.lpszClassName = "CoD4";

	if (!RegisterClassExA(&wce))
		Com_Error(ERR_FATAL, "EXE_ERR_COULDNT_REGISTER_WINDOW");
}

sysEvent_t* __cdecl Win_GetEvent(sysEvent_t* result)
{
	size_t v2; // [esp+0h] [ebp-50h]
	char* b; // [esp+10h] [ebp-40h]
	tagMSG msg; // [esp+18h] [ebp-38h] BYREF
	char* s; // [esp+34h] [ebp-1Ch]
	sysEvent_t ev; // [esp+38h] [ebp-18h] BYREF

	Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
	if (eventHead <= eventTail)
	{
		while (PeekMessageA(&msg, 0, 0, 0, 0))
		{
			if (!GetMessageA(&msg, 0, 0, 0))
				Com_Quit_f();
			g_wv.sysMsgTime = msg.time;
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		s = Sys_ConsoleInput();
		if (s)
		{
			v2 = strlen(s);
			b = (char*)Com_AllocEvent(v2 + 1);
			I_strncpyz(b, s, v2);
			Sys_QueEvent(0, SE_CONSOLE, 0, 0, v2 + 1, b);
		}
		if (eventHead <= eventTail)
		{
			memset(&ev, 0, sizeof(ev));
			ev.evTime = Sys_Milliseconds();
		}
		else
		{
			ev = eventQue[(unsigned __int8)eventTail++];
		}
		Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
		*result = ev;
		return result;
	}
	else
	{
		ev = eventQue[(unsigned __int8)eventTail++];
		Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
		*result = ev;
		return result;
	}
}

static int Sys_GetSemaphoreFileName()
{
	char* i; // [esp+0h] [ebp-118h]
	const char* moduleName; // [esp+4h] [ebp-114h]
	char modulePath[268]; // [esp+8h] [ebp-110h] BYREF

	GetModuleFileNameA(0, modulePath, MAX_PATH);
	modulePath[MAX_PATH - 1] = 0;
	moduleName = modulePath;
	for (i = modulePath; *i; ++i)
	{
		if (*i == '\\' || *i == ':')
		{
			moduleName = i + 1;
		}
		else if (*i == '.')
		{
			*i = 0;
		}
	}

	return sprintf_s(sys_processSemaphoreFile, "__%s", moduleName);
}

void __cdecl Sys_QuitAndStartProcess(const char *exeName, const char *parameters)
{
	char pathOrig[268]; // [esp+0h] [ebp-110h] BYREF

	GetCurrentDirectoryA(0x104u, pathOrig);
	if (parameters)
		Com_sprintf(sys_exitCmdLine, 0x400u, "\"%s\\%s\" %s", pathOrig, exeName, parameters);
	else
		Com_sprintf(sys_exitCmdLine, 0x400u, "\"%s\\%s\"", pathOrig, exeName);
	Cbuf_AddText(0, "quit\n");
}

void __cdecl Sys_NormalExit()
{
	DeleteFileA(sys_processSemaphoreFile);
}

void __cdecl Sys_OutOfMemErrorInternal(const char* filename, int line)
{
	HWND ActiveWindow; // eax
	char* v3; // [esp-Ch] [ebp-Ch]
	char* v4; // [esp-8h] [ebp-8h]

	Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
	Com_Printf(16, "Out of memory: filename '%s', line %d\n", filename, line);
	v4 = Win_LocalizeRef("WIN_OUT_OF_MEM_TITLE");
	v3 = Win_LocalizeRef("WIN_OUT_OF_MEM_BODY");
	ActiveWindow = GetActiveWindow();
	MessageBoxA(ActiveWindow, v3, v4, 0x10u);
	exit(-1);
}

int __cdecl Sys_IsGameProcess(DWORD id)
{
	tagMODULEENTRY32 me; // [esp+0h] [ebp-350h] BYREF
	int isGame; // [esp+22Ch] [ebp-124h]
	char* i; // [esp+230h] [ebp-120h]
	char* moduleName; // [esp+234h] [ebp-11Ch]
	char modulePath[268]; // [esp+238h] [ebp-118h] BYREF
	void* snapshot; // [esp+348h] [ebp-8h]
	void* process; // [esp+34Ch] [ebp-4h]

	process = OpenProcess(0x1F0FFFu, 0, id);
	if (!process)
		return 0;
	CloseHandle(process);
	snapshot = CreateToolhelp32Snapshot(8u, id);
	if (snapshot == (void*)-1)
		return 0;
	isGame = 0;
	me.dwSize = 548;
	if (Module32First(snapshot, &me))
	{
		GetModuleFileNameA(0, modulePath, 0x104u);
		modulePath[259] = 0;
		moduleName = modulePath;
		for (i = modulePath; *i; ++i)
		{
			if (*i == 92 || *i == 58)
				moduleName = i + 1;
		}
		while (I_stricmp(me.szModule, moduleName))
		{
			if (!Module32Next(snapshot, &me))
				goto LABEL_15;
		}
		isGame = 1;
	}
LABEL_15:
	CloseHandle(snapshot);
	return isGame;
}

void Sys_NoFreeFilesError()
{
	HWND ActiveWindow; // eax
	char* v1; // [esp-Ch] [ebp-Ch]
	char* v2; // [esp-8h] [ebp-8h]

	Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
	v2 = Win_LocalizeRef("WIN_DISK_FULL_TITLE");
	v1 = Win_LocalizeRef("WIN_DISK_FULL_BODY");
	ActiveWindow = GetActiveWindow();
	MessageBoxA(ActiveWindow, v1, v2, 0x10u);
	exit(-1);
}

int __cdecl Sys_CheckCrashOrRerun()
{
	HWND ActiveWindow; // eax
	char* v2; // [esp-Ch] [ebp-20h]
	char* v3; // [esp-8h] [ebp-1Ch]
	unsigned int procID; // [esp+0h] [ebp-14h] BYREF
	int answer; // [esp+4h] [ebp-10h]
	DWORD byteCount; // [esp+8h] [ebp-Ch] BYREF
	void* file; // [esp+Ch] [ebp-8h]
	unsigned int id; // [esp+10h] [ebp-4h] BYREF

	if (!sys_processSemaphoreFile[0])
		return 1;
	procID = GetCurrentProcessId();
	file = CreateFileA(sys_processSemaphoreFile, 0x80000000, 0, 0, 3u, 2u, 0);
	if (file != (void*)-1)
	{
		if (ReadFile(file, &id, 4u, &byteCount, 0) && byteCount == 4)
		{
			CloseHandle(file);
			if (procID != id && Sys_IsGameProcess(id))
				return 0;
			v3 = Win_LocalizeRef("WIN_IMPROPER_QUIT_TITLE");
			v2 = Win_LocalizeRef("WIN_IMPROPER_QUIT_BODY");
			ActiveWindow = GetActiveWindow();
			answer = MessageBoxA(ActiveWindow, v2, v3, 0x33u);
			if (answer == 6)
			{
				Com_ForceSafeMode();
			}
			else if (answer == 2)
			{
				return 0;
			}
		}
		else
		{
			CloseHandle(file);
		}
	}
	file = CreateFileA(sys_processSemaphoreFile, 0x40000000u, 0, 0, 2u, 2u, 0);
	if (file == (void*)-1)
		Sys_NoFreeFilesError();
	if (!WriteFile(file, &procID, 4u, &byteCount, 0) || byteCount != 4)
	{
		CloseHandle(file);
		Sys_NoFreeFilesError();
	}
	CloseHandle(file);
	return 1;
}

void Sys_Error(const char *error, ...)
{
	tagMSG Msg; // [esp+4h] [ebp-1024h] BYREF
	char string[4100]; // [esp+20h] [ebp-1008h] BYREF
	va_list va; // [esp+1034h] [ebp+Ch] BYREF

	va_start(va, error);
	Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
	Com_PrintStackTrace();
	com_errorEntered = 1;
	Sys_SuspendOtherThreads();
	vsnprintf_s(string, 0x1000u, error, va);
	
	// random gamma crap we don't care about
	// FixWindowsDesktop();

	if (com_dedicated->current.integer && Sys_IsMainThread())
	{
		Sys_ShowConsole();
		Conbuf_AppendText("\n\n");
		Conbuf_AppendText(string);
		Conbuf_AppendText("\n");
		Sys_SetErrorText(string);
		while (GetMessageA(&Msg, 0, 0, 0))
		{
			TranslateMessage(&Msg);
			DispatchMessageA(&Msg);
		}
		exit(0);
	}

	Sys_SetErrorText(string);
	exit(0);
}

void __cdecl Sys_OpenURL(const char *url, int doexit)
{
	const char *v2; // eax
	HWND__ *wnd; // [esp+0h] [ebp-4h]

	if (!ShellExecuteA(0, "open", url, 0, 0, 9))
	{
		v2 = va("EXE_ERR_COULDNT_OPEN_URL", url);
		Com_Error(ERR_DROP, v2);
	}
	wnd = GetForegroundWindow();
	if (wnd)
		ShowWindow(wnd, 3);
	if (doexit)
		Cbuf_AddText(0, "quit\n");
}

void Sys_SpawnQuitProcess()
{
	const char* v0; // eax
	_STARTUPINFOA dst; // [esp+0h] [ebp-60h] BYREF
	void* msgBuf; // [esp+48h] [ebp-18h] BYREF
	_PROCESS_INFORMATION pi; // [esp+4Ch] [ebp-14h] BYREF
	unsigned int error; // [esp+5Ch] [ebp-4h]

	if (sys_exitCmdLine[0])
	{
		memset((unsigned __int8*)&dst, 0, sizeof(dst));
		dst.cb = 68;
		if (!CreateProcessA(0, sys_exitCmdLine, 0, 0, 0, 0, 0, 0, &dst, &pi))
		{
			error = GetLastError();
			FormatMessageA(0x1300u, 0, error, 0x400u, (LPSTR)&msgBuf, 0, 0);
			v0 = va("EXE_ERR_COULDNT_START_PROCESS", sys_exitCmdLine, msgBuf, error);
			Com_Error(ERR_DROP, v0);
		}
	}
}

void __cdecl RefreshQuitOnErrorCondition()
{
	bool v0; // [esp+0h] [ebp-4h]

	if (Dvar_IsSystemActive())
	{
		v0 = Dvar_GetBool("QuitOnError") || Dvar_GetInt("r_vc_compile") == 2;
		shouldQuitOnError = v0;
	}
}

void __cdecl  Sys_Quit()
{
	Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
	timeEndPeriod(1u);
	Sys_SpawnQuitProcess();
	IN_Shutdown();
	Key_Shutdown();
	Sys_DestroyConsole();
	Sys_NormalExit();
	Win_ShutdownLocalization();
	RefreshQuitOnErrorCondition();
	Dvar_Shutdown();
	Cmd_Shutdown();
	CL_ResetStats_f();
	CL_ResetStats_f();
	Sys_ShutdownEvents();
	SL_Shutdown();
	if (!com_errorEntered)
		track_shutdown(0);
	Con_ShutdownChannels();
	exit(0);
}

void __cdecl Sys_Print(char *msg)
{
	Conbuf_AppendTextInMainThread(msg);
}

char *__cdecl Sys_GetClipboardData()
{
	SIZE_T v0; // eax
	SIZE_T v1; // eax
	HANDLE hClipboardData; // [esp+0h] [ebp-Ch]
	char *data; // [esp+4h] [ebp-8h]
	char *cliptext; // [esp+8h] [ebp-4h]

	data = 0;
	if (OpenClipboard(0))
	{
		hClipboardData = GetClipboardData(1u);
		if (hClipboardData)
		{
			cliptext = (char *)GlobalLock(hClipboardData);
			if (cliptext)
			{
				v0 = GlobalSize(hClipboardData);
				data = (char *)Z_Malloc(v0 + 1, "Sys_GetClipboardData", 10);
				v1 = GlobalSize(hClipboardData);
				I_strncpyz(data, cliptext, v1);
				GlobalUnlock(hClipboardData);

				while (*data) {
					if (*data == '\r' || *data == '\n' || *data == '\b') {
						*data = '\0';
						break;
					}

					data++;
				}
			}
		}
		CloseClipboard();
	}
	return data;
}

int __cdecl Sys_SetClipboardData(const char *text)
{
	char v2; // cl
	_BYTE *v3; // [esp+8h] [ebp-20h]
	const char *v4; // [esp+Ch] [ebp-1Ch]
	HGLOBAL hglbCopy; // [esp+24h] [ebp-4h]

	if (!OpenClipboard(0))
		return 0;
	EmptyClipboard();
	hglbCopy = GlobalAlloc(2u, strlen(text) + 1);
	if (hglbCopy)
	{
		v4 = text;
		v3 = (_BYTE*)GlobalLock(hglbCopy);
		do
		{
			v2 = *v4;
			*v3++ = *v4++;
		} while (v2);
		GlobalUnlock(hglbCopy);
		SetClipboardData(1u, hglbCopy);
		CloseClipboard();
		return 1;
	}
	else
	{
		CloseClipboard();
		return 0;
	}
}

void __cdecl Sys_QueEvent(unsigned int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr)
{
	sysEvent_t *ev; // [esp+0h] [ebp-4h]

	Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
	ev = &eventQue[(unsigned __int8)eventHead];
	if (eventHead - eventTail >= 256)
	{
		Com_Printf(16, "Sys_QueEvent: overflow\n");
		if (ev->evPtr)
			Z_Free((char *)ev->evPtr, 10);
		++eventTail;
	}
	++eventHead;
	if (!time)
		time = Sys_Milliseconds();
	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
	Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
}

void Sys_ShutdownEvents()
{
	sysEvent_t *ev; // [esp+0h] [ebp-4h]

	Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
	while (eventHead > eventTail)
	{
		ev = &eventQue[(unsigned __int8)eventTail++];
		if (ev->evPtr)
			Z_Free((char *)ev->evPtr, 10);
	}
	Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
}

void __cdecl Sys_LoadingKeepAlive()
{
	sysEvent_t result; // [esp+0h] [ebp-48h] BYREF
	sysEvent_t v1; // [esp+18h] [ebp-30h]
	sysEvent_t ev; // [esp+30h] [ebp-18h]

	do
	{
		v1 = *Win_GetEvent(&result);
		ev = v1;
	} while (v1.evType);
	R_CheckLostDevice();
}

sysEvent_t *__cdecl Sys_GetEvent(sysEvent_t *result)
{
	sysEvent_t v2; // [esp+0h] [ebp-30h] BYREF
	sysEvent_t v3; // [esp+18h] [ebp-18h]

	v3 = *Win_GetEvent(&v2);
	*result = v3;
	return result;
}

void __cdecl Sys_Init()
{
	// _OSVERSIONINFOA osversion; // [esp+14h] [ebp-A0h] BYREF

	timeBeginPeriod(1u);
	Cmd_AddCommandInternal("in_restart", Sys_In_Restart_f, &Sys_In_Restart_f_VAR);
	Cmd_AddCommandInternal("net_restart", Sys_Net_Restart_f, &Sys_Net_Restart_f_VAR);
	Cmd_AddCommandInternal("net_listen", Sys_Listen_f, &Sys_Listen_f_VAR);

	// REM
	// osversion.dwOSVersionInfoSize = 148;
	// if (!GetVersionExA(&osversion))
	// 	Sys_Error("Couldn't get OS info");
	// if (osversion.dwMajorVersion < 4)
	// 	Sys_Error("Call of Duty 4 Multiplayer requires Windows version 4 or greater");
	// if (!osversion.dwPlatformId)
	// 	Sys_Error("Call of Duty 4 Multiplayer doesn't run on Win32s");
	
	Com_Printf(16, "CPU vendor is \"%s\"\n", sys_info.cpuVendor);
	Com_Printf(16, "CPU name is \"%s\"\n", sys_info.cpuName);
	if (sys_info.logicalCpuCount == 1)
		Com_Printf(16, "%i logical CPU%s reported\n", 1, "");
	else
		Com_Printf(16, "%i logical CPU%s reported\n", sys_info.logicalCpuCount, "s");
	if (sys_info.physicalCpuCount == 1)
		Com_Printf(16, "%i physical CPU%s detected\n", 1, "");
	else
		Com_Printf(16, "%i physical CPU%s detected\n", sys_info.physicalCpuCount, "s");
	Com_Printf(16, "Measured CPU speed is %.2lf GHz\n", sys_info.cpuGHz);
	Com_Printf(16, "Total CPU performance is estimated as %.2lf GHz\n", sys_info.configureGHz);
	Com_Printf(16, "System memory is %i MB (capped at 1 GB)\n", sys_info.sysMB);
	Com_Printf(16, "Video card is \"%s\"\n", sys_info.gpuDescription);
	if (sys_info.SSE)
		Com_Printf(16, "Streaming SIMD Extensions (SSE) %ssupported\n", "");
	else
		Com_Printf(16, "Streaming SIMD Extensions (SSE) %ssupported\n", "not ");
	Com_Printf(16, "\n");
	IN_Init();
}

void Sys_In_Restart_f()
{
	IN_Shutdown();
	IN_Init();
}

void Sys_Net_Restart_f()
{
	NET_Restart();
}

void __cdecl Sys_CreateSplashWindow()
{
	HWND__* hwnd; // [esp+0h] [ebp-54h]
	int bmpSize; // [esp+4h] [ebp-50h]
	int bmpSize_4; // [esp+8h] [ebp-4Ch]
	tagWNDCLASSA wc; // [esp+Ch] [ebp-48h] BYREF
	void* hbmp; // [esp+34h] [ebp-20h]
	int style; // [esp+38h] [ebp-1Ch]
	tagSIZE screenSize; // [esp+3Ch] [ebp-18h]
	tagRECT rc; // [esp+44h] [ebp-10h] BYREF

	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = 0;
	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = g_wv.hInstance;
	wc.hIcon = LoadIconA(g_wv.hInstance, (LPCSTR)1);
	wc.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
	wc.hbrBackground = (HBRUSH__*)6;
	wc.lpszClassName = "CoD Splash Screen";
	if (RegisterClassA(&wc))
	{
		screenSize.cx = GetSystemMetrics(16);
		screenSize.cy = GetSystemMetrics(17);
		hbmp = LoadImageA(0, "cod.bmp", 0, 0, 0, 0x10u);
		if (hbmp)
		{
			g_splashWnd = CreateWindowExA(
				0x40000u,
				"CoD Splash Screen",
				"Call of Duty 4 Multiplayer",
				0x80880000,
				(screenSize.cx - 320) / 2,
				(screenSize.cy - 100) / 2,
				320,
				100,
				0,
				0,
				g_wv.hInstance,
				0);
			if (g_splashWnd)
			{
				style = 0x5000000E;
				hwnd = CreateWindowExA(0, "Static", 0, 0x5000000Eu, 0, 0, 320, 100, g_splashWnd, 0, g_wv.hInstance, 0);
				if (hwnd)
				{
					SendMessageA(hwnd, 0x172u, 0, (LPARAM)hbmp);
					GetWindowRect(hwnd, &rc);
					bmpSize = rc.right - rc.left + 2;
					bmpSize_4 = rc.bottom - rc.top + 2;
					rc.left = (screenSize.cx - bmpSize) / 2;
					rc.right = bmpSize + rc.left;
					rc.top = (screenSize.cy - bmpSize_4) / 2;
					rc.bottom = bmpSize_4 + rc.top;
					AdjustWindowRect(&rc, style, 0);
					SetWindowPos(g_splashWnd, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 4u);
				}
			}
		}
	}
}
void __cdecl Sys_ShowSplashWindow()
{
	if (g_splashWnd)
	{
		ShowWindow(g_splashWnd, 5);
		UpdateWindow(g_splashWnd);
	}
}

int __cdecl Sys_SystemMemoryMB()
{
	HWND ActiveWindow; // eax
	HWND v2; // eax
	char* v3; // [esp-Ch] [ebp-C8h]
	char* v4; // [esp-Ch] [ebp-C8h]
	char* v5; // [esp-8h] [ebp-C4h]
	char* v6; // [esp-8h] [ebp-C4h]
	float v7; // [esp+30h] [ebp-8Ch]
	float v8; // [esp+40h] [ebp-7Ch]
	int sysMB; // [esp+50h] [ebp-6Ch]
	int sysMBa; // [esp+50h] [ebp-6Ch]
	HINSTANCE__* hm; // [esp+54h] [ebp-68h]
	_MEMORYSTATUS status; // [esp+58h] [ebp-64h] BYREF
	int(__stdcall * MemStatEx)(_MEMORYSTATUSEX*); // [esp+78h] [ebp-44h]
	_MEMORYSTATUSEX statusEx; // [esp+7Ch] [ebp-40h] BYREF

	hm = GetModuleHandleA("kernel32.dll");
	if (hm && (MemStatEx = (int(__stdcall*)(_MEMORYSTATUSEX*))GetProcAddress(hm, "GlobalMemoryStatusEx")) != 0)
	{
		statusEx.dwLength = 64;
		MemStatEx(&statusEx);
		if (statusEx.ullAvailVirtual < 0x8000000)
		{
			v5 = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
			v3 = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
			ActiveWindow = GetActiveWindow();
			if (MessageBoxA(ActiveWindow, v3, v5, 0x34u) != 6)
			{
				Sys_NormalExit();
				exit(0);
			}
		}
		v8 = (double)statusEx.ullTotalPhys * 0.00000095367431640625;
		sysMB = (int)(v8 + 0.4999999990686774);
		if ((double)statusEx.ullTotalPhys > (double)sysMB * 1048576.0 || sysMB > 1024)
			return 1024;
		return sysMB;
	}
	else
	{
		status.dwLength = 32;
		GlobalMemoryStatus(&status);
		if (status.dwAvailVirtual < 0x8000000)
		{
			v6 = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
			v4 = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
			v2 = GetActiveWindow();
			if (MessageBoxA(v2, v4, v6, 0x34u) != 6)
			{
				Sys_NormalExit();
				exit(0);
			}
		}
		v7 = (double)status.dwTotalPhys * 0.00000095367431640625;
		sysMBa = (int)(v7 + 0.4999999990686774);
		if ((double)status.dwTotalPhys > (double)sysMBa * 1048576.0 || sysMBa > 1024)
			return 1024;
		return sysMBa;
	}
}

void Sys_FindInfo()
{
	sys_info.logicalCpuCount = Sys_GetCpuCount();
	sys_info.cpuGHz = 1.0 / (((double)1LL - (double)0LL) * msecPerRawTimerTick * 1000000.0);
	sys_info.sysMB = Sys_SystemMemoryMB();
	Sys_DetectVideoCard(512, sys_info.gpuDescription);
	sys_info.SSE = 1; // KISAKTODO if someone from 1990 time travels and complains Sys_SupportsSSE();
	Sys_DetectCpuVendorAndName(sys_info.cpuVendor, sys_info.cpuName);
	Sys_SetAutoConfigureGHz(&sys_info);
}

/*
==================
WinMain

# BURN, BABY, BURN -- MASTER IGNITION ROUTINE
==================
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	Sys_InitializeCriticalSections();
	Sys_InitMainThread();
	track_init();
	Win_InitLocalization();

	const bool allowDupe = I_strnicmp(lpCmdLine, "allowdupe", 9) && lpCmdLine[9] <= ' ';

	// initialize semaphore
	Sys_GetSemaphoreFileName();

	if (!allowDupe || Sys_CheckCrashOrRerun())
	{
		// should never get a previous instance in Win32
		if (!hPrevInstance)
		{
			Com_InitParse();
			Dvar_Init();
			InitTiming(); 
			Sys_FindInfo();
			g_wv.hInstance = hInstance;
			I_strncpyz(sys_cmdline, lpCmdLine, 1024);
			Sys_CreateSplashWindow();
			Sys_ShowSplashWindow();
			Win_RegisterClass();
			SetErrorMode(1);
			Sys_Milliseconds();
			// Profile_Init();
			// Profile_InitContext(0);
			CL_ResetStats_f();
			Com_Init(sys_cmdline);

			if (!com_dedicated->current.integer)
				Cbuf_AddText(0, "readStats\n");

			PrintWorkingDir();

			// LWSS: Punkbuster stuff
			//if ((!com_dedicated || !com_dedicated->current.integer) && !PbClientInitialize(hInstance))
			//	Com_SetErrorMessage("MPUI_NOPUNKBUSTER");
			//if (!PbServerInitialize())
			//{
			//	Com_PrintError(16, "Unable to initialize punkbuster.  Punkbuster is disabled\n");
			//	Com_SetErrorMessage("MPUI_NOPUNKBUSTER");
			//}

			SetFocus(g_wv.hWnd);

			// main game loop
			while (1) {
				// if not running as a game client, sleep a bit
				if (g_wv.isMinimized || (com_dedicated && com_dedicated->current.integer)) {
					Sleep(5);
				}

				// run the game
				Com_Frame();

				// LWSS: Punkbuster stuff
				//if (!com_dedicated || !com_dedicated->integer) {
				//	PbClientProcessEvents();
				//}
				//PbServerProcessEvents();
			}
		}
	}


	Win_ShutdownLocalization();
	track_shutdown(0);
	return 0;
}
