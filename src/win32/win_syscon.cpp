// win_syscon.h
// this include must remain at the top of every CPP file
//Anything above this #include will be ignored by the compiler
//#include "../qcommon/exe_headers.h"

#include "../client/client.h"
#include "win_local.h"

#include <Windows.h>

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <qcommon/threads.h>

#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define ERRORBOX_ID		10
#define ERRORTEXT_ID	11

#define EDIT_ID			100
#define INPUT_ID		101

struct WinConData // sizeof=0x620
{                                       // ...
	HWND__ *hWnd;                       // ...
	HWND__ *hwndBuffer;                 // ...
	HWND__ *codLogo;                    // ...
	HFONT__ *hfBufferFont;              // ...
	HWND__ *hwndInputLine;              // ...
	char errorString[512];              // ...
	char consoleText[512];              // ...
	char returnedText[512];             // ...
	int windowWidth;                    // ...
	int windowHeight;                   // ...
	WNDPROC SysInputLineWndProc;
	//int(__stdcall *SysInputLineWndProc)(HWND__ *, unsigned int, unsigned int, int); // ...
};

static WinConData s_wcd;
unsigned int s_totalChars;

LRESULT __stdcall ConWndProc(HWND__ *hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char* cmdString;

	switch (uMsg)
	{
	case 5u:
		SetWindowPos(s_wcd.hwndBuffer, 0, 5, 70, lParam - 15, HIWORD(lParam) - 100, 0);
		SetWindowPos(s_wcd.hwndInputLine, 0, 5, HIWORD(lParam) - 100 + 78, lParam - 15, 20, 0);
		s_wcd.windowWidth = lParam;
		s_wcd.windowHeight = HIWORD(lParam);
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	case 6u:
		if (wParam)
			SetFocus(s_wcd.hwndInputLine);
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	case 0x10u:
		if (com_dedicated && com_dedicated->current.integer)
		{
			cmdString = (char*)Com_AllocEvent(5);
			strcpy(cmdString, "quit");
			Sys_QueEvent(0, SE_CONSOLE, 0, 0, strlen(cmdString) + 1, cmdString);
		}
		else
		{
			PostQuitMessage(0);
		}
		return 0;
	default:
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
}

extern	void CompleteCommand( void ) ;

LRESULT __stdcall InputLineWndProc(HWND__ *hWnd, UINT uMsg, HWND__ *wParam, LPARAM lParam)
{
	char displayBuffer[1024]; // [esp+20h] [ebp-808h] BYREF
	char inputBuffer[1028]; // [esp+420h] [ebp-408h] BYREF

	if (uMsg == 8)
	{
		if (wParam == s_wcd.hWnd)
		{
			SetFocus(hWnd);
			return 0;
		}
	}
	else if (uMsg == 258 && (int)wParam == 13)
	{
		GetWindowTextA(s_wcd.hwndInputLine, inputBuffer, 1024);
		strncat(s_wcd.consoleText, inputBuffer, 512 - strlen(s_wcd.consoleText) - 5);
		strcat(s_wcd.consoleText, "\n");
		SetWindowTextA(s_wcd.hwndInputLine, "");
		Com_sprintf(displayBuffer, 0x400u, "]%s\n", inputBuffer);
		Sys_Print(displayBuffer);
		return 0;
	}
	return CallWindowProcA((WNDPROC)s_wcd.SysInputLineWndProc, hWnd, uMsg, (WPARAM)wParam, lParam);
}

unsigned int __cdecl Conbuf_CleanText(const char *source, char *target, int sizeofTarget)
{
	const char* start = target;
	const char* last = &target[sizeofTarget - 3];

	while (*source && target <= last)
	{
		if (source[0] == '\n' && source[1] == '\r')
		{
			target[0] = '\r';
			target[1] = '\n';
			target += 2;
			source += 2;
		}
		else if (source[0] == '\r' || source[0] == '\n')
		{
			target[0] = '\r';
			target[1] = '\n';
			target += 2;
			++source;
		}
		else if (source && source[0] == '^' && source[1] && source[1] != '^' && source[1] >= 48 && source[1] <= 57)
		{
			source += 2;
		}
		else
		{
			*target++ = *source++; // can target be null?
		}
	}

	*target = 0;
	return target - start;
}

/*
** Sys_CreateConsole
*/
void __cdecl Sys_CreateConsole(HMODULE hInstance)
{
	tagRECT Rect;

	char text[16388];
	char target[16384];

	static const DWORD dwStyle = 0x80CA0000;

	WNDCLASSA WndClass;
	WndClass.style = 0;
	WndClass.lpfnWndProc = ConWndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIconA(hInstance, reinterpret_cast<LPCSTR>(1));
	WndClass.hCursor = LoadCursorA(0, reinterpret_cast<LPCSTR>(0x7F00));
	WndClass.hbrBackground = reinterpret_cast<HBRUSH>(5);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = "CoD4 WinConsole";

	if (!RegisterClassA(&WndClass))
	{
		return;
	}

	Rect.left = 0;
	Rect.right = 620;
	Rect.top = 0;
	Rect.bottom = 450;
	AdjustWindowRect(&Rect, dwStyle, 0);

	const HWND desktop_window = GetDesktopWindow();
	HDC hDC = GetDC(desktop_window);
	const int swidth = GetDeviceCaps(hDC, 8);
	const int sheight = GetDeviceCaps(hDC, 10);
	ReleaseDC(desktop_window, hDC);

	s_wcd.windowWidth = Rect.right - Rect.left + 1;
	s_wcd.windowHeight = Rect.bottom - Rect.top + 1;

	s_wcd.hWnd = CreateWindowExA(
		0,
		"CoD4 WinConsole",
		"CoD4 Console",
		dwStyle,
		(swidth - 600) / 2,
		(sheight - 450) / 2,
		Rect.right - Rect.left + 1,
		Rect.bottom - Rect.top + 1,
		0,
		0,
		hInstance,
		0);

	if (!s_wcd.hWnd)
	{
		return;
	}

	// create fonts
	hDC = GetDC(s_wcd.hWnd);
	const int device_caps = GetDeviceCaps(hDC, 90);
	int cHeight = -MulDiv(8, device_caps, 72);
	s_wcd.hfBufferFont = CreateFontA(cHeight, 0, 0, 0, 300, 0, 0, 0, 1u, 0, 0, 0, 0x31u, "Courier New");
	ReleaseDC(s_wcd.hWnd, hDC);

	const HANDLE logo = LoadImageA(0, "codlogo.bmp", 0, 0, 0, 0x10u);
	if (logo)
	{
		s_wcd.codLogo = CreateWindowExA(
			0, "Static", 0, 0x5000000Eu, 5, 5, 0, 0, s_wcd.hWnd, reinterpret_cast<HMENU>(1), hInstance, 0);
		SendMessageA(s_wcd.codLogo, 0x172u, 0, reinterpret_cast<LPARAM>(logo));
	}

	// create the input line
	s_wcd.hwndInputLine = CreateWindowExA(0, "edit", 0, 0x50800080u, 6, 400, 608, 20, s_wcd.hWnd, (HMENU)0x65, hInstance, 0);
	s_wcd.hwndBuffer = CreateWindowExA(0, "edit", 0, 0x50A00844u, 6, 70, 606, 324, s_wcd.hWnd, (HMENU)0x64, hInstance, 0);
	SendMessageA(s_wcd.hwndBuffer, 0x30u, reinterpret_cast<WPARAM>(s_wcd.hfBufferFont), 0);

	s_wcd.SysInputLineWndProc = reinterpret_cast<WNDPROC>(
		SetWindowLongA(s_wcd.hwndInputLine, -4, reinterpret_cast<LONG>(InputLineWndProc)));

	SendMessageA(s_wcd.hwndInputLine, 0x30u, reinterpret_cast<WPARAM>(s_wcd.hfBufferFont), 0);

	SetFocus(s_wcd.hwndInputLine);
	Con_GetTextCopy(text, 0x4000);
	Conbuf_CleanText(text, target, 0x4000);
	SetWindowTextA(s_wcd.hwndBuffer, target);
}

/*
** Sys_DestroyConsole
*/
void __cdecl Sys_DestroyConsole()
{
	if (s_wcd.hWnd)
	{
		ShowWindow(s_wcd.hWnd, 0);
		CloseWindow(s_wcd.hWnd);
		DestroyWindow(s_wcd.hWnd);
		s_wcd.hWnd = 0;
	}
}

/*
** Sys_ShowConsole
*/
void __cdecl Sys_ShowConsole()
{
	HMODULE module;

	if (!s_wcd.hWnd)
	{
		module = GetModuleHandleA(0);
		Sys_CreateConsole(module);

		if (!s_wcd.hWnd)
			MyAssertHandler(".\\win32\\win_syscon.cpp", 385, 0, "%s", "s_wcd.hWnd");
	}

	ShowWindow(s_wcd.hWnd, 1);
	SendMessageA(s_wcd.hwndBuffer, 0xB6u, 0, 0xFFFF);
}

/*
** Sys_ConsoleInput
*/
char *Sys_ConsoleInput( void )
{
	if ( s_wcd.consoleText[0] == 0 )
	{
		return NULL;
	}
		
	strcpy( s_wcd.returnedText, s_wcd.consoleText );
	s_wcd.consoleText[0] = 0;
	
	return s_wcd.returnedText;
}

/*
** Conbuf_AppendText
*/
void __cdecl Conbuf_AppendText(const char *pMsg)
{
	char target[32772]; // [esp+20h] [ebp-8010h] BYREF
	const char *source; // [esp+8028h] [ebp-8h]

	if (!s_wcd.hwndBuffer)
		MyAssertHandler(".\\win32\\win_syscon.cpp", 420, 0, "%s", "s_wcd.hwndBuffer");
	if (strlen(pMsg) <= 0x3FFF)
		source = pMsg;
	else
		source = &pMsg[strlen(pMsg) - 0x3FFF];

	unsigned int character_amount = Conbuf_CleanText(source, target, 0x8000);
	s_totalChars += character_amount;
	if (s_totalChars <= 0x4000)
	{
		SendMessageA(s_wcd.hwndBuffer, 0xB1u, 0xFFFFu, 0xFFFF);
	}
	else
	{
		SendMessageA(s_wcd.hwndBuffer, 0xB1u, 0, -1);
		s_totalChars = character_amount;
	}

	SendMessageA(s_wcd.hwndBuffer, 0xB6u, 0, 0xFFFF);
	SendMessageA(s_wcd.hwndBuffer, 0xB7u, 0, 0);
	SendMessageA(s_wcd.hwndBuffer, 0xC2u, 0, (LPARAM)target);
}
/*
** Sys_SetErrorText
*/
void __cdecl Sys_SetErrorText(const char *buf)
{
	HWND ActiveWindow; // eax

	I_strncpyz(s_wcd.errorString, buf, 512);
	DestroyWindow(s_wcd.hwndInputLine);
	s_wcd.hwndInputLine = 0;
	ActiveWindow = GetActiveWindow();
	MessageBoxA(ActiveWindow, buf, "Error", 0x10u);
}

void __cdecl Conbuf_AppendTextInMainThread(const char* msg)
{
	if (s_wcd.hwndBuffer)
	{
		if (Sys_IsMainThread())
			Conbuf_AppendText(msg);
	}
}