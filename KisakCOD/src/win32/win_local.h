// win_local.h: Win32-specific Quake3 header file

#if defined (_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(disable : 4201)
#pragma warning( push )
#endif
//#include <windows.h>
//#include "../qcommon/platform.h"
#if defined (_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning( pop )
#endif

#ifndef _XBOX
#define DIRECTINPUT_VERSION 0x0800  //[ 0x0300 | 0x0500 | 0x0700 | 0x0800 ]
#include <dinput.h>
#include <dsound.h>
#include <winsock.h>
#include <wsipx.h>
#endif
#include <universal/q_shared.h>

void	IN_MouseEvent (int mstate);

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );

void	Sys_CreateConsole( void );
void	Sys_DestroyConsole( void );

char	*Sys_ConsoleInput (void);

qboolean	Sys_GetPacket ( netadr_t *net_from, msg_t *net_message );
qboolean	Sys_GetBroadcastPacket( msg_t *net_message );

// Input subsystem

void	IN_Init (void);
void	IN_Shutdown (void);
void	IN_JoystickCommands (void);

void	IN_Move (usercmd_t *cmd);
// add additional non keyboard / non mouse movement on top of the keyboard move cmd

void	IN_DeactivateWin32Mouse( void);

void	IN_Activate (qboolean active);
void	IN_Frame (void);

// window procedure
#ifndef _XBOX
LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam);
#endif

void Conbuf_AppendText( const char *msg );

#ifndef _XBOX
// LWSS: Accurate to cod4
typedef struct
{
	HINSTANCE		reflib_library;		// Handle to refresh DLL 
	qboolean		reflib_active;

	HWND			hWnd;
	HINSTANCE		hInstance;
	qboolean		activeApp;
	qboolean		isMinimized;
	qboolean		recenterMouse;

	OSVERSIONINFO	osversion;

	// when we get a windows message, we store the time off so keyboard processing
	// can know the exact time of an event
	unsigned		sysMsgTime;
} WinVars_t;

extern WinVars_t	g_wv;
#endif

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

// LWSS add

extern _RTL_CRITICAL_SECTION s_criticalSections[];

enum CriticalSection : int
{
	CRITSECT_CONSOLE = 0x0,
	CRITSECT_DEBUG_SOCKET = 0x1,
	CRITSECT_COM_ERROR = 0x2,
	CRITSECT_STATMON = 0x3,
	CRITSECT_DEBUG_LINE = 0x4,
	CRITSECT_ALLOC_MARK = 0x5,
	CRITSECT_SCRIPT_STRING = 0x6,
	CRITSECT_MEMORY_TREE = 0x7,
	CRITSECT_ASSERT = 0x8,
	CRITSECT_RD_BUFFER = 0x9,
	CRITSECT_SYS_EVENT_QUEUE = 0xA,
	CRITSECT_GPU_FENCE = 0xB,
	CRITSECT_FATAL_ERROR = 0xC,
	CRITSECT_SCRIPT_DEBUGGER_ALLOC = 0xD,
	CRITSECT_MISSING_ASSET = 0xE,
	CRITSECT_PHYSICS = 0xF,
	CRITSECT_LIVE = 0x10,
	CRITSECT_AUDIO_PHYSICS = 0x11,
	CRITSECT_CINEMATIC = 0x12,
	CRITSECT_CINEMATIC_TARGET_CHANGE = 0x13,
	CRITSECT_FX_ALLOC = 0x14,
	CRITSECT_CBUF = 0x15,

	CRITSECT_COUNT = 0x16,
};


struct FastCriticalSection
{
	volatile unsigned int readCount;
	volatile unsigned int writeCount;
};

void Sys_InitializeCriticalSections();
void Sys_EnterCriticalSection(int critSect);
void Sys_LeaveCriticalSection(int critSect);
void Sys_LockWrite(FastCriticalSection* critSect);
void Sys_UnlockWrite(FastCriticalSection* critSect);


void Sys_InitMainThread();