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
//#include "../qcommon/stringed_ingame.h"

static void PrintWorkingDir()
{
	char cwd[260];

	_getcwd(cwd, 256);
	Com_Printf(16, "Working directory: %s\n", cwd);
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