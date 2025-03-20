//#include "../qcommon/exe_headers.h"

//#include "../client/client.h"
#include "win_local.h"
//#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

#include <Windows.h>
#include "../qcommon/qcommon.h"

#include <qcommon/threads.h>
#include "win_localize.h"
#include <qcommon/mem_track.h>
#include <universal/com_memory.h>
//#include "../qcommon/stringed_ingame.h"

static void PrintWorkingDir()
{
	char cwd[260];

	_getcwd(cwd, 256);
	Com_Printf(16, "Working directory: %s\n", cwd);
}

static void Win_RegisterClass()
{
	tagWNDCLASSEXA wce; // [esp+0h] [ebp-30h] BYREF

	memset((unsigned __int8 *)&wce, 0, sizeof(wce));
	wce.cbSize = 48;
	wce.lpfnWndProc = MainWndProc;
	wce.hInstance = g_wv.hInstance;
	wce.hIcon = LoadIconA(g_wv.hInstance, (LPCSTR)1);
	wce.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
	wce.hbrBackground = CreateSolidBrush(0);
	wce.lpszClassName = "CoD4";
	if (!RegisterClassExA(&wce))
		Com_Error(ERR_FATAL, "EXE_ERR_COULDNT_REGISTER_WINDOW");
}

/*
==================
WinMain

==================
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	Sys_InitializeCriticalSections();
	Sys_InitMainThread();
	track_init();
	Win_InitLocalization();
	
	if (!I_strnicmp(lpCmdLine, "allowdupe", 9) && lpCmdLine[9] <= 32
		|| (Sys_GetSemaphoreFileName(), Sys_CheckCrashOrRerun()))
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
			Profile_Init();
			Profile_InitContext(0);
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
				if (g_wv.isMinimized || (com_dedicated && com_dedicated->integer)) {
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
	_vsnprintf(string, 0x1000u, error, va);
	FixWindowsDesktop();

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

double __cdecl ConvertToMB(int bytes)
{
	return (float)((double)bytes / 1048576.0);
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
				strtok(data, "\n\r\b");
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
		v3 = GlobalLock(hglbCopy);
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
	_OSVERSIONINFOA osversion; // [esp+14h] [ebp-A0h] BYREF

	timeBeginPeriod(1u);
	Cmd_AddCommandInternal("in_restart", Sys_In_Restart_f, &Sys_In_Restart_f_VAR);
	Cmd_AddCommandInternal("net_restart", Sys_Net_Restart_f, &Sys_Net_Restart_f_VAR);
	Cmd_AddCommandInternal("net_listen", Sys_Listen_f, &Sys_Listen_f_VAR);
	osversion.dwOSVersionInfoSize = 148;
	if (!GetVersionExA(&osversion))
		Sys_Error("Couldn't get OS info");
	if (osversion.dwMajorVersion < 4)
		Sys_Error("Call of Duty 4 Multiplayer requires Windows version 4 or greater");
	if (!osversion.dwPlatformId)
		Sys_Error("Call of Duty 4 Multiplayer doesn't run on Win32s");
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

void __cdecl Sys_In_Restart_f()
{
	IN_Shutdown();
	IN_Init();
}

void __cdecl Sys_Net_Restart_f()
{
	NET_Restart();
}

