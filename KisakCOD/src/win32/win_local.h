// win_local.h: Win32-specific Quake3 header file
#pragma once // addition

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
//#include <dsound.h>
#include <winsock.h>
#include <wsipx.h>
#endif
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/net_chan_mp.h>

void	IN_MouseEvent (int mstate);

void	Sys_CreateConsole( void );
void	Sys_DestroyConsole( void );
void __cdecl Sys_ShowConsole();

char	*Sys_ConsoleInput (void);

void Sys_ShowIP();
bool Sys_IsLANAddress(netadr_t adr);
bool Sys_IsLANAddress_IgnoreSubnet(netadr_t adr);

struct netadr_t;
struct msg_t;

qboolean	Sys_GetPacket ( netadr_t *net_from, msg_t *net_message );
qboolean	Sys_GetBroadcastPacket( msg_t *net_message );

// Input subsystem

void	IN_Init (void);
void	IN_Shutdown (void);
void	IN_JoystickCommands (void);

void __cdecl IN_ShowSystemCursor(BOOL show);

// KISAKTODO void	IN_Move (usercmd_s *cmd); // usercmd_t -> usercmd_s
// add additional non keyboard / non mouse movement on top of the keyboard move cmd

void	IN_DeactivateWin32Mouse( void);

void	IN_Activate (qboolean active);
void	IN_Frame (void);

bool IN_IsTalkKeyHeld();

// window procedure
#ifndef _XBOX
LRESULT WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam);
#endif

void Conbuf_AppendText( const char *msg );
void Conbuf_AppendTextInMainThread(const char* msg);

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

struct __declspec(align(8)) SysInfo // sizeof=0x260
{                                       // ...
	long double cpuGHz;                 // ...
	long double configureGHz;           // ...
	int logicalCpuCount;                // ...
	int physicalCpuCount;               // ...
	int sysMB;                          // ...
	char gpuDescription[512];           // ...
	bool SSE;                           // ...
	char cpuVendor[13];                 // ...
	char cpuName[49];                   // ...
	// padding byte
	// padding byte
	// padding byte
	// padding byte
	// padding byte
};

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

// LWSS add

extern _RTL_CRITICAL_SECTION s_criticalSections[];
extern int client_state; // LWSS ADD. This looks similar to signonstate
extern HWND g_splashWnd;


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

struct sysEvent_t // sizeof=0x18
{                                       // ...
	int evTime;                         // ...
	sysEventType_t evType;              // ...
	int evValue;                        // ...
	int evValue2;                       // ...
	int evPtrLength;                    // ...
	void *evPtr;                        // ...
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

void Sys_SetErrorText(const char* buf);
void Sys_Error(const char *error, ...);
void __cdecl Sys_OutOfMemErrorInternal(const char* filename, int line);
void __cdecl Sys_NormalExit();

void __cdecl Sys_OpenURL(const char *url, int doexit);
void __cdecl  Sys_Quit();
void __cdecl Sys_Print(const char *msg);
char *__cdecl Sys_GetClipboardData();
int __cdecl Sys_SetClipboardData(const char *text);
void __cdecl Sys_QueEvent(unsigned int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr);
void Sys_ShutdownEvents();
void __cdecl Sys_LoadingKeepAlive();
sysEvent_t *__cdecl Sys_GetEvent(sysEvent_t *result);
void __cdecl Sys_Init();

void Sys_In_Restart_f();
void Sys_Net_Restart_f();
void __cdecl Sys_Listen_f();

void __cdecl Sys_Mkdir(const char *path);
BOOL __cdecl Sys_RemoveDirTree(const char *path);
int __cdecl Sys_CountFileList(char **list);
char **__cdecl Sys_ListFiles(
	const char *directory,
	const char *extension,
	const char *filter,
	int *numfiles,
	int wantsubs);
char *__cdecl Sys_Cwd();
const char *__cdecl Sys_DefaultCDPath();
char *__cdecl Sys_DefaultInstallPath();
void __cdecl Sys_QuitAndStartProcess(const char *exeName, const char *parameters);


// win_voice
bool __cdecl Voice_SendVoiceData();
bool __cdecl Voice_Init();
void __cdecl Voice_Shutdown();
double __cdecl Voice_GetVoiceLevel();
void __cdecl Voice_Playback();
int __cdecl Voice_GetLocalVoiceData();
void __cdecl Voice_IncomingVoiceData(unsigned __int8 talker, unsigned __int8 *data, int packetDataSize);
bool __cdecl Voice_IsClientTalking(unsigned int clientNum);
char __cdecl Voice_StartRecording();
char __cdecl Voice_StopRecording();

extern SysInfo sys_info;