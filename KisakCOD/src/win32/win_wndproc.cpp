#include "win_local.h"
#include "win_input.h"

#include <qcommon/qcommon.h>

#include <client/client.h>
#include <client_mp/client_mp.h>

#include <ui/keycodes.h>

#include <sound/snd_public.h>

#include <gfx_d3d/r_dvars.h>
#include <qcommon/cmd.h>

static UINT MSH_MOUSEWHEEL;

#define WM_BUTTON4DOWN	(WM_MOUSELAST+2)
#define WM_BUTTON4UP	(WM_MOUSELAST+3)
#define MK_BUTTON4L		0x0020
#define MK_BUTTON4R		0x0040

// LWSS: Updated for Cod4
static byte virtualKeyConvert[0x92][2] =
{
	{ 0,				0				},
	{ K_MOUSE1,			K_MOUSE1		}, // VK_LBUTTON 01 Left mouse button  
	{ K_MOUSE2,			K_MOUSE2		}, // VK_RBUTTON 02 Right mouse button  
	{ 0,				0				}, // VK_CANCEL 03 Control-break processing  
	{ K_MOUSE3,			K_MOUSE3		}, // VK_MBUTTON 04 Middle mouse button (three-button mouse)  
	{ K_MOUSE4,			K_MOUSE4		}, // VK_XBUTTON1 05 Windows 2000/XP: X1 mouse button 
	{ K_MOUSE5,			K_MOUSE5		}, // VK_XBUTTON2 06 Windows 2000/XP: X2 mouse button 
	{ 0,				0				}, // 07 Undefined  
	{ K_BACKSPACE,		K_BACKSPACE		}, // VK_BACK 08 BACKSPACE key  
	{ K_TAB,			K_TAB			}, // VK_TAB 09 TAB key  
	{ 0,				0				}, // 0A Reserved  
	{ 0,				0				}, // 0B Reserved  
	{ K_KP_5,			0				}, // VK_CLEAR 0C CLEAR key  
	{ K_ENTER, 			K_KP_ENTER 		}, // VK_RETURN 0D ENTER key  
	{ 0,				0				}, // 0E Undefined  
	{ 0,				0				}, // 0F Undefined  
	{ K_SHIFT,			K_SHIFT			}, // VK_SHIFT 10 SHIFT key  
	{ K_CTRL,			K_CTRL			}, // VK_CONTROL 11 CTRL key  
	{ K_ALT,			K_ALT			}, // VK_MENU 12 ALT key  
	{ K_PAUSE,			K_PAUSE			}, // VK_PAUSE 13 PAUSE key  
	{ K_CAPSLOCK,		K_CAPSLOCK		}, // VK_CAPITAL 14 CAPS LOCK key  
	{ 0,				0				}, // VK_KANA 15 IME Kana mode 
	{ 0,				0				}, // 16 Undefined  
	{ 0,				0				}, // VK_JUNJA 17 IME Junja mode 
	{ 0,				0				}, // VK_FINAL 18 IME final mode 
	{ 0,				0				}, // VK_KANJI 19 IME Kanji mode 
	{ 0,				0				}, // 1A Undefined  
	{ K_ESCAPE,			K_ESCAPE		}, // VK_ESCAPE 1B ESC key  
	{ 0,				0				}, // VK_CONVERT 1C IME convert 
	{ 0,				0				}, // VK_NONCONVERT 1D IME nonconvert 
	{ 0,				0				}, // VK_ACCEPT 1E IME accept 
	{ 0,				0				}, // VK_MODECHANGE 1F IME mode change request 
	{ K_SPACE,			K_SPACE			}, // VK_SPACE 20 SPACEBAR  
	{ K_KP_RIGHTARROW,	K_PGUP		    }, // VK_PRIOR 21 PAGE UP key  
	{ K_KP_PGDN,		K_PGDN			}, // VK_NEXT 22 PAGE DOWN key  
	{ K_KP_END,			K_END			}, // VK_END 23 END key  
	{ K_KP_HOME,		K_HOME			}, // VK_HOME 24 HOME key  
	{ K_KP_LEFTARROW,	K_LEFTARROW		}, // VK_LEFT 25 LEFT ARROW key  
	{ K_KP_UPARROW,		K_UPARROW   	}, // VK_UP 26 UP ARROW key  
	{ K_KP_RIGHTARROW,	K_RIGHTARROW	}, // VK_RIGHT 27 RIGHT ARROW key  
	{ K_KP_DOWNARROW,	K_DOWNARROW		}, // VK_DOWN 28 DOWN ARROW key  
	{ 0,				0				}, // VK_SELECT 29 SELECT key  
	{ 0,				0				}, // VK_PRINT 2A PRINT key 
	{ 0,				0				}, // VK_EXECUTE 2B EXECUTE key  
	{ 0,				0				}, // VK_SNAPSHOT 2C PRINT SCREEN key  
	{ K_KP_INS,			K_INS			}, // VK_INSERT 2D INS key  
	{ K_KP_DEL,			K_DEL			}, // VK_DELETE 2E DEL key  
	{ 0,				0				}, // VK_HELP 2F HELP key  
	{ 0x30,				0x30			}, // 30 0 key  
	{ 0x31,				0x31			}, // 31 1 key  
	{ 0x32,				0x32			}, // 32 2 key  
	{ 0x33,				0x33			}, // 33 3 key  
	{ 0x34,				0x34			}, // 34 4 key  
	{ 0x35,				0x35			}, // 35 5 key  
	{ 0x36,				0x36			}, // 36 6 key  
	{ 0x37,				0x37			}, // 37 7 key  
	{ 0x38,				0x38			}, // 38 8 key  
	{ 0x39,				0x39			}, // 39 9 key  
	{ 0,				0				}, // 3A Undefined  
	{ 0,				0				}, // 3B Undefined  
	{ 0,				0				}, // 3C Undefined  
	{ 0,				0				}, // 3D Undefined  
	{ 0,				0				}, // 3E Undefined  
	{ 0,				0				}, // 3F Undefined  
	{ 0,				0				}, // 40 Undefined  
	{ 0x61,				0x41			}, // 41 A key  
	{ 0x62,				0x42			}, // 42 B key  
	{ 0x63,				0x43			}, // 43 C key  
	{ 0x64,				0x44			}, // 44 D key  
	{ 0x65,				0x45			}, // 45 E key  
	{ 0x66,				0x46			}, // 46 F key  
	{ 0x67,				0x47			}, // 47 G key  
	{ 0x68,				0x48			}, // 48 H key  
	{ 0x69,				0x49			}, // 49 I key  
	{ 0x6A,				0x4A			}, // 4A J key  
	{ 0x6B,				0x4B			}, // 4B K key  
	{ 0x6C,				0x4C			}, // 4C L key  
	{ 0x6D,				0x4D			}, // 4D M key  
	{ 0x6E,				0x4E			}, // 4E N key  
	{ 0x6F,				0x4F			}, // 4F O key  
	{ 0x70,				0x50			}, // 50 P key  
	{ 0x71,				0x51			}, // 51 Q key  
	{ 0x72,				0x52			}, // 52 R key  
	{ 0x73,				0x53			}, // 53 S key  
	{ 0x74,				0x54			}, // 54 T key  
	{ 0x75,				0x55			}, // 55 U key  
	{ 0x76,				0x56			}, // 56 V key  
	{ 0x77,				0x57			}, // 57 W key  
	{ 0x78,				0x58			}, // 58 X key  
	{ 0x79,				0x59			}, // 59 Y key  
	{ 0x7A,				0x5A			}, // 5A Z key  
	{ 0,				0				}, // VK_LWIN 5B Left Windows key (Microsoft® Natural® keyboard)  
	{ 0,				0				}, // VK_RWIN 5C Right Windows key (Natural keyboard)  
	{ 0,				0				}, // VK_APPS 5D Applications key (Natural keyboard)  
	{ 0,				0				}, // 5E Reserved  
	{ 0,				0				}, // VK_SLEEP 5F Computer Sleep key 
	{ K_KP_INS,			K_KP_INS		}, // VK_NUMPAD0 60 Numeric keypad 0 key  
	{ K_KP_END,			K_KP_END		}, // VK_NUMPAD1 61 Numeric keypad 1 key  
	{ K_KP_DOWNARROW,	K_KP_DOWNARROW  }, // VK_NUMPAD2 62 Numeric keypad 2 key  
	{ K_KP_PGDN,		K_KP_PGDN		}, // VK_NUMPAD3 63 Numeric keypad 3 key  
	{ K_KP_LEFTARROW,	K_KP_LEFTARROW  }, // VK_NUMPAD4 64 Numeric keypad 4 key  
	{ K_KP_5,			K_KP_5			}, // VK_NUMPAD5 65 Numeric keypad 5 key  
	{ K_KP_RIGHTARROW,	K_KP_RIGHTARROW }, // VK_NUMPAD6 66 Numeric keypad 6 key  
	{ K_KP_HOME,		K_KP_HOME		}, // VK_NUMPAD7 67 Numeric keypad 7 key  
	{ K_KP_UPARROW,		K_KP_UPARROW	}, // VK_NUMPAD8 68 Numeric keypad 8 key  
	{ K_KP_PGUP,		K_KP_PGUP		}, // VK_NUMPAD9 69 Numeric keypad 9 key  
	{ K_KP_STAR,		K_KP_STAR		}, // VK_MULTIPLY 6A Multiply key  
	{ K_KP_PLUS, 		K_KP_PLUS 		}, // VK_ADD 6B Add key  
	{ 0,				0				}, // VK_SEPARATOR 6C Separator key  
	{ K_KP_MINUS,		K_KP_MINUS		}, // VK_SUBTRACT 6D Subtract key  
	{ K_KP_DEL,			K_KP_DEL		}, // VK_DECIMAL 6E Decimal key  
	{ K_KP_SLASH,		K_KP_SLASH		}, // VK_DIVIDE 6F Divide key  
	{ K_F1,				K_F1			}, // VK_F1 70 F1 key  
	{ K_F2,				K_F2			}, // VK_F2 71 F2 key  
	{ K_F3,				K_F3			}, // VK_F3 72 F3 key  
	{ K_F4,				K_F4			}, // VK_F4 73 F4 key  
	{ K_F5,				K_F5			}, // VK_F5 74 F5 key  
	{ K_F6,				K_F6			}, // VK_F6 75 F6 key  
	{ K_F7,				K_F7			}, // VK_F7 76 F7 key  
	{ K_F8,				K_F8			}, // VK_F8 77 F8 key  
	{ K_F9,				K_F9			}, // VK_F9 78 F9 key  
	{ K_F10,			K_F10			}, // VK_F10 79 F10 key  
	{ K_F11,			K_F11			}, // VK_F11 7A F11 key  
	{ K_F12,			K_F12			}, // VK_F12 7B F12 key  
	{ 0,				0				}, // VK_F13 7C F13 key  
	{ 0,				0				}, // VK_F14 7D F14 key  
	{ 0,				0				}, // VK_F15 7E F15 key  
	{ 0,				0				}, // VK_F16 7F F16 key  
	{ 0,				0				}, // VK_F17 80H F17 key  
	{ 0,				0				}, // VK_F18 81H F18 key  
	{ 0,				0				}, // VK_F19 82H F19 key  
	{ 0,				0				}, // VK_F20 83H F20 key  
	{ 0,				0				}, // VK_F21 84H F21 key  
	{ 0,				0				}, // VK_F22 85H F22 key  
	{ 0,				0				}, // VK_F23 86H F23 key  
	{ 0,				0				}, // VK_F24 87H F24 key  
	{ 0,				0				}, // 88 Unassigned
	{ 0,				0				}, // 89 Unassigned
	{ 0,				0				}, // 8A Unassigned
	{ 0,				0				}, // 8B Unassigned
	{ 0,				0				}, // 8C Unassigned
	{ 0,				0				}, // 8D Unassigned
	{ 0,				0				}, // 8E Unassigned
	{ 0,				0				}, // 8F Unassigned
	{ K_KP_NUMLOCK,		K_KP_NUMLOCK	}, // VK_NUMLOCK 90 NUM LOCK key  
	{ 0,				0				}  // VK_SCROLL 91 
};
static byte extendedVirtualKeyConvert[21][2] =
{
	{ K_F15,			K_ASCII_181		},
	{ K_KP_ENTER,		K_ASCII_191		},
	{ K_LAST_KEY,		K_ASCII_223		},
	{ 0xE0,				K_ASCII_224		},
	{ 0xE1,				K_ASCII_225		},
	{ 0xE4,				K_ASCII_228		},
	{ 0xE5,				K_ASCII_229		},
	{ 0xE6,				K_ASCII_230		},
	{ 0xE7,				K_ASCII_231		},
	{ 0xE8,				K_ASCII_232		},
	{ 0xE9,				K_ASCII_233		},
	{ 0xEC,				K_ASCII_236		},
	{ 0xF1,				K_ASCII_241		},
	{ 0xF2,				K_ASCII_242		},
	{ 0xF3,				K_ASCII_243		},
	{ 0xF6,				K_ASCII_246		},
	{ 0xF8,				K_ASCII_248		},
	{ 0xF9,				K_ASCII_249		},
	{ 0xFA,				K_ASCII_250		},
	{ 0xFC,				K_ASCII_252		},
	{ 0,				0				},
};

static bool IsNumLockAffectedVK(unsigned int wParam)
{
	return wParam >= 0x60 && wParam <= 0x69 || wParam == 110;
}

static unsigned int AdustKeyForNumericKeypad(unsigned int key, unsigned int wParam, unsigned int extended)
{
	if ((clientUIActives[0].keyCatchers & 0x11) == 0)
		return key;
	if (extended)
		return key;
	return !IsNumLockAffectedVK(wParam) ? key : 0;
}

static unsigned char MapKey(int key, unsigned int wParam)
{
	unsigned int result;
	int i;

	if (((key >> 8) & 0xFF)/*BYTE2*/ == ')')
		return '~';

	result = 0;
	if (wParam && wParam <= 0x91)
		result = AdustKeyForNumericKeypad(virtualKeyConvert[wParam][(key >> 24) & 1], wParam, (key >> 24) & 1);

	if (!result)
	{
		result = (unsigned __int8)MapVirtualKeyA(wParam, 2u);
		if (result > 0x7F)
		{
			for (i = 0; extendedVirtualKeyConvert[i][0]; ++i)
			{
				if (result == extendedVirtualKeyConvert[i][0])
				{
					result = extendedVirtualKeyConvert[i][1];
					break;
				}
			}
		}
	}

	iassert((result >= 0) && (result < 256));
	return result;
}

void __cdecl VID_AppActivate(unsigned int activeState, int minimize)
{
	BOOL v2; // [esp+0h] [ebp-8h]

	g_wv.isMinimized = minimize;
	Key_ClearStates(0);
	v2 = activeState && !g_wv.isMinimized;
	g_wv.activeApp = v2;
	if (v2)
		Com_TouchMemory();
	IN_Activate(g_wv.activeApp);
}

const dvar_t *r_autopriority;
LRESULT WINAPI MainWndProc(
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	byte code;

	int buttons;
	HMONITOR monitorHandle;
	tagMONITORINFO monitorInfo;
	tagRECT rc;

	int xPos, yPos;
	tagRECT r;
	int style;

	if (uMsg == MSH_MOUSEWHEEL)
	{
		if (((int)wParam) > 0)
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL);
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL);
		}
		else
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL);
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_MOUSEFIRST:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		buttons = (wParam & 1) != 0;
		if ((wParam & 2) != 0)
			buttons |= 2u;
		if ((wParam & 0x10) != 0)
			buttons |= 4u;
		if ((wParam & 0x20) != 0)
			buttons |= 8u;
		if ((wParam & 0x40) != 0)
			buttons |= 0x10u;
		IN_MouseEvent(buttons);
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	case WM_MOUSEWHEEL:
		//
		//
		// this chunk of code theoretically only works under NT4 and Win98
		// since this message doesn't exist under Win95
		//
		if ((short)HIWORD(wParam) > 0)
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL);
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL);
		}
		else
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL);
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL);
		}
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	case WM_POWERBROADCAST:
		if (wParam > 1)
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		return 0x424D5144;
		break;
	case WM_SYSCOMMAND:
		if (wParam == 61760)
			return 0;
		else
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DISPLAYCHANGE:
		monitorHandle = MonitorFromWindow(hWnd, 2u);
		monitorInfo.cbSize = sizeof(tagMONITORINFO);
		GetMonitorInfoA(monitorHandle, &monitorInfo);
		GetWindowRect(hWnd, &rc);
		if (rc.right - rc.left == monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left
			&& rc.bottom - rc.top == monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top)
		{
			MoveWindow(
				hWnd,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				rc.right - rc.left,
				rc.bottom - rc.top,
				1);
		}
		IN_RecenterMouse();
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	case WM_KEYDOWN:
		code = MapKey(lParam, wParam);
		if (code)
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, code, 1, 0, 0);
		return 0;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		code = MapKey(lParam, wParam);
		if (code)
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, code, 0, 0, 0);
		}
		return 0;
		break;
	case WM_CHAR:
		Sys_QueEvent(g_wv.sysMsgTime, SE_CHAR, wParam, 0, 0, 0);
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SYSKEYDOWN:
		if (wParam == 13)
		{
			if (client_state == 7)
			{
				return 0;
			}
			else
			{
				if (r_fullscreen && Dvar_GetInt("developer"))
				{
					Dvar_SetBool(r_fullscreen, !r_fullscreen->current.enabled);
					Cbuf_AddText(0, "vid_restart\n");
				}
				return 0;
			}
		}
		else
		{
			code = MapKey(lParam, wParam);
			if (code)
				Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, code, 1, 0, 0);
			return 0;
		}
		break;
	case WM_CLOSE:
		Key_RemoveCatcher(0, -3);
		Com_Quit_f();
		break;
	case WM_CREATE:
		g_wv.hWnd = hWnd;

		SND_SetHWND(hWnd);

		iassert(r_reflectionProbeGenerate);
		iassert(r_fullscreen);

		if (r_reflectionProbeGenerate->current.enabled && r_fullscreen->current.enabled)
		{
			Dvar_SetBool(r_fullscreen, 0);
			Cbuf_AddText(0, "vid_restart\n");
		}
		r_autopriority = Dvar_RegisterBool(
			"r_autopriority",
			0,
			1u,
			"Automatically set the priority of the windows process when the game is minimized");
		MSH_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;

	case WM_DESTROY:
		g_wv.hWnd = NULL;
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;

	case WM_MOVE:
		if (r_fullscreen->current.enabled)
		{
			IN_ActivateMouse(0);
		}
		else
		{
			xPos = (__int16)lParam;
			yPos = (int)HIWORD(lParam);
			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;
			style = GetWindowLongA(hWnd, -16);
			AdjustWindowRect(&r, style, 0);
			Dvar_SetInt(vid_xpos, r.left + xPos);
			Dvar_SetInt(vid_ypos, r.top + yPos);
			Dvar_ClearModified((dvar_s*)vid_xpos);
			Dvar_ClearModified((dvar_s*)vid_ypos);
			if (g_wv.activeApp)
				IN_Activate(1);
		}
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	case WM_ACTIVATE:
		VID_AppActivate((unsigned __int16)wParam, HIWORD(wParam));
		break;
	case WM_SETFOCUS:
		if (!r_autopriority->current.enabled)
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		SetPriorityClass(GetCurrentProcess(), 0x20u);
		break;
	case WM_KILLFOCUS:
		if (!r_autopriority->current.enabled)
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		SetPriorityClass(GetCurrentProcess(), 0x40u);
		break;
	default:
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	iassert(0); // LWSS: shouldn't reach.
}