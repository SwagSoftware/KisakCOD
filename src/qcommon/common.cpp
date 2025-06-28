#include "qcommon.h"

#include "cmd.h"
#include "threads.h"
#include "../win32/win_local.h"

#include <universal/com_memory.h>
#include <client/client.h>
#include <win32/win_net_debug.h>
#include <bgame/bg_local.h>
#include <script/scr_debugger.h>
#include <client_mp/client_mp.h>
#include "com_bsp.h"
#include "com_playerprofile.h"
#include <universal/com_files.h>
#include <stringed/stringed_hooks.h>
#include "files.h"
#include <server_mp/server.h>
#include <gfx_d3d/r_rendercmds.h>
#include <script/scr_vm.h>
#include <gfx_d3d/r_init.h>
#include <EffectsCore/fx_system.h>
#include <database/database.h>
#include <universal/com_constantconfigstrings.h>
#include <universal/physicalmemory.h>
#include <win32/win_storage.h>
#include <buildnumber_mp.h>
#include <win32/win_net.h>
#include <xanim/dobj.h>
#include <sound/snd_local.h>
#include <game_mp/g_public_mp.h>
#include <script/scr_animtree.h>
#include <gfx_d3d/r_scene.h>
#include <ragdoll/ragdoll.h>
#include <devgui/devgui.h>
#include <ui/ui.h>
#include <server/sv_game.h>
#include <gfx_d3d/r_workercmds.h>
#include <universal/com_convexhull.h>
#include <gfx_d3d/r_dvars.h>
#include "mem_track.h"
#include <universal/profile.h>

#include <setjmp.h>

int marker_common;

int com_skelTimeStamp;
unsigned int com_errorPrintsCount;
float com_timescaleValue;
//00751450       struct _iobuf* debuglogfile 82f01450     common.obj
int com_fixedConsolePosition;
int com_errorEntered;
int com_frameNumber;
int com_consoleLogOpenFailed;
int com_missingAssetOpenFailed;
int com_frameTime;

const dvar_t *com_dedicated;
const dvar_t *com_hiDef;
const dvar_t *com_animCheck;
const dvar_t *com_developer_script;
const dvar_t *dev_timescale;
const dvar_t *cl_useMapPreloading;
const dvar_t *com_maxfps;
const dvar_t *com_recommendedSet;
const dvar_t *sv_useMapPreloading;
const dvar_t *ui_errorMessage;
const dvar_t *com_introPlayed;
const dvar_t *com_wideScreen;
const dvar_t *ui_errorTitle;
const dvar_t *shortversion;
const dvar_t *com_attractmodeduration;
const dvar_t *com_attractmode;
const dvar_t *cl_paused_simple;
const dvar_t *sv_paused;
const dvar_t *com_fixedtime;
const dvar_t *fastfile_allowNoAuth;
const dvar_t *com_logfile;
const dvar_t *cl_paused;
const dvar_t *com_timescale;
const dvar_t *nextmap;
const dvar_t *version;
const dvar_t *com_sv_running;
const dvar_t *com_useConfig;
const dvar_t *com_maxFrameTime;
const dvar_t *com_statmon;
const dvar_t *com_filter_output;
const dvar_t *com_developer;

const dvar_t *sys_lockThreads;
const dvar_t *sys_smp_allowed;
const dvar_t *com_masterServerName;
const dvar_t *com_authServerName;
const dvar_t *com_masterPort;
const dvar_t *com_authPort;

const dvar_t *useFastFile;

#define MAX_NUM_ARGVS	50

int		com_argc;
char *com_argv[MAX_NUM_ARGVS + 1];

static char* rd_buffer = NULL;
static void(QDECL* rd_flush)(char*) = NULL;
static unsigned int rd_buffersize = 0;

char com_errorMessage[4096];

int com_lastFrameIndex;
int com_lastFrameTime[1];
int com_fullyInitialized;
float com_codeTimeScale;
int timeClientFrame;

int logfile;

int com_numConsoleLines;
char *com_consoleLines[32];

int weaponInfoSource;

errorParm_t errorcode;
int lastErrorTime;
int errorCount;
int com_safemode;

int opening_qconsole;

const char *noticeErrors[10] =
{
  "EXE_SERVER_DISCONNECTED",
  "EXE_DISCONNECTED",
  "EXE_SERVERISFULL",
  "XBOXLIVE_SIGNEDOUTOFLIVE",
  "XBOXLIVE_CANTJOINSESSION",
  "XBOXLIVE_MPNOTALLOWED",
  "XBOXLIVE_MUSTLOGIN",
  "MENU_RESETCUSTOMCLASSES",
  "XBOXLIVE_NETCONNECTION",
  ""
}; // idb

void QDECL Com_PrintMessage(int channel, const char* msg, int error)
{
	// LWSS: Punkbuster stuff
	//PbCaptureConsoleOutput(msg, 4096);

    // always print to stdout console
    fprintf(stderr, "%s", msg);

	if (rd_buffer)
	{
		if (channel != 6)
		{
			Sys_EnterCriticalSection(CRITSECT_RD_BUFFER);
			if (strlen(rd_buffer) + strlen(msg) > rd_buffersize - 1)
			{
				rd_flush(rd_buffer);
				*rd_buffer = 0;
			}
			I_strncat(rd_buffer, rd_buffersize, msg);
			Sys_LeaveCriticalSection(CRITSECT_RD_BUFFER);
		}
	}
	else
	{
		if (channel != 6 && com_dedicated && !com_dedicated->current.integer)
		{
			if (channel == 2 || channel == 3 || channel == 4)
				MyAssertHandler(".\\qcommon\\common.cpp", 625, 0, "%s", "!Con_IsNotifyChannel( channel )");
			CL_ConsolePrint(0, channel, msg, 0, 0, 32 * error);
		}
		if (*msg == 94 && msg[1])
			msg += 2;
		if (channel != 6
			&& (!com_filter_output || !com_filter_output->current.enabled
				|| Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 3)))
		{
			Sys_Print(msg);
		}
		if (channel != 7 && com_logfile && com_logfile->current.integer)
			Com_LogPrintMessage(channel, msg);
	}

}

void __cdecl Debug_Frame(int localClientNum)
{
    // KISAKTODO
#if 0
    unsigned int v1; // edx
    int lastFrameIndex; // [esp+0h] [ebp-18h]
    int newEvent; // [esp+4h] [ebp-14h]
    int msec; // [esp+8h] [ebp-10h]
    bgs_t *oldBgs; // [esp+Ch] [ebp-Ch]
    int minMsec; // [esp+10h] [ebp-8h]
    int newEvent2; // [esp+14h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 4297, 0, "%s", "Sys_IsMainThread()");
    oldBgs = bgs;
    bgs = 0;
    IN_Frame();
    if (Sys_IsRemoteDebugClient())
    {
        minMsec = 33;
    }
    else
    {
        minMsec = 1;
        if (com_maxfps->current.integer > 0 && !com_dedicated->current.integer)
        {
            minMsec = 1000 / com_maxfps->current.integer;
            if (minMsec < 0)
                MyAssertHandler(".\\qcommon\\common.cpp", 4319, 0, "%s", "minMsec >= 0");
            if (!minMsec)
                minMsec = 1;
        }
    }
    v1 = com_lastFrameIndex & 0x80000000;
    if (com_lastFrameIndex < 0)
        v1 = 0;
    lastFrameIndex = v1;
    ++com_lastFrameIndex;
    while (1)
    {
        newEvent = Scr_UpdateDebugSocket();
        newEvent2 = Debug_EventLoop(localClientNum);
        com_frameTime = Sys_Milliseconds();
        if (com_lastFrameTime[lastFrameIndex] > com_frameTime)
            com_lastFrameTime[lastFrameIndex] = com_frameTime;
        msec = com_frameTime - com_lastFrameTime[lastFrameIndex];
        if (newEvent || newEvent2 || msec >= minMsec)
            break;
        NET_Sleep(1u);
    }
    com_lastFrameTime[lastFrameIndex] = com_frameTime;
    cls.realFrametime = Com_ModifyMsec(msec);
    cls.realtime += cls.frametime;
    CL_UpdateSound();
    if (Scr_CanDrawScript())
    {
        R_BeginDebugFrame();
        Scr_DrawScript();
        R_EndDebugFrame();
    }
    if (bgs)
        MyAssertHandler(".\\qcommon\\common.cpp", 4370, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
    bgs = oldBgs;
#endif
}

void QDECL Com_Printf(int channel, const char* fmt, ...)
{
	char string[4100];
	va_list va;

	va_start(va, fmt);
	_vsnprintf(string, 0x1000u, fmt, va);
	string[4095] = 0;
	Com_PrintMessage(channel, string, 0);
}

typedef enum
{
	PRE_READ,									// prefetch assuming that buffer is used for reading only
	PRE_WRITE,									// prefetch assuming that buffer is used for writing only
	PRE_READ_WRITE								// prefetch assuming that buffer is used for both reading and writing
} e_prefetch;

#define EMMS_INSTRUCTION	__asm emms

void _copyDWord(unsigned int* dest, const unsigned int constant, const unsigned int count) 
{
    for (unsigned i = 0; i < count; i++)
        dest[i] = constant;
}

qboolean Com_Memcmp(const void* src0, const void* src1, const unsigned int count)
{
	unsigned int i;
	// MMX version anyone?

	if (count >= 16)
	{
		unsigned int* dw = (unsigned int*)(src0);
		unsigned int* sw = (unsigned int*)(src1);

		unsigned int nm2 = count / 16;
		for (i = 0; i < nm2; i += 4)
		{
			unsigned int tmp = (dw[i + 0] - sw[i + 0]) | (dw[i + 1] - sw[i + 1]) |
				(dw[i + 2] - sw[i + 2]) | (dw[i + 3] - sw[i + 3]);
			if (tmp)
				return qfalse;
		}
	}
	if (count & 15)
	{
		byte* d = (byte*)src0;
		byte* s = (byte*)src1;
		for (i = count & 0xfffffff0; i < count; i++)
			if (d[i] != s[i])
				return qfalse;
	}

	return qtrue;
}

void Com_Prefetch(const void* s, const unsigned int bytes, e_prefetch type)
{
	// write buffer prefetching is performed only if
	// the processor benefits from it. Read and read/write
	// prefetching is always performed.

	switch (type)
	{
	case PRE_WRITE: break;
	case PRE_READ:
	case PRE_READ_WRITE:
#ifdef _M_X686
		__asm
		{
			mov		ebx, s
			mov		ecx, bytes
			cmp		ecx, 4096				// clamp to 4kB
			jle		skipClamp
			mov		ecx, 4096
			skipClamp:
			add		ecx, 0x1f
			shr		ecx, 5					// number of cache lines
			jz		skip
			jmp		loopie

			align 16
			loopie:	test	byte ptr[ebx], al
			add		ebx, 32
			dec		ecx
			jnz		loopie
			skip :
		}
#else
        // DI: lol
        PreFetchCacheLine(PF_NON_TEMPORAL_LEVEL_ALL, s);
#endif

		break;
	}
}

void __cdecl Com_LogPrintMessage(int channel, const char* msg)
{
    Sys_EnterCriticalSection(CRITSECT_CONSOLE);
    if (FS_Initialized())
    {
        if (!logfile)
            Com_OpenLogFile();
        if (logfile)
        {
            FS_WriteLog(msg, strlen(msg), logfile);
            if (com_logfile->current.integer > 1)
                FS_Flush(logfile);
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
}

void Com_OpenLogFile()
{
    const char* BuildNumber; // eax
    const char* v1; // [esp-4h] [ebp-14h]
    __int64 aclock; // [esp+0h] [ebp-10h] BYREF
    tm* newtime; // [esp+Ch] [ebp-4h]

    if (Sys_IsMainThread() && !opening_qconsole)
    {
        opening_qconsole = 1;
        _time64(&aclock);
        newtime = _localtime64(&aclock);
        logfile = FS_FOpenTextFileWrite("console_mp.log");
        com_consoleLogOpenFailed = logfile == 0;
        v1 = asctime(newtime);
        BuildNumber = getBuildNumber();
        Com_Printf(16, "Build %s\nlogfile opened on %s\n", BuildNumber, v1);
        opening_qconsole = 0;
    }
}

void Com_DPrintf(int channel, const char* fmt, ...)
{
    char string[4100]; // [esp+4h] [ebp-1008h] BYREF
    va_list va; // [esp+101Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    if (com_developer)
    {
        if (com_developer->current.integer)
        {
            _vsnprintf(string, 0x1000u, fmt, va);
            string[4095] = 0;
            Com_Printf(channel, "%s", string);
        }
    }
}

void Com_PrintError(int channel, const char *fmt, ...)
{
    char dest[4096]; // [esp+14h] [ebp-1008h] BYREF
    int v3; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    if (I_stristr(fmt, "error"))
        I_strncpyz(dest, "^1", 4096);
    else
        I_strncpyz(dest, "^1Error: ", 4096);
    v3 = &dest[strlen(dest) + 1] - &dest[1];
    _vsnprintf(&dest[v3], 4096 - v3, fmt, va);
    dest[4095] = 0;
    ++com_errorPrintsCount;
    Com_PrintMessage(channel, dest, 3);
}

void Com_PrintWarning(int channel, const char *fmt, ...)
{
    char dest[4096]; // [esp+14h] [ebp-1008h] BYREF
    int v3; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    I_strncpyz(dest, "^3", 4096);
    v3 = &dest[strlen(dest) + 1] - &dest[1];
    _vsnprintf(&dest[v3], 4096 - v3, fmt, va);
    dest[4095] = 0;
    Com_PrintMessage(channel, dest, 2);
}

void __cdecl Com_Shutdown(const char* finalmsg)
{
    Com_ShutdownInternal(finalmsg);
    if (!com_dedicated->current.integer)
    {
        CL_InitRenderer();
        Com_AssetLoadUI();
    }
}

void __cdecl CL_ShutdownDemo()
{
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    clc = CL_GetLocalClientConnection(0);
    if (clc->demofile)
    {
        FS_FCloseFile(clc->demofile);
        clc->demofile = 0;
        clc->demoplaying = 0;
        clc->demorecording = 0;
    }
}

void __cdecl Com_ShutdownInternal(const char* finalmsg)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        CL_Disconnect(localClientNum);
    KISAK_NULLSUB();
    CL_ShutdownAll();
    CL_ShutdownDemo();
    FakeLag_Shutdown();
    SV_Shutdown(finalmsg);
    Com_Restart();
}

void __cdecl Com_SetLocalizedErrorMessage(char* localizedErrorMessage, const char* titleToken)
{
    char* translation; // [esp+0h] [ebp-4h]

    ui_errorMessage = Dvar_RegisterString("com_errorMessage", (char*)"", 0x40u, "Most recent error message");
    ui_errorTitle = Dvar_RegisterString(
        "com_errorTitle",
        (char*)"",
        0x40u,
        "Title of the most recent error message");
    translation = SEH_LocalizeTextMessage(titleToken, "error message", LOCMSG_NOERR);
    if (translation)
        Dvar_SetString((dvar_s*)ui_errorTitle, translation);
    else
        Dvar_SetString((dvar_s*)ui_errorTitle, (char*)"");
    Dvar_SetString((dvar_s*)ui_errorMessage, localizedErrorMessage);
    if (com_errorMessage != localizedErrorMessage) I_strncpyz(com_errorMessage, localizedErrorMessage, 4096);
}

void __cdecl Com_SetErrorMessage(char* errorMessage)
{
    char* translation; // [esp+0h] [ebp-8h]
    const char* title; // [esp+4h] [ebp-4h]

    if (!errorMessage)
        MyAssertHandler(".\\qcommon\\common.cpp", 977, 0, "%s", "errorMessage");
    if (!*errorMessage)
        MyAssertHandler(".\\qcommon\\common.cpp", 978, 0, "%s", "errorMessage[0]");
    if (errorcode == ERR_SERVERDISCONNECT || Com_ErrorIsNotice(errorMessage))
        title = "MENU_NOTICE";
    else
        title = "MENU_ERROR";
    translation = SEH_LocalizeTextMessage(errorMessage, "error message", LOCMSG_NOERR);
    if (!translation)
        translation = errorMessage;
    Com_SetLocalizedErrorMessage(translation, title);
}

char __cdecl Com_ErrorIsNotice(const char* errorMessage)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; *noticeErrors[i]; ++i)
    {
        if (!I_stricmp(noticeErrors[i], errorMessage))
            return 1;
    }
    return 0;
}

void __cdecl Com_PrintStackTrace()
{
    // KISAKTODO
   //DoStackTrace(g_stackTrace, 1);
   //Com_Printf(16, "STACKBEGIN -------------------------------------------------------------------\n");
   //Com_Printf(16, g_stackTrace);
   //Com_Printf(16, "STACKEND ---------------------------------------------------------------------\n");
}

void __cdecl  Com_ErrorAbort()
{
    Sys_Error("%s", com_errorMessage);
}

void Com_Error(errorParm_t code, const char* fmt, ...)
{
    jmp_buf * Value; // eax
    va_list va; // [esp+18h] [ebp+10h] BYREF

    va_start(va, fmt);
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    if ((unsigned int)code <= ERR_DROP)
        Com_PrintStackTrace();
    if (com_errorEntered)
        Sys_Error("recursive error after: %s", com_errorMessage);
    com_errorEntered = 1;
    _vsnprintf(com_errorMessage, 0x1000u, fmt, va);
    com_errorMessage[4095] = 0;
    if (!com_errorMessage[0])
        MyAssertHandler(".\\qcommon\\common.cpp", 1343, 0, "%s", "com_errorMessage[0]");
    if (code == ERR_SCRIPT || code == ERR_LOCALIZATION)
    {
        if (!com_fixedConsolePosition)
        {
            com_fixedConsolePosition = 1;
            CL_ConsoleFixPosition();
        }
        if (cls.uiStarted)
        {
            Com_SetErrorMessage(com_errorMessage);
            if (Sys_IsMainThread())
                UI_SetActiveMenu(0, (uiMenuCommand_t)1);
        }
        if (cls.uiStarted)
        {
            com_errorEntered = 0;
            Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
            return;
        }
        code = ERR_DROP;
    LABEL_27:
        if (!com_errorEntered)
            MyAssertHandler(".\\qcommon\\common.cpp", 1456, 0, "%s", "com_errorEntered");
        errorcode = code;
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
        Value = (jmp_buf *)Sys_GetValue(2);
        longjmp(*Value, -1);
    }
    if (code == ERR_SCRIPT_DROP)
    {
        com_fixedConsolePosition = 1;
        CL_ConsoleFixPosition();
        code = ERR_DROP;
        goto LABEL_27;
    }
    if (code != ERR_MAPLOADERRORSUMMARY)
    {
        com_fixedConsolePosition = 0;
        goto LABEL_27;
    }
    com_fixedConsolePosition = 1;
    CL_ConsoleFixPosition();
    if (cls.uiStarted && Sys_IsMainThread())
    {
        Com_SetErrorMessage(com_errorMessage);
        UI_SetActiveMenu(0, (uiMenuCommand_t)5);
        com_errorEntered = 0;
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }
    else
    {
        if (!com_dedicated->current.integer)
            goto LABEL_27;
        Sys_Print((char*)"\n==========================\n");
        Sys_Print(com_errorMessage);
        Sys_Print((char*)"\n==========================\n");
        com_errorEntered = 0;
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }
}

void __cdecl  Com_Quit_f()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    Com_Printf(0, "quitting...\n");
    R_PopRemoteScreenUpdate();
    Com_SyncThreads();
    Scr_Cleanup();
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    GScr_Shutdown();
    if (!com_errorEntered)
    {
        Com_ClearTempMemory();
        Sys_DestroySplashWindow();
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
            CL_Shutdown(localClientNum);
        FakeLag_Shutdown();
        SV_Shutdown("EXE_SERVERQUIT");
        CL_ShutdownRef();
        Com_Close();
        Com_CloseLogfiles();
        FS_Shutdown();
        FS_ShutdownServerIwdNames();
        FS_ShutdownServerReferencedIwds();
        FS_ShutdownServerReferencedFFs();
    }
    Sys_Quit();
}

void Com_ClearTempMemory()
{
    Hunk_ClearTempMemory();
    Hunk_ClearTempMemoryHigh();
}

void __cdecl Com_ParseCommandLine(char* commandLine)
{
    if (!commandLine)
        MyAssertHandler(".\\qcommon\\common.cpp", 1729, 0, "%s", "commandLine");
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;
    while (*commandLine)
    {
        if (*commandLine == 43 || *commandLine == 10)
        {
            if (com_numConsoleLines == 32)
                return;
            com_consoleLines[com_numConsoleLines++] = commandLine + 1;
            *commandLine = 0;
        }
        ++commandLine;
    }
}

int __cdecl Com_SafeMode()
{
    const char* v0; // eax
    const char* v1; // eax
    bool v3; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < com_numConsoleLines; ++i)
    {
        Cmd_TokenizeString(com_consoleLines[i]);
        v0 = Cmd_Argv(0);
        v3 = 1;
        if (I_stricmp(v0, "safe"))
        {
            v1 = Cmd_Argv(0);
            if (I_stricmp(v1, "dvar_restart"))
                v3 = 0;
        }
        Cmd_EndTokenizedString();
        if (v3)
        {
            *com_consoleLines[i] = 0;
            return 1;
        }
    }
    return com_safemode;
}

void __cdecl Com_ForceSafeMode()
{
    com_safemode = 1;
}

void __cdecl Com_StartupVariable(const char* match)
{
    const char* v1; // eax
    const char* v2; // eax
    int lineIndex; // [esp+14h] [ebp-4h]

    for (lineIndex = 0; lineIndex < com_numConsoleLines; ++lineIndex)
    {
        Cmd_TokenizeString(com_consoleLines[lineIndex]);
        if (!match || !strcmp(Cmd_Argv(1), match))
        {
            v1 = Cmd_Argv(0);
            if (I_stricmp(v1, "set"))
            {
                v2 = Cmd_Argv(0);
                if (!I_stricmp(v2, "seta"))
                    Dvar_SetA_f();
            }
            else
            {
                Dvar_Set_f();
            }
        }
        Cmd_EndTokenizedString();
    }
}

void __cdecl Info_Print(const char* s)
{
    unsigned __int8* o; // [esp+0h] [ebp-410h]
    char* oa; // [esp+0h] [ebp-410h]
    char key[512]; // [esp+8h] [ebp-408h] BYREF
    char value[516]; // [esp+208h] [ebp-208h] BYREF

    if (*s == 92)
        ++s;
    while (*s)
    {
        o = (unsigned __int8*)key;
        while (*s && *s != 92)
            *o++ = *s++;
        if (o - (unsigned __int8*)key >= 20)
        {
            *o = 0;
        }
        else
        {
            memset(o, 0x20u, 20 - (o - (unsigned __int8*)key));
            key[20] = 0;
        }
        Com_Printf(0, "%s", key);
        if (!*s)
        {
            Com_Printf(16, "MISSING VALUE\n");
            return;
        }
        oa = value;
        ++s;
        while (*s && *s != 92)
            *oa++ = *s++;
        *oa = 0;
        if (*s)
            ++s;
        Com_Printf(0, "%s\n", value);
    }
}

unsigned int* __cdecl Com_AllocEvent(int size)
{
    return (unsigned int *)Z_Malloc(size, "Com_AllocEvent", 10);
}

unsigned __int8 clientCommonMsgBuf[131072];
void __cdecl Com_ClientPacketEvent()
{
    msg_t netmsg; // [esp+4Ch] [ebp-40h] BYREF
    netadr_t adr; // [esp+74h] [ebp-18h] BYREF

    iassert(Sys_IsMainThread());

    PROF_SCOPED("Com_ClientPacketEvent");

    MSG_Init(&netmsg, clientCommonMsgBuf, 0x20000);
    Com_PacketEventLoop(NS_CLIENT1, &netmsg);

    if (!com_sv_running->current.enabled)
    {
        while (NET_GetClientPacket(&adr, &netmsg))
            Com_DispatchClientPacketEvent(adr, &netmsg);
    }
}

void __cdecl Com_PacketEventLoop(netsrc_t client, msg_t* netmsg)
{
    netadr_t adr; // [esp+0h] [ebp-18h] BYREF

    while (NET_GetLoopPacket(client, &adr, netmsg))
    {
        CL_PacketEvent(client, adr, netmsg, Sys_Milliseconds());
    }
}

void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t* netmsg)
{
    CL_PacketEvent(NS_CLIENT1, adr, netmsg, Sys_Milliseconds());
}

unsigned __int8 serverCommonMsgBuf[0x20000];
void __cdecl Com_ServerPacketEvent()
{
    msg_t netmsg; // [esp+30h] [ebp-40h] BYREF
    netadr_t adr; // [esp+58h] [ebp-18h] BYREF

    PROF_SCOPED("Com_ServerPacketEvent");

    iassert(Sys_IsMainThread());

    MSG_Init(&netmsg, serverCommonMsgBuf, 0x20000);

    if (com_sv_running->current.enabled)
    {
        while (NET_GetServerPacket(&adr, &netmsg))
            SV_PacketEvent(adr, &netmsg);
    }
    while (NET_GetLoopPacket(NS_SERVER, &adr, &netmsg))
    {
        if (com_sv_running->current.enabled)
            SV_PacketEvent(adr, &netmsg);
    }
}

void __cdecl Com_EventLoop()
{
    sysEvent_t result; // [esp+4h] [ebp-48h] BYREF
    sysEvent_t v1; // [esp+1Ch] [ebp-30h]
    sysEvent_t ev; // [esp+34h] [ebp-18h]

    while (1)
    {
        v1 = *Sys_GetEvent(&result);
        ev = v1;
        switch (v1.evType)
        {
        case SE_NONE:
            iassert(!ev.evPtr);
            Com_ClientPacketEvent();
            Com_ServerPacketEvent();
            return;
        case SE_KEY:
            if (ev.evPtr)
                MyAssertHandler(".\\qcommon\\common.cpp", 2164, 0, "%s", "!ev.evPtr");
            CL_KeyEvent(0, ev.evValue, ev.evValue2, ev.evTime);
            break;
        case SE_CHAR:
            if (ev.evPtr)
                MyAssertHandler(".\\qcommon\\common.cpp", 2170, 0, "%s", "!ev.evPtr");
            CL_CharEvent(0, ev.evValue);
            break;
        case SE_CONSOLE:
            if (!ev.evPtr)
                MyAssertHandler(".\\qcommon\\common.cpp", 2184, 0, "%s", "ev.evPtr");
            Cbuf_AddText(0, (const char*)ev.evPtr);
            Com_FreeEvent((char*)ev.evPtr);
            Cbuf_AddText(0, "\n");
            break;
        default:
            if (ev.evPtr)
                MyAssertHandler(".\\qcommon\\common.cpp", 2146, 0, "%s", "!ev.evPtr");
            Com_Error(ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType);
            break;
        }
    }
}

void __cdecl Com_SetScriptSettings()
{
    bool v0; // [esp+0h] [ebp-Ch]

    v0 = com_developer->current.integer || com_logfile->current.integer;
    Scr_Settings(v0, com_developer_script->current.color[0], com_developer->current.integer);
}

void __cdecl Com_RunAutoExec(int localClientNum, int controllerIndex)
{
    Dvar_SetInAutoExec(1);
    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, (char*)"exec autoexec_dev_mp.cfg");
    Dvar_SetInAutoExec(0);
}

void __cdecl Com_ExecStartupConfigs(int localClientNum, const char* configFile)
{
    const char* v2; // eax

    Cbuf_AddText(localClientNum, "exec default_mp.cfg\n");
    Cbuf_AddText(localClientNum, "exec language.cfg\n");
    if (configFile)
    {
        v2 = va("exec %s\n", configFile);
        Cbuf_AddText(localClientNum, v2);
    }
    Cbuf_Execute(localClientNum, CL_ControllerIndexFromClientNum(localClientNum));
    Com_RunAutoExec(localClientNum, CL_ControllerIndexFromClientNum(localClientNum));
    if (Com_SafeMode())
        Cbuf_AddText(localClientNum, "exec safemode_mp.cfg\n");
    Cbuf_Execute(localClientNum, CL_ControllerIndexFromClientNum(localClientNum));
}

void __cdecl Com_Init(char* commandLine)
{
    jmp_buf* Value; // eax
    jmp_buf * v3; // eax
    jmp_buf * v4; // eax

    Value = (jmp_buf *)Sys_GetValue(2);
    //if (_setjmp3(Value, 0))
    if (_setjmp(*Value))
    {
        Sys_Error(va("Error during initialization:\n%s\n", com_errorMessage));
    }
    Com_Init_Try_Block_Function(commandLine);
    v3 = (jmp_buf *)Sys_GetValue(2);
    //if (!_setjmp3(v3, 0))
    if (!_setjmp(*v3))
        Com_AddStartupCommands();
    if (com_errorEntered)
        Com_ErrorCleanup();

    if (!com_sv_running->current.enabled && !com_dedicated->current.integer)
    {
        v4 = (jmp_buf *)Sys_GetValue(2);
        //if (_setjmp3(v4, 0))
        if (_setjmp(*v4))
        {
            Sys_Error(va("Error during initialization:\n%s\n", com_errorMessage));
        }
        if (!cls.rendererStarted)
            CL_InitRenderer();
        R_BeginRemoteScreenUpdate();
        CL_StartHunkUsers();
        R_EndRemoteScreenUpdate();
    }
}

bool shouldQuitOnError;
void __cdecl RefreshQuitOnErrorCondition()
{
    bool v0; // [esp+0h] [ebp-4h]

    if (Dvar_IsSystemActive())
    {
        v0 = Dvar_GetBool("QuitOnError") || Dvar_GetInt("r_vc_compile") == 2;
        shouldQuitOnError = v0;
    }
}

bool __cdecl QuitOnError()
{
    RefreshQuitOnErrorCondition();
    return shouldQuitOnError;
}

void Com_ErrorCleanup()
{
    int MenuScreenForError; // eax
    char v1; // [esp+3h] [ebp-1021h]
    char* v2; // [esp+8h] [ebp-101Ch]
    char* v3; // [esp+Ch] [ebp-1018h]
    char* src; // [esp+14h] [ebp-1010h]
    unsigned int v5; // [esp+18h] [ebp-100Ch]
    char finalmsg[4100]; // [esp+1Ch] [ebp-1008h] BYREF

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 1010, 0, "%s", "Sys_IsMainThread()");
    LargeLocalReset();
    R_PopRemoteScreenUpdate();
    Com_SyncThreads();
    if (!com_dedicated->current.enabled)
        R_ComErrorCleanup();
    Cmd_ComErrorCleanup();
    Dvar_SetInAutoExec(0);
    if (useFastFile->current.enabled)
        DB_Cleanup();
    Com_ClearTempMemory();
    if (!useFastFile->current.enabled)
        FX_UnregisterAll();
    if (ProfLoad_IsActive())
        ProfLoad_Deactivate();
    Dvar_SetIntByName("cl_paused", 0);
    FS_PureServerSetLoadedIwds((char*)"", (char*)"");
    SEH_UpdateLanguageInfo();
    v3 = com_errorMessage;
    v2 = finalmsg;
    do
    {
        v1 = *v3;
        *v2++ = *v3++;
    } while (v1);
    if (errorcode == ERR_DISCONNECT)
    {
        if (com_errorMessage[0])
        {
            src = SEH_LocalizeTextMessage(com_errorMessage, "error message", LOCMSG_NOERR);
            if (src)
                I_strncpyz(com_errorMessage, src, 4096);
        }
    }
    else
    {
        if (cls.uiStarted && errorcode != ERR_DROP)
        {
            MenuScreenForError = UI_GetMenuScreenForError();
            UI_SetActiveMenu(0, (uiMenuCommand_t)MenuScreenForError);
        }
        Com_SetErrorMessage(com_errorMessage);
    }
    if (fs_debug && fs_debug->current.integer == 2)
        Dvar_SetInt((dvar_s*)fs_debug, 0);
    SND_ErrorCleanup();
    Com_CleanupBsp();
    KISAK_NULLSUB();
    Com_ResetParseSessions();
    CL_FlushDebugServerData();
    CL_UpdateDebugServerData();
    FS_ResetFiles();
    if (errorcode == ERR_DROP)
        Cbuf_Init();
    v5 = Sys_Milliseconds();
    if ((int)(v5 - lastErrorTime) >= 100)
    {
        errorCount = 0;
    }
    else if (++errorCount > 3)
    {
        errorcode = ERR_FATAL;
    }
    lastErrorTime = v5;
    if (errorcode != ERR_SERVERDISCONNECT && errorcode != ERR_DROP && errorcode != ERR_DISCONNECT)
        Sys_Error("%s", com_errorMessage);
    updateScreenCalled = 0;
    if (errorcode == ERR_SERVERDISCONNECT)
    {
        Com_ShutdownInternal("EXE_DISCONNECTEDFROMOWNLISTENSERVER");
    }
    else
    {
        if (errorcode != ERR_DROP && errorcode != ERR_DISCONNECT)
            MyAssertHandler(
                ".\\qcommon\\common.cpp",
                1163,
                0,
                "%s\n\t(errorcode) = %i",
                "(errorcode == ERR_DROP || errorcode == ERR_DISCONNECT)",
                errorcode);
        if (errorcode == ERR_DROP)
        {
            Com_PrintError(16, "********************\nERROR: %s\n********************\n", com_errorMessage);
            if (cls.uiStarted && !com_fixedConsolePosition)
                CL_ConsoleFixPosition();
        }
        else
        {
            Com_Printf(16, "********************\nDisconnecting: %s\n********************\n", com_errorMessage);
        }
        Com_ShutdownInternal(finalmsg);
        if (errorcode == ERR_DROP && QuitOnError())
            Com_Quit_f();
    }
    bgs = 0;
    com_fixedConsolePosition = 0;
    NET_RestartDebug();
    com_errorEntered = 0;
}

void Com_AddStartupCommands()
{
    int v0; // eax
    int i; // [esp+0h] [ebp-414h]
    char localBuffer[1036]; // [esp+4h] [ebp-410h] BYREF

    for (i = 0; i < com_numConsoleLines; ++i)
    {
        if (!com_consoleLines[i])
            MyAssertHandler(".\\qcommon\\common.cpp", 1847, 0, "%s", "com_consoleLines[i]");
        if (*com_consoleLines[i])
        {
            Com_sprintf(localBuffer, 0x401u, "%s\n", com_consoleLines[i]);
            v0 = CL_ControllerIndexFromClientNum(0);
            Cbuf_ExecuteBuffer(0, v0, localBuffer);
        }
    }
}

cmd_function_s Com_Error_f_VAR;
cmd_function_s Com_Crash_f_VAR;
cmd_function_s Com_Freeze_f_VAR;
cmd_function_s Com_Assert_f_VAR;
cmd_function_s Com_Quit_f_VAR;
cmd_function_s Com_WriteConfig_f_VAR;
cmd_function_s Com_WriteDefaults_f_VAR;

static const char* comInitAllocName = "$init";
void __cdecl Com_Init_Try_Block_Function(char* commandLine)
{
    int v1; // eax
    const char* BuildNumber; // eax
    unsigned int v3; // eax
    int localClientNum; // [esp+10h] [ebp-Ch]
    int localClientNuma; // [esp+10h] [ebp-Ch]
    char* s; // [esp+14h] [ebp-8h]
    unsigned int initStartTime; // [esp+18h] [ebp-4h]

    Com_Printf(16, "%s %s build %s %s\n", "KisakCoD4", "1.0", "win-x86", __DATE__);
    Com_ParseCommandLine(commandLine);
    SL_Init();
    Swap_Init();
    Cbuf_Init();
    Cmd_Init();
    Com_StartupVariable(0);
    Com_InitDvars();
    CCS_InitConstantConfigStrings();
    initStartTime = 0;
    if (useFastFile->current.enabled)
    {
        PMem_Init();
        DB_SetInitializing(1);
        Com_Printf(7, "begin $init\n");
        initStartTime = Sys_Milliseconds();
        PMem_BeginAlloc(comInitAllocName, 1u);
    }
    if (useFastFile->current.enabled)
        Com_InitXAssets();
    CL_InitKeyCommands();
    FS_InitFilesystem();
    Con_InitChannels();
    LiveStorage_Init();
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        Com_StartupConfigs(localClientNum);
    v1 = CL_ControllerIndexFromClientNum(0);
    Cbuf_Execute(0, v1);
    if ((dvar_modifiedFlags & 0x20) != 0)
        Com_InitDvars();
    com_recommendedSet = Dvar_RegisterBool("com_recommendedSet", 0, 1u, "Use recommended settings");
    Com_CheckSetRecommended(0);
    Com_StartupVariable(0);
    if (!useFastFile->current.enabled)
        SEH_UpdateLanguageInfo();
    if (com_dedicated->current.integer)
        CL_InitDedicated();
    Com_InitHunkMemory();
    Hunk_InitDebugMemory();
    dvar_modifiedFlags &= ~1u;
    com_codeTimeScale = 1.0;
    ProfLoad_Init();
    if (com_developer->current.integer)
    {
        Cmd_AddCommandInternal("error", Com_Error_f, &Com_Error_f_VAR);
        Cmd_AddCommandInternal("crash", Com_Crash_f, &Com_Crash_f_VAR);
        Cmd_AddCommandInternal("freeze", Com_Freeze_f, &Com_Freeze_f_VAR);
        Cmd_AddCommandInternal("assert", Com_Assert_f, &Com_Assert_f_VAR);
    }
    Cmd_AddCommandInternal("quit", Com_Quit_f, &Com_Quit_f_VAR);
    Cmd_AddCommandInternal("writeconfig", Com_WriteConfig_f, &Com_WriteConfig_f_VAR);
    Cmd_AddCommandInternal("writedefaults", Com_WriteDefaults_f, &Com_WriteDefaults_f_VAR);
    BuildNumber = getBuildNumber();
    s = va("%s %s build %s %s", "CoD4 MP", "1.0", BuildNumber, "win-x86");
    version = Dvar_RegisterString("version", "", 0x40u, "Game version");
    Dvar_SetString(version, s);
    shortversion = Dvar_RegisterString("shortversion", "1.0", 0x44u, "Short game version");
    Sys_Init();
    Netchan_Init(__rdtsc());
    Scr_InitVariables();
    Scr_Init();
    Com_SetScriptSettings();
    XAnimInit();
    DObjInit();
    SV_Init();
    NET_Init();
    Dvar_ClearModified((dvar_s*)com_dedicated);
    if (!com_dedicated->current.integer)
    {
        KISAK_NULLSUB();
        CL_InitOnceForAllClients();
        for (localClientNuma = 0; localClientNuma < 1; ++localClientNuma)
            CL_Init(localClientNuma);
    }
    com_frameTime = Sys_Milliseconds();
    Com_StartupVariable(0);

    if (!com_dedicated->current.integer)
    {
        SND_InitDriver();
        R_InitThreads();
        KISAK_NULLSUB();
        CL_InitRenderer();
        KISAK_NULLSUB();
        if (cls.soundStarted)
            MyAssertHandler(".\\qcommon\\common.cpp", 3357, 0, "%s", "!cls.soundStarted");
        cls.soundStarted = 1;
        SND_Init();
    }

    COM_PlayIntroMovies();
    if (useFastFile->current.enabled)
    {
        PMem_EndAlloc(comInitAllocName, 1u);
        DB_SetInitializing(0);
        v3 = Sys_Milliseconds();
        Com_Printf(16, "end $init %d ms\n", v3 - initStartTime);
    }
    com_fullyInitialized = 1;
    Com_Printf(16, "--- Common Initialization Complete ---\n");
    Com_DvarDump(6, 0);
}

void __cdecl Com_Error_f()
{
    if (Cmd_Argc() <= 1)
        Com_Error(ERR_FATAL, "Testing fatal error");
    else
        Com_Error(ERR_DROP, "Testing drop error");
}

void __cdecl Com_Freeze_f()
{
    const char* v0; // eax
    unsigned int start; // [esp+4h] [ebp-Ch]
    float s; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        s = atof(v0);
        start = Sys_Milliseconds();
        while (s >= (double)(int)(Sys_Milliseconds() - start) * 0.001)
            ;
    }
    else
    {
        Com_Printf(0, "freeze <seconds>\n");
    }
}

void Com_BeginRedirect(char *buffer, int buffersize, void (*flush)(char *))
{
    if (!buffer || !buffersize || !flush)
        return;
    rd_buffer = buffer;
    rd_buffersize = buffersize;
    rd_flush = flush;

    *rd_buffer = 0;
}

void Com_EndRedirect(void)
{
    if (rd_flush) {
        rd_flush(rd_buffer);
    }

    rd_buffer = NULL;
    rd_buffersize = 0;
    rd_flush = NULL;
}


/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
void __cdecl Com_Crash_f()
{
    *(volatile int *)0 = 0x12345678;
}

void __cdecl Com_Assert_f()
{
    MyAssertHandler(".\\qcommon\\common.cpp", 2323, 0, "%s", "a");
}

void COM_PlayIntroMovies()
{
    if (!com_dedicated->current.integer && !com_introPlayed->current.enabled)
    {
        Cbuf_AddText(0, "cinematic IW_logo\n");
        Dvar_SetString((dvar_s*)nextmap, (char*)"cinematic atvi; set nextmap cinematic cod_intro");
        Dvar_SetBool((dvar_s*)com_introPlayed, 1);
    }
}

const char *s_lockThreadNames[4] = { "none", "minimal", "all" };

void Com_InitDvars()
{
#if !defined(DEDICATED) && !defined(KISAK_DEDICATED)
    com_dedicated = Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x20u, "True if this is a dedicated server");
#else
    com_dedicated = Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 2, 0x20u, "True if this is a dedicated server");
#endif

#if !defined(DEDICATED) && !defined(KISAK_DEDICATED)
    if (com_dedicated->current.integer)
        Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x40u, "True if this is a dedicated server");
#endif

    com_maxfps = Dvar_RegisterInt("com_maxfps", 85, 0, 1000, 1u, "Cap frames per second");
    useFastFile = Dvar_RegisterBool(
        "useFastFile",
        1,
        0x10u,
        "Enables loading data from fast files. Only tools can run without fast files.");
    sys_lockThreads = Dvar_RegisterEnum(
        "sys_lockThreads",
        s_lockThreadNames,
        0,
        0,
        "Prevents specified threads from changing CPUs; improves profiling and may fix some bugs, but can hurt performance");

    sys_smp_allowed = Dvar_RegisterBool("sys_smp_allowed", Sys_GetCpuCount() > 1u, 0x10u, "Allow multi-threading");
    com_masterServerName = Dvar_RegisterString(
        "masterServerName",
        "cod4master.activision.com",
        0x80u,
        "Master server name for listing public inet games");
    com_authServerName = Dvar_RegisterString(
        "authServerName",
        "cod4master.activision.com",
        0x80u,
        "Authentication server name for listing public inet games");
    com_masterPort = Dvar_RegisterInt("masterPort", 20810, 0, 0xFFFF, 0x80u, "Master server port");
    com_authPort = Dvar_RegisterInt("authPort", 20800, 0, 0xFFFF, 0x80u, "Auth server port");
    com_developer = Dvar_RegisterInt("developer", 0, 0, 2, 0, "Enable development options");
    com_developer_script = Dvar_RegisterBool("developer_script", 0, 0, "Enable developer script comments");
    com_logfile = Dvar_RegisterInt(
        "logfile",
        1,
        0,
        2,
        0,
        "Write to log file - 0 = disabled, 1 = async file write, 2 = Sync every write");
    com_statmon = Dvar_RegisterBool("com_statmon", 0, 0, "Draw stats monitor");
    com_timescale = Dvar_RegisterFloat("com_timescale", 1.0, 0.001f, 1000.0f, 0x10C8u, "Scale time of each frame");
    dev_timescale = Dvar_RegisterFloat("timescale", 1.0, 0.001f, 1000.0f, 0x88u, "Scale time of each frame");
    com_fixedtime = Dvar_RegisterInt("fixedtime", 0, 0, 1000, 0x80u, "Use a fixed time rate for each frame");
    com_maxFrameTime = Dvar_RegisterInt(
        "com_maxFrameTime",
        100,
        50,
        5000,
        0,
        "Time slows down if a frame takes longer than this many milliseconds");
    sv_paused = Dvar_RegisterInt("sv_paused", 0, 0, 2, 0x40u, "Pause the server");
    cl_paused = Dvar_RegisterInt("cl_paused", 0, 0, 2, 0x40u, "Pause the client");
    cl_paused_simple = Dvar_RegisterBool(
        "cl_paused_simple",
        0,
        0,
        "Toggling pause won't do any additional special processing if true.");
    com_sv_running = Dvar_RegisterBool("sv_running", 0, 0x40u, "Server is running");
    com_filter_output = Dvar_RegisterBool("com_filter_output", 0, 0, "Use console filters for filtering output.");
    com_introPlayed = Dvar_RegisterBool("com_introPlayed", 0, 1u, "Intro movie has been played");
    com_animCheck = Dvar_RegisterBool("com_animCheck", 0, 0, "Check anim tree");
    com_hiDef = Dvar_RegisterBool("hiDef", 1, 0x40u, "True if the game video is running in high-def.");
    com_wideScreen = Dvar_RegisterBool(
        "wideScreen",
        1,
        0x40u,
        "True if the game video is running in 16x9 aspect, false if 4x3.");
}

void __cdecl Com_StartupConfigs(int localClientNum)
{
    Com_InitPlayerProfiles(localClientNum);
}

void Com_InitXAssets()
{
    DB_InitThread();
}

void __cdecl Com_WriteDefaultsToFile(char* filename)
{
    int file = FS_FOpenFileWrite(filename);
    if (file)
    {
        FS_Printf(file, "// generated by Call of Duty, do not modify\n");
        Dvar_WriteDefaults(file);
        FS_FCloseFile(file);
    }
    else
    {
        Com_Printf(16, "Couldn't write %s.\n", filename);
    }
}

void __cdecl Com_WriteConfig_f()
{
    char* v0; // eax
    char filename[68]; // [esp+0h] [ebp-48h] BYREF

    if (Cmd_Argc() == 2)
    {
        v0 = (char*)Cmd_Argv(1);
        I_strncpyz(filename, v0, 64);
        Com_DefaultExtension(filename, 0x40u, ".cfg");
        Com_Printf(0, "Writing %s.\n", filename);
        Com_WriteConfigToFile(0, filename);
    }
    else
    {
        Com_Printf(0, "Usage: writeconfig <filename>\n");
    }
}

void __cdecl Com_WriteConfigToFile(int localClientNum, char* filename)
{
    int file = FS_FOpenFileWriteToDir(filename, (char*)"players");
    if (file)
    {
        FS_Printf(file, "// generated by Call of Duty, do not modify\n");
        FS_Printf(file, "unbindall\n");
        Key_WriteBindings(localClientNum, file);
        Dvar_WriteVariables(file);
        Con_WriteFilterConfigString(file);
        FS_FCloseFile(file);
    }
    else
    {
        Com_Printf(16, "Couldn't write %s.\n", filename);
    }
}

void __cdecl Com_WriteDefaults_f()
{
    char* v0; // eax
    char filename[68]; // [esp+0h] [ebp-48h] BYREF

    if (Cmd_Argc() == 2)
    {
        v0 = (char*)Cmd_Argv(1);
        I_strncpyz(filename, v0, 64);
        Com_DefaultExtension(filename, 0x40u, ".cfg");
        Com_Printf(0, "Writing %s.\n", filename);
        Com_WriteDefaultsToFile(filename);
    }
    else
    {
        Com_Printf(0, "Usage: writedefaults <filename>\n");
    }
}

double __cdecl Com_GetTimescaleForSnd()
{
    if (com_fixedtime->current.integer)
        return (double)com_fixedtime->current.integer;
    return (float)(com_timescale->current.value * dev_timescale->current.value);
}

void __cdecl Com_AdjustMaxFPS(int* maxFPS)
{
    int maxUserCmdsPerSecond; // [esp+0h] [ebp-4h]

    if (com_timescaleValue < 1.0)
    {
        maxUserCmdsPerSecond = (int)(com_timescaleValue * 320.0);
        if (maxUserCmdsPerSecond < 1)
            maxUserCmdsPerSecond = 1;
        if (!*maxFPS || *maxFPS > maxUserCmdsPerSecond)
            *maxFPS = maxUserCmdsPerSecond;
    }
}

void Com_DedicatedModified()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    if ((com_dedicated->flags & 0x40) == 0)
    {
        if ((com_dedicated->flags & 0x20) == 0)
            MyAssertHandler(".\\qcommon\\common.cpp", 3727, 0, "%s", "com_dedicated->flags & DVAR_LATCH");
        if (com_dedicated->latched.integer != com_dedicated->current.integer)
        {
            com_dedicated = Dvar_RegisterEnum(
                "dedicated",
                g_dedicatedEnumNames,
                0,
                0x20u,
                "True if this is a dedicated server");
            if (com_dedicated->current.integer)
                Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x40u, "True if this is a dedicated server");
            if (!com_dedicated->current.integer)
                MyAssertHandler(
                    ".\\qcommon\\common.cpp",
                    3735,
                    0,
                    "%s\n\t(com_dedicated->current.integer) = %i",
                    "(com_dedicated->current.integer != 0)",
                    com_dedicated->current.integer);
            if ((com_dedicated->flags & 0x40) == 0)
                MyAssertHandler(
                    ".\\qcommon\\common.cpp",
                    3736,
                    0,
                    "%s\n\t(com_dedicated->flags) = %i",
                    "(com_dedicated->flags & (1 << 6))",
                    com_dedicated->flags);
            Dvar_ClearModified((dvar_s*)com_dedicated);
            for (localClientNum = 0; localClientNum < 1; ++localClientNum)
                CL_Shutdown(localClientNum);
            CL_ShutdownRef();
            CL_InitDedicated();
            SV_AddDedicatedCommands();
        }
    }
}

void __cdecl Com_Frame_Try_Block_Function()
{
    unsigned int v0; // ecx
    int v1; // eax
    int v2; // eax
    float deltaTime; // [esp+4h] [ebp-78h]
    int v4; // [esp+8h] [ebp-74h]
    int lastFrameIndex; // [esp+68h] [ebp-14h]
    int msec; // [esp+6Ch] [ebp-10h]
    int mseca; // [esp+6Ch] [ebp-10h]
    int msecb; // [esp+6Ch] [ebp-10h]
    int localClientNum; // [esp+70h] [ebp-Ch]
    netsrc_t localClientNuma; // [esp+70h] [ebp-Ch]
    int minMsec; // [esp+74h] [ebp-8h]
    int maxFPS; // [esp+78h] [ebp-4h] BYREF

    iassert(cmd_args.nesting == -1);

    Com_WriteConfiguration(0);
    SetAnimCheck(com_animCheck->current.color[0]);
    minMsec = 1;
    maxFPS = com_maxfps->current.integer;
    Com_AdjustMaxFPS(&maxFPS);
    if (maxFPS > 0 && !com_dedicated->current.integer)
    {
        minMsec = 1000 / maxFPS;
        iassert(minMsec >= 0);
        if (!minMsec)
            minMsec = 1;
    }
    Win_UpdateThreadLock();
    iassert(minMsec > 0);
    v0 = com_lastFrameIndex & 0x80000000;
    if (com_lastFrameIndex < 0)
        v0 = 0;
    lastFrameIndex = v0;
    ++com_lastFrameIndex;
    if (com_dedicated->current.integer)
    {
        PROF_SCOPED("MaxFPSSpin");
        while (1)
        {
            Com_EventLoop();
            com_frameTime = Sys_Milliseconds();
            if (com_frameTime - com_lastFrameTime[lastFrameIndex] < 0)
                com_lastFrameTime[lastFrameIndex] = com_frameTime;
            msec = com_frameTime - com_lastFrameTime[lastFrameIndex];
            if (msec >= minMsec)
                break;
            NET_Sleep(1u);
        }
        com_lastFrameTime[lastFrameIndex] = com_frameTime;
    }
    else
    {
        KISAK_NULLSUB();
        PROF_SCOPED("MaxFPSSpin");
        while (1)
        {
            Com_EventLoop();
            com_frameTime = Sys_Milliseconds();
            if (com_frameTime - com_lastFrameTime[lastFrameIndex] < 0)
                com_lastFrameTime[lastFrameIndex] = com_frameTime;
            if (com_frameTime - com_lastFrameTime[lastFrameIndex] > 0)
                break;
            NET_Sleep(1u);
        }

        if (com_frameTime - com_lastFrameTime[lastFrameIndex] < minMsec)
            v4 = minMsec;
        else
            v4 = com_frameTime - com_lastFrameTime[lastFrameIndex];
        com_lastFrameTime[lastFrameIndex] += v4;
        msec = lastFrameIndex + v4;
        if (!(lastFrameIndex + v4))
            msec = 1;
    }

    Cbuf_Execute(0, CL_ControllerIndexFromClientNum(0));
    iassert(msec > 0);
    mseca = Com_ModifyMsec(msec);
    iassert(msec > 0);
    msecb = SV_Frame(mseca);
    Com_DedicatedModified();

    if (!com_dedicated->current.integer)
    {
        R_SetEndTime(com_lastFrameTime[lastFrameIndex]);
        KISAK_NULLSUB();
        CL_RunOncePerClientFrame(0, msecb);
        Com_EventLoop();
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            Cbuf_Execute(localClientNum, CL_ControllerIndexFromClientNum(localClientNum));
        }
        //KISAK_NULLSUB();
        {
            PROF_SCOPED("CL_Frame");
            for (localClientNuma = NS_CLIENT1; localClientNuma < NS_SERVER; ++localClientNuma)
                CL_Frame(localClientNuma);
        }
        dvar_modifiedFlags &= ~2u;
        Com_UpdateMenu();
        SCR_UpdateScreen();
        Ragdoll_Update(msecb);
        iassert(Sys_IsMainThread());
        KISAK_NULLSUB();
        deltaTime = (double)cls.frametime * EQUAL_EPSILON;
        DevGui_Update(0, deltaTime);
        Com_Statmon();
        R_WaitEndTime();
    }
}

void __cdecl Com_WriteConfiguration(int localClientNum)
{
    char configFile[68]; // [esp+0h] [ebp-48h] BYREF

    if (com_fullyInitialized && (dvar_modifiedFlags & 1) != 0)
    {
        dvar_modifiedFlags &= ~1u;
        if (Com_HasPlayerProfile())
        {
            Com_BuildPlayerProfilePath(configFile, 64, "config_mp.cfg");
            Com_WriteConfigToFile(localClientNum, configFile);
        }
    }
}

int __cdecl Com_ModifyMsec(int msec)
{
    double v2; // [esp+0h] [ebp-24h]
    int clampTime; // [esp+18h] [ebp-Ch]
    int originalMsec; // [esp+1Ch] [ebp-8h]
    bool useTimescale; // [esp+23h] [ebp-1h]

    originalMsec = msec;
    if (com_fixedtime->current.integer)
    {
        msec = com_fixedtime->current.integer;
        useTimescale = 1;
    }
    else if (com_timescale->current.value == 1.0 && com_codeTimeScale == 1.0 && dev_timescale->current.value == 1.0)
    {
        useTimescale = 0;
    }
    else
    {
        msec = (int)(dev_timescale->current.value * (com_codeTimeScale * (com_timescale->current.value * (double)msec)));
        useTimescale = 1;
    }
    if (msec < 1)
        msec = 1;
    if (com_dedicated->current.integer)
    {
        if (msec > 500 && msec < 500000)
            Com_PrintWarning(16, "Hitch warning: %i msec frame time\n", msec);
        clampTime = 5000;
    }
    else if (com_sv_running->current.enabled)
    {
        clampTime = com_maxFrameTime->current.integer;
    }
    else
    {
        clampTime = 5000;
    }
    if (msec > clampTime)
        msec = clampTime;
    if (useTimescale && originalMsec)
        v2 = (double)msec / (double)originalMsec;
    else
        v2 = 1.0;
    com_timescaleValue = v2;
    return msec;
}

void Com_Statmon()
{
    int timePrevFrame; // [esp+0h] [ebp-4h]

    if (com_statmon->current.enabled)
    {
        if (com_fileAccessed)
        {
            StatMon_Warning(1, 3000, "code_warning_file");
            com_fileAccessed = 0;
        }
        timePrevFrame = timeClientFrame;
        timeClientFrame = Sys_Milliseconds();
        if (com_statmon->current.enabled)
        {
            if (timeClientFrame - timePrevFrame > 33 && timePrevFrame)
                StatMon_Warning(0, 3000, "code_warning_fps");
            if (sv.serverFrameTimeMax > 50.0)
                StatMon_Warning(6, 3000, "code_warning_serverfps");
        }
    }
}

int Com_UpdateMenu()
{
    int result; // eax
    uiMenuCommand_t MenuScreen; // eax
    connstate_t clcState; // [esp+4h] [ebp-4h]

    clcState = clientUIActives[0].connectionState;
    result = UI_IsFullscreen(0);
    if (!result && clcState == CA_DISCONNECTED)
    {
        MenuScreen = (uiMenuCommand_t)UI_GetMenuScreen();
        return UI_SetActiveMenu(0, MenuScreen);
    }
    return result;
}

void __cdecl Com_AssetLoadUI()
{
    XZoneInfo zoneInfo; // [esp+0h] [ebp-10h] BYREF

    if (useFastFile->current.enabled)
    {
        zoneInfo.name = "ui_mp";
        zoneInfo.allocFlags = 8;
        zoneInfo.freeFlags = 104;
        DB_LoadXAssets(&zoneInfo, 1u, 0);
    }
    UI_SetMap((char*)"", (char*)"");
    R_BeginRemoteScreenUpdate();
    CL_StartHunkUsers();
    R_EndRemoteScreenUpdate();
}

void __cdecl Com_CheckSyncFrame()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 4195, 0, "%s", "Sys_IsMainThread()");
    DB_Update();
}

void __cdecl Com_Frame()
{
#ifdef TRACY_ENABLE
    TracyCFrameMarkStart("Com_Frame");
#endif
    void* Value; // eax

    KISAK_NULLSUB();
    Value = Sys_GetValue(2);
    //if (_setjmp3(Value, 0))
    if (_setjmp(*(jmp_buf *)Value))
    {
        Profile_Recover(1);
    }
    else
    {
        Profile_Guard(1);
        Com_CheckSyncFrame();
        {
            PROF_SCOPED("MainThread");
            Com_Frame_Try_Block_Function();
        }
        ++com_frameNumber;
        Profile_Unguard(1);
    }
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    if (com_errorEntered)
    {
        Com_ErrorCleanup();
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
        if (!com_dedicated->current.integer)
        {
            CL_InitRenderer();
            Com_StartHunkUsers();
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }

#ifdef TRACY_ENABLE
    TracyCFrameMarkEnd("Com_Frame");
#endif
}

void Com_StartHunkUsers()
{
    void* Value; // eax
    int MenuScreen; // eax

    Value = Sys_GetValue(2);

    //if (_setjmp3(Value, 0))
    if (_setjmp(*(jmp_buf *)Value))
        Sys_Error("Error during initialization:\n%s\n", com_errorMessage);
    Com_AssetLoadUI();
    MenuScreen = UI_GetMenuScreen();
    UI_SetActiveMenu(0, (uiMenuCommand_t)MenuScreen);
    IN_Frame();
    Com_EventLoop();
}

void __cdecl Com_CloseLogfiles()
{
    if (logfile)
    {
        FS_FCloseLogFile(logfile);
        logfile = 0;
    }
}

bool __cdecl Com_LogFileOpen()
{
    return logfile != 0;
}

void __cdecl Com_Close()
{
    Com_ShutdownDObj();
    DObjShutdown();
    XAnimShutdown();
    Com_ShutdownWorld();
    CM_Shutdown();
    SND_ShutdownChannels();
    Hunk_Clear();
    if (useFastFile->current.enabled)
        DB_ShutdownXAssets();
    Scr_Shutdown();
    NET_ShutdownDebug();
    Hunk_ShutdownDebugMemory();
}

void __cdecl Field_Clear(field_t* edit)
{
    memset((unsigned __int8*)edit->buffer, 0, sizeof(edit->buffer));
    edit->cursor = 0;
    edit->scroll = 0;
    edit->drawWidth = 256;
}

void __cdecl Com_Restart()
{
    CL_ShutdownHunkUsers();
    SV_ShutdownGameProgs();
    Com_ShutdownDObj();
    DObjShutdown();
    XAnimShutdown();
    Com_ShutdownWorld();
    CM_Shutdown();
    SND_ShutdownChannels();
    Hunk_Clear();
    Hunk_ResetDebugMem();
    if (useFastFile->current.enabled)
        DB_ReleaseXAssets();
    Com_SetScriptSettings();
    com_fixedConsolePosition = 0;
    if (Sys_IsRemoteDebugClient())
        NET_RestartDebug();
    FakeLag_Init();
    XAnimInit();
    DObjInit();
    Com_InitDObj();
}

void __cdecl Com_SetWeaponInfoMemory(int source)
{
    if (source != 1 && source != 2)
        MyAssertHandler(
            ".\\qcommon\\common.cpp",
            4551,
            0,
            "%s",
            "(source == WEAPMEMSOURCE_SERVER) || (source == WEAPMEMSOURCE_CLIENT)");
    if (weaponInfoSource)
        MyAssertHandler(".\\qcommon\\common.cpp", 4552, 0, "%s", "weaponInfoSource == WEAPMEMSOURCE_NONE");
    weaponInfoSource = source;
}

void __cdecl Com_FreeWeaponInfoMemory(int source)
{
    if (source != 1 && source != 2)
        MyAssertHandler(
            ".\\qcommon\\common.cpp",
            4560,
            0,
            "%s",
            "(source == WEAPMEMSOURCE_SERVER) || (source == WEAPMEMSOURCE_CLIENT)");
    if (source == weaponInfoSource)
    {
        weaponInfoSource = 0;
        BG_ShutdownWeaponDefFiles();
    }
}

int __cdecl Com_AddToString(const char* add, char* msg, int len, int maxlen, int mayAddQuotes)
{
    int addQuotes; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    addQuotes = 0;
    if (mayAddQuotes)
    {
        if (*add)
        {
            for (i = 0; i < maxlen - len && add[i]; ++i)
            {
                if (add[i] <= 32)
                {
                    addQuotes = 1;
                    break;
                }
            }
        }
        else
        {
            addQuotes = 1;
        }
    }
    if (addQuotes && len < maxlen)
        msg[len++] = 34;
    for (ia = 0; len < maxlen && add[ia]; ++ia)
        msg[len++] = add[ia];
    if (addQuotes && len < maxlen)
        msg[len++] = 34;
    return len;
}

char __cdecl Com_GetDecimalDelimiter()
{
    int lang; // [esp+0h] [ebp-4h]

    lang = loc_language->current.integer;
    if (lang == 1 || lang == 2 || lang == 3 || lang == 4 || lang == 6 || lang == 7 || lang == 14)
        return 44;
    else
        return 46;
}

void __cdecl Com_LocalizedFloatToString(float f, char* buffer, unsigned int maxlen, unsigned int numDecimalPlaces)
{
    unsigned int charPos; // [esp+8h] [ebp-8h]
    char delimiter; // [esp+Fh] [ebp-1h]

    _snprintf(buffer, maxlen - 1, "%.*f", numDecimalPlaces, f);
    buffer[maxlen - 1] = 0;
    delimiter = Com_GetDecimalDelimiter();
    if (delimiter != 46)
    {
        for (charPos = 0; charPos < maxlen; ++charPos)
        {
            if (buffer[charPos] == 46)
            {
                buffer[charPos] = delimiter;
                return;
            }
        }
    }
}

void __cdecl Com_SyncThreads()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 4655, 0, "%s", "Sys_IsMainThread()");
    R_SyncRenderThread();
    R_WaitWorkerCmds();
}

void __cdecl Com_FreeEvent(char* ptr)
{
    Z_Free(ptr, 10);
}

