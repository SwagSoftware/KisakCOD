// win_input.c -- win32 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.
//Anything above this #include will be ignored by the compiler
//#include "../qcommon/exe_headers.h"

#include "../client/client.h"
#include "win_local.h"
#include <gfx_d3d/r_dvars.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <client/cl_input.h>
#endif


struct WinMouseVars_t // sizeof=0x10
{                                       // ...
	int oldButtonState;                 // ...
	tagPOINT oldPos;                    // ...
	bool mouseActive;                   // ...
	bool mouseInitialized;              // ...
	// padding byte
	// padding byte
};

static WinMouseVars_t s_wmv;

static int	window_center_x, window_center_y;

//
// MIDI definitions
//
static void IN_StartupMIDI( void );
static void IN_ShutdownMIDI( void );

#define MAX_MIDIIN_DEVICES	8

typedef struct {
	int			numDevices;
	MIDIINCAPS	caps[MAX_MIDIIN_DEVICES];

	HMIDIIN		hMidiIn;
} MidiInfo_t;

static MidiInfo_t s_midiInfo;

//
// Joystick definitions
//
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V

typedef struct {
	qboolean	avail;
	int			id;			// joystick number
	JOYCAPS		jc;

	int			oldbuttonstate;
	int			oldpovstate;

	JOYINFOEX	ji;
} joystickInfo_t;

static	joystickInfo_t	joy;


const dvar_t	*in_midi;
const dvar_t	*in_midiport;
const dvar_t	*in_midichannel;
const dvar_t	*in_mididevice;
const dvar_t	*in_mouse;
const dvar_t	*in_joystick;
const dvar_t	*in_joyBallScale;
const dvar_t	*in_debugJoystick;
const dvar_t	*joy_threshold;
const dvar_t	*joy_xbutton;
const dvar_t	*joy_ybutton;

qboolean	in_appactive;

// forward-referenced functions
void IN_StartupJoystick (void);
void IN_JoyMove(void);

static void MidiInfo_f( void );

/*
============================================================

WIN32 MOUSE CONTROL

============================================================
*/

/*
================
IN_InitWin32Mouse
================
*/
void IN_InitWin32Mouse( void ) 
{
}

/*
================
IN_ShutdownWin32Mouse
================
*/
void IN_ShutdownWin32Mouse( void ) {
}

/*
================
IN_ActivateWin32Mouse
================
*/
void IN_ActivateWin32Mouse( void ) {
	int			width, height;
	RECT		window_rect;

	width = GetSystemMetrics (SM_CXSCREEN);
	height = GetSystemMetrics (SM_CYSCREEN);

	GetWindowRect ( g_wv.hWnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width-1;
	if (window_rect.bottom >= height-1)
		window_rect.bottom = height-1;
	window_center_x = (window_rect.right + window_rect.left)/2;
	window_center_y = (window_rect.top + window_rect.bottom)/2;

	SetCursorPos (window_center_x, window_center_y);

	SetCapture ( g_wv.hWnd );
	ClipCursor (&window_rect);
	while (ShowCursor (FALSE) >= 0)
		;
}

/*
================
IN_DeactivateWin32Mouse
================
*/
void IN_DeactivateWin32Mouse( void ) 
{
	ClipCursor (NULL);
	ReleaseCapture ();
	while (ShowCursor (TRUE) < 0)
		;
}

/*
================
IN_Win32Mouse
================
*/
void IN_Win32Mouse( int *mx, int *my ) {
	POINT		current_pos;

	// find mouse movement
	GetCursorPos (&current_pos);

	// force the mouse to the center, so there's room to move
	SetCursorPos (window_center_x, window_center_y);

	*mx = current_pos.x - window_center_x;
	*my = current_pos.y - window_center_y;
}


/*
============================================================

DIRECT INPUT MOUSE CONTROL

============================================================
*/

// LWSS: This was causing psycho link errors. Nice one guys
//#undef DEFINE_GUIDX
//
//#define DEFINE_GUIDX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
//        EXTERN_C const GUID name \
//                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
//
//DEFINE_GUIDX(GUID_SysMouse,   0x6F1D2B60,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
//DEFINE_GUIDX(GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
//DEFINE_GUIDX(GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
//DEFINE_GUIDX(GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);


#define DINPUT_BUFFERSIZE           16
#define iDirectInputCreate(a,b,c,d)	pDirectInputCreate(a,b,c,d)

HRESULT (WINAPI *pDirectInputCreate)(HINSTANCE hinst, DWORD dwVersion,
	LPDIRECTINPUT * lplpDirectInput, LPUNKNOWN punkOuter);

static HINSTANCE hInstDI;

typedef struct MYDATA {
	LONG  lX;                   // X axis goes here
	LONG  lY;                   // Y axis goes here
	LONG  lZ;                   // Z axis goes here
	BYTE  bButtonA;             // One button goes here
	BYTE  bButtonB;             // Another button goes here
	BYTE  bButtonC;             // Another button goes here
	BYTE  bButtonD;             // Another button goes here
} MYDATA;

static DIOBJECTDATAFORMAT rgodf[] = {
  { &GUID_XAxis,    FIELD_OFFSET(MYDATA, lX),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_YAxis,    FIELD_OFFSET(MYDATA, lY),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_ZAxis,    FIELD_OFFSET(MYDATA, lZ),       0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonA), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonB), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonC), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonD), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
};

#define NUM_OBJECTS (sizeof(rgodf) / sizeof(rgodf[0]))

static DIDATAFORMAT	df = {
	sizeof(DIDATAFORMAT),       // this structure
	sizeof(DIOBJECTDATAFORMAT), // size of object data format
	DIDF_RELAXIS,               // absolute axis coordinates
	sizeof(MYDATA),             // device data size
	NUM_OBJECTS,                // number of objects
	rgodf,                      // and here they are
};

BOOL __cdecl IN_IsForegroundWindow()
{
	return GetForegroundWindow() == g_wv.hWnd;
}

void __cdecl IN_SetForegroundWindow()
{
	SetForegroundWindow(g_wv.hWnd);
	SetFocus(g_wv.hWnd);
}

/*
============================================================

  MOUSE CONTROL

============================================================
*/

/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void __cdecl IN_ActivateMouse(int force)
{
	if (s_wmv.mouseInitialized)
	{
		if (!r_fullscreen)
			MyAssertHandler(".\\win32\\win_input.cpp", 330, 0, "%s", "r_fullscreen");
		if (in_mouse->current.enabled)
		{
			if (force || !s_wmv.mouseActive)
				s_wmv.mouseActive = IN_IsForegroundWindow() != 0;
		}
		else
		{
			s_wmv.mouseActive = 0;
		}
	}
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void __cdecl IN_DeactivateMouse()
{
	if (s_wmv.mouseInitialized && s_wmv.mouseActive)
	{
		s_wmv.mouseActive = 0;
		IN_DeactivateWin32Mouse();
	}
}



/*
===========
IN_StartupMouse
===========
*/
void __cdecl IN_StartupMouse()
{
	s_wmv.mouseInitialized = 0;
	if (in_mouse->current.enabled)
	{
		s_wmv.mouseInitialized = 1;
		KISAK_NULLSUB();
	}
	else
	{
		Com_Printf(16, "Mouse control not active.\n");
	}
}

/*
===========
IN_MouseEvent
===========
*/
#define MAX_MOUSE_BUTTONS	5

//static int mouseConvert[MAX_MOUSE_BUTTONS] =
//{
//	A_MOUSE1,
//	A_MOUSE2,
//	A_MOUSE3,
//	A_MOUSE4,
//	A_MOUSE5
//};

void __cdecl IN_MouseEvent(int mstate)
{
	int diff; // [esp+0h] [ebp-8h]
	int button; // [esp+4h] [ebp-4h]

	if (s_wmv.mouseInitialized)
	{
		diff = s_wmv.oldButtonState ^ mstate;
		if (s_wmv.oldButtonState != mstate)
		{
			for (button = 0; button < MAX_MOUSE_BUTTONS; ++button)
			{
				if ((diff & (1 << button)) != 0)
					Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, button + 200, (mstate & (1 << button)) != 0, 0, 0);
			}
			s_wmv.oldButtonState = mstate;
		}
	}
}


void __cdecl IN_ClampMouseMove(tagPOINT *curPos)
{
	bool isClamped; // [esp+3h] [ebp-11h]
	tagRECT rc; // [esp+4h] [ebp-10h] BYREF

	GetWindowRect(g_wv.hWnd, &rc);
	isClamped = 0;
	if (curPos->x >= rc.left)
	{
		if (curPos->x >= rc.right)
		{
			curPos->x = rc.right - 1;
			isClamped = 1;
		}
	}
	else
	{
		curPos->x = rc.left;
		isClamped = 1;
	}
	if (curPos->y >= rc.top)
	{
		if (curPos->y >= rc.bottom)
		{
			curPos->y = rc.bottom - 1;
			isClamped = 1;
		}
	}
	else
	{
		curPos->y = rc.top;
		isClamped = 1;
	}
	if (isClamped)
		SetCursorPos(curPos->x, curPos->y);
}

/*
===========
IN_MouseMove
===========
*/
void __cdecl IN_RecenterMouse();
int IN_MouseMove()
{
	int result; // eax
	int v1; // [esp+0h] [ebp-10h]
	tagPOINT curPos; // [esp+4h] [ebp-Ch] BYREF
	int dy; // [esp+Ch] [ebp-4h]

	if (!s_wmv.mouseInitialized)
		MyAssertHandler(".\\win32\\win_input.cpp", 466, 0, "%s", "s_wmv.mouseInitialized");
	if (!r_fullscreen)
		MyAssertHandler(".\\win32\\win_input.cpp", 467, 0, "%s", "r_fullscreen");
	result = IN_IsForegroundWindow();
	if (result)
	{
		GetCursorPos(&curPos);
		if (r_fullscreen->current.enabled)
			IN_ClampMouseMove(&curPos);
		v1 = curPos.x - s_wmv.oldPos.x;
		dy = curPos.y - s_wmv.oldPos.y;
		s_wmv.oldPos = curPos;
		ScreenToClient(g_wv.hWnd, &curPos);
		result = CL_MouseEvent(curPos.x, curPos.y, v1, dy);
		g_wv.recenterMouse = result;
		if (result && (v1 || dy))
		{
			IN_RecenterMouse();
			result = window_center_x;
			s_wmv.oldPos.x = window_center_x;
			s_wmv.oldPos.y = window_center_y;
		}
	}
	return result;
}


/*
=========================================================================

=========================================================================
*/

void __cdecl IN_ShowSystemCursor(BOOL show)
{
	int actualShow; // [esp+0h] [ebp-8h]
	int desiredShow; // [esp+4h] [ebp-4h]

	desiredShow = show - 1;
	for (actualShow = ShowCursor(show); actualShow != desiredShow; actualShow = ShowCursor(actualShow < desiredShow))
		;
}

/*
===========
IN_Startup
===========
*/
void __cdecl IN_Startup()
{
	IN_StartupMouse();
	Dvar_ClearModified((dvar_s*)in_mouse);
}

/*
===========
IN_Shutdown
===========
*/
void __cdecl IN_Shutdown()
{
	IN_DeactivateMouse();
}

/*
===========
IN_Init
===========
*/
void __cdecl IN_Init()
{
	in_mouse = Dvar_RegisterBool("in_mouse", 1, DVAR_ARCHIVE | DVAR_LATCH, "Initialize the mouse drivers");
	IN_Startup();
}

/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
// LWSS: Done
void IN_Activate (qboolean active) {
	in_appactive = active;

	if (active)
	{
		IN_ActivateMouse(1);
	}
	else
	{
		IN_DeactivateMouse();
	}
}

extern const dvar_t	*r_fullscreen;

/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void __cdecl IN_Frame()
{
	if (Dvar_GetBool("ClickToContinue"))
		PostMessageA(g_wv.hWnd, 0x201u, 1u, 0);
	if (s_wmv.mouseInitialized)
	{
		if (in_appactive)
		{
			IN_ActivateMouse(0);
			IN_MouseMove();
		}
		else
		{
			IN_DeactivateMouse();
		}
	}
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) 
{
	s_wmv.oldButtonState = 0;
}

void __cdecl IN_RecenterMouse()
{
	tagRECT window_rect; // [esp+0h] [ebp-10h] BYREF

	GetWindowRect(g_wv.hWnd, &window_rect);
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.bottom + window_rect.top) / 2;
	SetCursorPos((window_rect.left + window_rect.right) / 2, (window_rect.bottom + window_rect.top) / 2);
}
void __cdecl IN_SetCursorPos(tagPOINT x)
{
	tagPOINT curPos; // [esp+0h] [ebp-8h] BYREF

	curPos = x;
	ClientToScreen(g_wv.hWnd, &curPos);
	SetCursorPos(curPos.x, curPos.y);
	s_wmv.oldPos = curPos;
}
