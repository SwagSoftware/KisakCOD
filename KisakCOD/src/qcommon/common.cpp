#include "qcommon.h"

#include "cmd.h"
#include "threads.h"
#include "../win32/win_local.h"

#include <universal/com_memory.h>

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

void QDECL Com_PrintMessage(int channel, char* msg, int error)
{
	// LWSS: Punkbuster stuff
	//PbCaptureConsoleOutput(msg, 4096);

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

void QDECL Com_Printf(int channel, const char* fmt, ...)
{
	char string[4100];
	va_list va;

	va_start(va, fmt);
	_vsnprintf(string, 0x1000u, fmt, va);
	string[4095] = 0;
	Com_PrintMessage(channel, string, 0);
}

void QDECL Com_Error(errorParm_t code, const char* fmt, ...)
{
	// KISAKTODO
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
	__asm
	{
		mov		edx, dest
		mov		eax, constant
		mov		ecx, count
		and ecx, ~7
		jz		padding
		sub		ecx, 8
		jmp		loopu
		align	16
		loopu:
		test[edx + ecx * 4 + 28], ebx		// fetch next block destination to L1 cache
		mov[edx + ecx * 4 + 0], eax
		mov[edx + ecx * 4 + 4], eax
		mov[edx + ecx * 4 + 8], eax
		mov[edx + ecx * 4 + 12], eax
		mov[edx + ecx * 4 + 16], eax
		mov[edx + ecx * 4 + 20], eax
		mov[edx + ecx * 4 + 24], eax
		mov[edx + ecx * 4 + 28], eax
		sub		ecx, 8
		jge		loopu
		padding : mov		ecx, count
		mov		ebx, ecx
		and ecx, 7
		jz		outta
		and ebx, ~7
		lea		edx, [edx + ebx * 4]				// advance dest pointer
		test[edx + 0], eax					// fetch destination to L1 cache
		cmp		ecx, 4
		jl		skip4
		mov[edx + 0], eax
		mov[edx + 4], eax
		mov[edx + 8], eax
		mov[edx + 12], eax
		add		edx, 16
		sub		ecx, 4
		skip4:		cmp		ecx, 2
		jl		skip2
		mov[edx + 0], eax
		mov[edx + 4], eax
		add		edx, 8
		sub		ecx, 2
		skip2 : cmp		ecx, 1
		jl		outta
		mov[edx + 0], eax
		outta :
	}
}

// optimized memory copy routine that handles all alignment
// cases and block sizes efficiently
//void Com_Memcpy(void* dest, const void* src, const size_t count) {
//	Com_Prefetch(src, count, PRE_READ);
//	__asm
//	{
//		push	edi
//		push	esi
//		mov		ecx, count
//		cmp		ecx, 0						// count = 0 check (just to be on the safe side)
//		je		outta
//		mov		edx, dest
//		mov		ebx, src
//		cmp		ecx, 32						// padding only?
//		jl		padding
//
//		mov		edi, ecx
//		and edi, ~31					// edi = count&~31
//		sub		edi, 32
//
//		align 16
//		loopMisAligned:
//		mov		eax, [ebx + edi + 0 + 0 * 8]
//		mov		esi, [ebx + edi + 4 + 0 * 8]
//		mov[edx + edi + 0 + 0 * 8], eax
//		mov[edx + edi + 4 + 0 * 8], esi
//		mov		eax, [ebx + edi + 0 + 1 * 8]
//		mov		esi, [ebx + edi + 4 + 1 * 8]
//		mov[edx + edi + 0 + 1 * 8], eax
//		mov[edx + edi + 4 + 1 * 8], esi
//		mov		eax, [ebx + edi + 0 + 2 * 8]
//		mov		esi, [ebx + edi + 4 + 2 * 8]
//		mov[edx + edi + 0 + 2 * 8], eax
//		mov[edx + edi + 4 + 2 * 8], esi
//		mov		eax, [ebx + edi + 0 + 3 * 8]
//		mov		esi, [ebx + edi + 4 + 3 * 8]
//		mov[edx + edi + 0 + 3 * 8], eax
//		mov[edx + edi + 4 + 3 * 8], esi
//		sub		edi, 32
//		jge		loopMisAligned
//
//		mov		edi, ecx
//		and edi, ~31
//		add		ebx, edi					// increase src pointer
//		add		edx, edi					// increase dst pointer
//		and ecx, 31					// new count
//		jz		outta					// if count = 0, get outta here
//
//		padding :
//		cmp		ecx, 16
//		jl		skip16
//		mov		eax, dword ptr[ebx]
//		mov		dword ptr[edx], eax
//		mov		eax, dword ptr[ebx + 4]
//		mov		dword ptr[edx + 4], eax
//		mov		eax, dword ptr[ebx + 8]
//		mov		dword ptr[edx + 8], eax
//		mov		eax, dword ptr[ebx + 12]
//		mov		dword ptr[edx + 12], eax
//		sub		ecx, 16
//		add		ebx, 16
//		add		edx, 16
//
//		skip16:
//		cmp		ecx, 8
//		jl		skip8
//		mov		eax, dword ptr[ebx]
//		mov		dword ptr[edx], eax
//		mov		eax, dword ptr[ebx + 4]
//		sub		ecx, 8
//		mov		dword ptr[edx + 4], eax
//		add		ebx, 8
//		add		edx, 8
//		skip8:
//		cmp		ecx, 4
//		jl		skip4
//		mov		eax, dword ptr[ebx]	// here 4-7 bytes
//		add		ebx, 4
//		sub		ecx, 4
//		mov		dword ptr[edx], eax
//		add		edx, 4
//		skip4:							// 0-3 remaining bytes
//		cmp		ecx, 2
//		jl		skip2
//		mov		ax, word ptr[ebx]	// two bytes
//		cmp		ecx, 3				// less than 3?
//		mov		word ptr[edx], ax
//		jl		outta
//		mov		al, byte ptr[ebx + 2]	// last byte
//		mov		byte ptr[edx + 2], al
//		jmp		outta
//		skip2 :
//		cmp		ecx, 1
//		jl		outta
//		mov		al, byte ptr[ebx]
//		mov		byte ptr[edx], al
//		outta :
//		pop		esi
//		pop		edi
//	}
//}

//void Com_Memset(void* dest, const int val, const size_t count)
//{
//	unsigned int fillval;
//
//	if (count < 8)
//	{
//		__asm
//		{
//			mov		edx, dest
//			mov		eax, val
//			mov		ah, al
//			mov		ebx, eax
//			and ebx, 0xffff
//			shl		eax, 16
//			add		eax, ebx				// eax now contains pattern
//			mov		ecx, count
//			cmp		ecx, 4
//			jl		skip4
//			mov[edx], eax			// copy first dword
//			add		edx, 4
//			sub		ecx, 4
//			skip4:	cmp		ecx, 2
//			jl		skip2
//			mov		word ptr[edx], ax	// copy 2 bytes
//			add		edx, 2
//			sub		ecx, 2
//			skip2 : cmp		ecx, 0
//			je		skip1
//			mov		byte ptr[edx], al	// copy single byte
//			skip1 :
//		}
//		return;
//	}
//
//	fillval = val;
//
//	fillval = fillval | (fillval << 8);
//	fillval = fillval | (fillval << 16);		// fill dword with 8-bit pattern
//
//	_copyDWord((unsigned int*)(dest), fillval, count / 4);
//
//	__asm									// padding of 0-3 bytes
//	{
//		mov		ecx, count
//		mov		eax, ecx
//		and ecx, 3
//		jz		skipA
//		and eax, ~3
//		mov		ebx, dest
//		add		ebx, eax
//		mov		eax, fillval
//		cmp		ecx, 2
//		jl		skipB
//		mov		word ptr[ebx], ax
//		cmp		ecx, 2
//		je		skipA
//		mov		byte ptr[ebx + 2], al
//		jmp		skipA
//		skipB :
//		cmp		ecx, 0
//		je		skipA
//		mov		byte ptr[ebx], al
//		skipA :
//	}
//}

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

		break;
	}
}


/*
=================
Com_Init
=================
*/
void Com_Init(char* commandLine) {
	char* s;

	try
	{
		Com_Printf(16, "%s %s build %s %s\n", "KisakCOD", "1.0", "win-x86", __DATE__); // KISAKTODO: incorporate CPUSTRING 



		com_fullyInitialized = qtrue;
		Com_Printf(16, "--- Common Initialization Complete ---\n");
	}

	catch (const char* reason) 
	{
		Sys_Error("Error during initialization: %s", reason);
	}
}





void __cdecl Com_ShutdownWorld()
{
    comWorld.isInUse = 0;
}

void __cdecl Com_InitPlayerProfiles(int localClientNum)
{
    DvarValue v1; // [esp-10h] [ebp-24h]
    unsigned int value_4; // [esp+4h] [ebp-10h]
    __int64 value_8; // [esp+8h] [ebp-Ch]

    ui_playerProfileAlreadyChosen = Dvar_RegisterInt(
        "ui_playerProfileAlreadyChosen",
        0,
        (DvarLimits)0x100000000LL,
        0x200u,
        "true if player profile has been selected.");
    *(_QWORD*)&v1.enabled = __PAIR64__(value_4, 1);
    *(_QWORD*)&v1.color[8] = value_8;
    Dvar_ChangeResetValue((dvar_s*)ui_playerProfileAlreadyChosen, v1);
    com_playerProfile = Dvar_RegisterString("com_playerProfile", (char*)"", 0x40u, "Player profile");
    if (!Com_SetInitialPlayerProfile(localClientNum))
        Com_ExecStartupConfigs(localClientNum, 0);
}

void __cdecl Com_PrintMessage(int channel, const char* msg, int error)
{
    //PbCaptureConsoleOutput(msg, 4096);
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

void __cdecl Com_LogPrintMessage(int channel, char* msg)
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

void Com_Printf(int channel, const char* fmt, ...)
{
    char string[4100]; // [esp+4h] [ebp-1008h] BYREF
    va_list va; // [esp+101Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    _vsnprintf(string, 0x1000u, fmt, va);
    string[4095] = 0;
    Com_PrintMessage(channel, string, 0);
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

void Com_PrintError(int channel, const char* fmt, ...)
{
    char dest; // [esp+14h] [ebp-1008h] BYREF
    char v3[4095]; // [esp+15h] [ebp-1007h] BYREF
    int v4; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    if (I_stristr(fmt, "error"))
        I_strncpyz(&dest, "^1", 4096);
    else
        I_strncpyz(&dest, "^1Error: ", 4096);
    v4 = &v3[strlen(&dest)] - v3;
    _vsnprintf(&v3[v4 - 1], 4096 - v4, fmt, va);
    v3[4094] = 0;
    ++com_errorPrintsCount;
    Com_PrintMessage(channel, &dest, 3);
}

void Com_PrintWarning(int channel, const char* fmt, ...)
{
    char dest; // [esp+14h] [ebp-1008h] BYREF
    char v3[4095]; // [esp+15h] [ebp-1007h] BYREF
    int v4; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    I_strncpyz(&dest, "^3", 4096);
    v4 = &v3[strlen(&dest)] - v3;
    _vsnprintf(&v3[v4 - 1], 4096 - v4, fmt, va);
    v3[4094] = 0;
    Com_PrintMessage(channel, &dest, 2);
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

void __cdecl Com_ShutdownInternal(const char* finalmsg)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        CL_Disconnect(localClientNum);
    CL_ResetStats_f();
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
    I_strncpyz(com_errorMessage, localizedErrorMessage, 4096);
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
    DoStackTrace(g_stackTrace, 1);
    Com_Printf(16, "STACKBEGIN -------------------------------------------------------------------\n");
    Com_Printf(16, g_stackTrace);
    Com_Printf(16, "STACKEND ---------------------------------------------------------------------\n");
}

void __cdecl  Com_ErrorAbort()
{
    Sys_Error("%s", com_errorMessage);
}

void Com_Error(errorParm_t code, const char* fmt, ...)
{
    int* Value; // eax
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
                UI_SetActiveMenu(0, 1);
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
        Value = (int*)Sys_GetValue(2);
        longjmp(Value, -1);
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
        UI_SetActiveMenu(0, 5);
        com_errorEntered = 0;
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }
    else
    {
        if (!com_dedicated->current.integer)
            goto LABEL_27;
        Sys_Print("\n==========================\n");
        Sys_Print(com_errorMessage);
        Sys_Print("\n==========================\n");
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
    return Z_Malloc(size, "Com_AllocEvent", 10);
}

void __cdecl Com_ClientPacketEvent()
{
    msg_t netmsg; // [esp+4Ch] [ebp-40h] BYREF
    netadr_t adr; // [esp+74h] [ebp-18h] BYREF

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 2010, 0, "%s", "Sys_IsMainThread()");
    //Profile_Begin(29);
    MSG_Init(&netmsg, clientCommonMsgBuf, 0x20000);
    Com_PacketEventLoop(NS_CLIENT1, &netmsg);
    if (!com_sv_running->current.enabled)
    {
        while (NET_GetClientPacket(&adr, &netmsg))
            Com_DispatchClientPacketEvent(adr, &netmsg);
    }
    //Profile_EndInternal(0);
}

void __cdecl Com_PacketEventLoop(netsrc_t client, msg_t* netmsg)
{
    unsigned int v2; // eax
    netadr_t adr; // [esp+0h] [ebp-18h] BYREF

    while (NET_GetLoopPacket(client, &adr, netmsg))
    {
        v2 = Sys_Milliseconds();
        CL_PacketEvent(client, adr, netmsg, v2);
    }
}

void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t* netmsg)
{
    unsigned int v2; // eax

    v2 = Sys_Milliseconds();
    CL_PacketEvent(NS_CLIENT1, adr, netmsg, v2);
}

void __cdecl Com_ServerPacketEvent()
{
    msg_t netmsg; // [esp+30h] [ebp-40h] BYREF
    netadr_t adr; // [esp+58h] [ebp-18h] BYREF

    //Profile_Begin(30);
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\common.cpp", 2072, 0, "%s", "Sys_IsMainThread()");
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
    //Profile_EndInternal(0);
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
            if (ev.evPtr)
                MyAssertHandler(".\\qcommon\\common.cpp", 2151, 0, "%s", "!ev.evPtr");
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
            Com_Error(ERR_FATAL, &byte_896E8C, ev.evType);
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
    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, "exec autoexec_dev_mp.cfg");
    Dvar_SetInAutoExec(0);
}

void __cdecl Com_ExecStartupConfigs(int localClientNum, const char* configFile)
{
    const char* v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax

    Cbuf_AddText(localClientNum, "exec default_mp.cfg\n");
    Cbuf_AddText(localClientNum, "exec language.cfg\n");
    if (configFile)
    {
        v2 = va("exec %s\n", configFile);
        Cbuf_AddText(localClientNum, v2);
    }
    v3 = CL_ControllerIndexFromClientNum(localClientNum);
    Cbuf_Execute(localClientNum, v3);
    v4 = CL_ControllerIndexFromClientNum(localClientNum);
    Com_RunAutoExec(localClientNum, v4);
    if (Com_SafeMode())
        Cbuf_AddText(localClientNum, "exec safemode_mp.cfg\n");
    v5 = CL_ControllerIndexFromClientNum(localClientNum);
    Cbuf_Execute(localClientNum, v5);
}

void __cdecl Com_Init(char* commandLine)
{
    void* Value; // eax
    const char* v2; // eax
    void* v3; // eax
    void* v4; // eax
    const char* v5; // eax

    Value = Sys_GetValue(2);
    if (_setjmp3(Value, 0))
    {
        v2 = va("Error during initialization:\n%s\n", com_errorMessage);
        Sys_Error(v2);
    }
    Com_Init_Try_Block_Function(commandLine);
    v3 = Sys_GetValue(2);
    if (!_setjmp3(v3, 0))
        Com_AddStartupCommands();
    if (com_errorEntered)
        Com_ErrorCleanup();
    if (!com_sv_running->current.enabled && !com_dedicated->current.integer)
    {
        v4 = Sys_GetValue(2);
        if (_setjmp3(v4, 0))
        {
            v5 = va("Error during initialization:\n%s\n", com_errorMessage);
            Sys_Error(v5);
        }
        if (!cls.rendererStarted)
            CL_InitRenderer();
        R_BeginRemoteScreenUpdate();
        CL_StartHunkUsers();
        R_EndRemoteScreenUpdate();
    }
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
            UI_SetActiveMenu(0, MenuScreenForError);
        }
        Com_SetErrorMessage(com_errorMessage);
    }
    if (fs_debug && fs_debug->current.integer == 2)
        Dvar_SetInt((dvar_s*)fs_debug, 0);
    SND_ErrorCleanup();
    Com_CleanupBsp();
    CL_ResetStats_f();
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

void __cdecl Com_Init_Try_Block_Function(char* commandLine)
{
    int v1; // eax
    const char* BuildNumber; // eax
    unsigned int v3; // eax
    int localClientNum; // [esp+10h] [ebp-Ch]
    int localClientNuma; // [esp+10h] [ebp-Ch]
    char* s; // [esp+14h] [ebp-8h]
    unsigned int initStartTime; // [esp+18h] [ebp-4h]

    Com_Printf(16, "%s %s build %s %s\n", "CoD4 MP", "1.0", "win-x86", "Aug 22 2011");
    Com_ParseCommandLine(commandLine);
    SL_Init();
    Swap_Init();
    Cbuf_Init();
    _Cmd_Init();
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
        PMem_BeginAlloc("$init", 1u);
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
    Cmd_AddCommandInternal(aQuit_1, Com_Quit_f, &Com_Quit_f_VAR);
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
    Dvar_ClearModified(com_dedicated);
    if (!com_dedicated->current.integer)
    {
        CL_ResetStats_f();
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
        CL_ResetStats_f();
        CL_InitRenderer();
        CL_ResetStats_f();
        if (cls.soundStarted)
            MyAssertHandler(".\\qcommon\\common.cpp", 3357, 0, "%s", "!cls.soundStarted");
        cls.soundStarted = 1;
        SND_Init();
    }
    COM_PlayIntroMovies();
    if (useFastFile->current.enabled)
    {
        PMem_EndAlloc("$init", 1u);
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
        Com_Error(ERR_FATAL, &byte_8975D0);
    else
        Com_Error(ERR_DROP, &byte_8975E8);
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
        Dvar_SetString((dvar_s*)nextmap, "cinematic atvi; set nextmap cinematic cod_intro");
        Dvar_SetBool((dvar_s*)com_introPlayed, 1);
    }
}

void Com_InitDvars()
{
    unsigned int CpuCount; // eax

    com_dedicated = Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x20u, "True if this is a dedicated server");
    if (com_dedicated->current.integer)
        Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x40u, "True if this is a dedicated server");
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
    CpuCount = Sys_GetCpuCount();
    sys_smp_allowed = Dvar_RegisterBool("sys_smp_allowed", CpuCount > 1, 0x10u, "Allow multi-threading");
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
    com_timescale = Dvar_RegisterFloat("com_timescale", 1.0, 0.001, 1000.0, 0x10C8u, "Scale time of each frame");
    dev_timescale = Dvar_RegisterFloat("timescale", 1.0, 0.001, 1000.0, 0x88u, "Scale time of each frame");
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
    int f; // [esp+0h] [ebp-4h]

    f = FS_FOpenFileWrite(filename);
    if (f)
    {
        FS_Printf(f, "// generated by Call of Duty, do not modify\n");
        Dvar_WriteDefaults(f);
        FS_FCloseFile(f);
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
    int f; // [esp+0h] [ebp-4h]

    f = FS_FOpenFileWriteToDir(filename, "players");
    if (f)
    {
        FS_Printf(f, "// generated by Call of Duty, do not modify\n");
        FS_Printf(f, "unbindall\n");
        Key_WriteBindings(localClientNum, f);
        Dvar_WriteVariables(f);
        Con_WriteFilterConfigString(f);
        FS_FCloseFile(f);
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

    if (cmd_args.nesting != -1)
        MyAssertHandler(".\\qcommon\\common.cpp", 3904, 0, "cmd_args.nesting == -1\n\t%i, %i", cmd_args.nesting, -1);
    Com_WriteConfiguration(0);
    SetAnimCheck(com_animCheck->current.color[0]);
    minMsec = 1;
    maxFPS = com_maxfps->current.integer;
    Com_AdjustMaxFPS(&maxFPS);
    if (maxFPS > 0 && !com_dedicated->current.integer)
    {
        minMsec = 1000 / maxFPS;
        if (1000 / maxFPS < 0)
            MyAssertHandler(".\\qcommon\\common.cpp", 3931, 0, "%s", "minMsec >= 0");
        if (!minMsec)
            minMsec = 1;
    }
    Win_UpdateThreadLock();
    if (minMsec <= 0)
        MyAssertHandler(".\\qcommon\\common.cpp", 3954, 0, "%s", "minMsec > 0");
    v0 = com_lastFrameIndex & 0x80000000;
    if (com_lastFrameIndex < 0)
        v0 = 0;
    lastFrameIndex = v0;
    ++com_lastFrameIndex;
    if (com_dedicated->current.integer)
    {
        //Profile_Begin(361);
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
        //Profile_EndInternal(0);
        com_lastFrameTime[lastFrameIndex] = com_frameTime;
    }
    else
    {
        CL_ResetStats_f();
        //Profile_Begin(361);
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
        //Profile_EndInternal(0);
        if (com_frameTime - com_lastFrameTime[lastFrameIndex] < minMsec)
            v4 = minMsec;
        else
            v4 = com_frameTime - com_lastFrameTime[lastFrameIndex];
        com_lastFrameTime[lastFrameIndex] += v4;
        msec = lastFrameIndex + v4;
        if (!(lastFrameIndex + v4))
            msec = 1;
    }
    v1 = CL_ControllerIndexFromClientNum(0);
    Cbuf_Execute(0, v1);
    if (msec <= 0)
        MyAssertHandler(".\\qcommon\\common.cpp", 4008, 0, "%s", "msec > 0");
    mseca = Com_ModifyMsec(msec);
    if (mseca <= 0)
        MyAssertHandler(".\\qcommon\\common.cpp", 4013, 0, "%s", "msec > 0");
    msecb = SV_Frame(mseca);
    Com_DedicatedModified();
    if (!com_dedicated->current.integer)
    {
        R_SetEndTime(com_lastFrameTime[lastFrameIndex]);
        CL_ResetStats_f();
        CL_RunOncePerClientFrame(0, msecb);
        Com_EventLoop();
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            v2 = CL_ControllerIndexFromClientNum(localClientNum);
            Cbuf_Execute(localClientNum, v2);
        }
        CL_ResetStats_f();
        //Profile_Begin(16);
        for (localClientNuma = NS_CLIENT1; localClientNuma < NS_SERVER; ++localClientNuma)
            CL_Frame(localClientNuma);
        //Profile_EndInternal(0);
        dvar_modifiedFlags &= ~2u;
        Com_UpdateMenu();
        SCR_UpdateScreen();
        Ragdoll_Update(msecb);
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\qcommon\\common.cpp", 4079, 0, "%s", "Sys_IsMainThread()");
        CL_ResetStats_f();
        deltaTime = (double)cls.frametime * 0.001000000047497451;
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
    float v3; // [esp+8h] [ebp-1Ch]
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
        v3 = dev_timescale->current.value * (com_codeTimeScale * (com_timescale->current.value * (double)msec));
        msec = (int)(v3 + 9.313225746154785e-10);
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
    int MenuScreen; // eax
    connstate_t clcState; // [esp+4h] [ebp-4h]

    clcState = client_state[0];
    result = UI_IsFullscreen(0);
    if (!result && clcState == CA_DISCONNECTED)
    {
        MenuScreen = UI_GetMenuScreen();
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
    void* Value; // eax

    CL_ResetStats_f();
    Value = Sys_GetValue(2);
    if (_setjmp3(Value, 0))
    {
        //Profile_Recover(1);
    }
    else
    {
        //Profile_Guard(1);
        Com_CheckSyncFrame();
        //Profile_Begin(360);
        Com_Frame_Try_Block_Function();
        //Profile_EndInternal(0);
        ++com_frameNumber;
        //Profile_Unguard(1);
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
}

void Com_StartHunkUsers()
{
    void* Value; // eax
    int MenuScreen; // eax

    Value = Sys_GetValue(2);

    if (_setjmp3(Value, 0))
        Sys_Error("Error during initialization:\n%s\n", com_errorMessage);

    Com_AssetLoadUI();
    MenuScreen = UI_GetMenuScreen();
    UI_SetActiveMenu(0, MenuScreen);
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

void __cdecl Com_InitDObj()
{
    Com_Memset((unsigned int*)objAlloced, 0, 2048);
    objFreeCount = 2047;
    Com_Memset((unsigned int*)clientObjMap, 0, 2304);
    Com_Memset((unsigned int*)serverObjMap, 0, 2048);
    com_lastDObjIndex = 1;
    g_bDObjInited = 1;
}

void __cdecl Com_ShutdownDObj()
{
    const char* v0; // eax
    const char* v1; // eax
    unsigned int i; // [esp+0h] [ebp-4h]
    unsigned int ia; // [esp+0h] [ebp-4h]
    unsigned int ib; // [esp+0h] [ebp-4h]

    if (g_bDObjInited)
    {
        g_bDObjInited = 0;
        for (i = 0; i < 0x800; ++i)
        {
            if (objAlloced[i])
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 492, 0, "%s\n\t(i) = %i", "(!objAlloced[i])", i);
        }
        for (ia = 0; ia < 0x480; ++ia)
        {
            if (clientObjMap[ia])
            {
                v0 = va("i: %d, clientObjMap[i]: %d", ia, clientObjMap[ia]);
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 494, 0, "%s\n\t%s", "!clientObjMap[i]", v0);
            }
        }
        for (ib = 0; ib < 0x400; ++ib)
        {
            if (serverObjMap[ib])
            {
                v1 = va("i: %d, serverObjMap[i]: %d", ib, serverObjMap[ib]);
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 500, 0, "%s\n\t%s", "!serverObjMap[i]", v1);
            }
        }
        if (objFreeCount != 2047)
            MyAssertHandler(
                ".\\qcommon\\dobj_management.cpp",
                501,
                0,
                "%s\n\t(objFreeCount) = %i",
                "(objFreeCount == (sizeof( objAlloced ) / (sizeof( objAlloced[0] ) * (sizeof( objAlloced ) != 4 || sizeof( objAll"
                "oced[0] ) <= 4))) - 1)",
                objFreeCount);
    }
}

void __cdecl Com_InitialHull(
    const float (*points)[2],
    unsigned int* pointOrder,
    unsigned int pointCount,
    unsigned int* hullOrder)
{
    unsigned int maxIndex; // [esp+0h] [ebp-Ch]
    unsigned int pointIndex; // [esp+4h] [ebp-8h]
    unsigned int minIndex; // [esp+8h] [ebp-4h]

    minIndex = 0;
    maxIndex = 0;
    *pointOrder = 0;
    for (pointIndex = 1; pointIndex < pointCount; ++pointIndex)
    {
        pointOrder[pointIndex] = pointIndex;
        if ((*points)[2 * pointIndex + 1] < (double)(*points)[2 * maxIndex + 1])
        {
            if ((*points)[2 * pointIndex + 1] < (double)(*points)[2 * minIndex + 1])
                minIndex = pointIndex;
        }
        else
        {
            maxIndex = pointIndex;
        }
    }
    if (minIndex == maxIndex)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 36, 1, "%s", "minIndex != maxIndex");
    *hullOrder = minIndex;
    hullOrder[1] = maxIndex;
    if (minIndex <= maxIndex)
    {
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 2);
    }
    else
    {
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 2);
    }
}

void Com_InitHunkMemory()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\universal\\com_memory.cpp", 1258, 0, "%s", "Sys_IsMainThread()");
    if (s_hunkData)
        MyAssertHandler(".\\universal\\com_memory.cpp", 1259, 0, "%s", "!s_hunkData");
    if (FS_LoadStack())
        Com_Error(ERR_FATAL, &byte_8C0604);
    if (!useFastFile->current.enabled)
        s_hunkTotal = (int)&svs.archivedSnapshotBuffer[22495840];
    if (useFastFile->current.enabled)
        s_hunkTotal = (int)&unk_A00000;
    R_ReflectionProbeRegisterDvars();
    if (r_reflectionProbeGenerate->current.enabled)
        s_hunkTotal = 0x20000000;
    s_hunkData = (unsigned __int8*)Z_VirtualReserve(s_hunkTotal);
    if (!s_hunkData)
        Sys_OutOfMemErrorInternal(".\\universal\\com_memory.cpp", 1318);
    s_origHunkData = s_hunkData;
    track_set_hunk_size(s_hunkTotal);
    Hunk_Clear();
    _Cmd_AddCommandInternal("meminfo", Com_Meminfo_f, &Com_Meminfo_f_VAR);
    _Cmd_AddCommandInternal("tempmeminfo", Com_TempMeminfo_f, &Com_TempMeminfo_f_VAR);
}

void __cdecl Com_InitSoundDevGuiGraphs()
{
    if (useFastFile->current.enabled)
        ((void(__cdecl*)(void (*)()))Com_InitSoundDevGuiGraphs_FastFile)(Com_InitSoundDevGuiGraphs_FastFile);
    else
        ((void(__cdecl*)(void (*)()))Com_InitSoundDevGuiGraphs_LoadObj)(Com_InitSoundDevGuiGraphs_LoadObj);
}

void Com_InitSoundDevGuiGraphs_LoadObj()
{
    char devguiPath[256]; // [esp+0h] [ebp-108h] BYREF
    int i; // [esp+104h] [ebp-4h]

    if (!g_sa.curvesInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 240, 0, "%s", "g_sa.curvesInitialized");
    for (i = 1; i < 16; ++i)
    {
        if (*(unsigned int*)&g_sa.volumeFalloffCurveNames[-18][72 * i])
        {
            sprintf(
                devguiPath,
                "Main/Snd:6/Volume Falloff Curves/%s:%d",
                *(const char**)&g_sa.volumeFalloffCurveNames[-18][72 * i],
                i);
            g_sa.curveDevGraphs[i].knotCountMax = 8;
            g_sa.curveDevGraphs[i].knots = g_sa.volumeFalloffCurves[i].knots;
            g_sa.curveDevGraphs[i].knotCount = &g_sa.volumeFalloffCurves[i].knotCount;
            g_sa.curveDevGraphs[i].eventCallback = (void(__cdecl*)(const DevGraph*, DevEventType, int))Com_VolumeFalloffCurveGraphEventCallback;
            g_sa.curveDevGraphs[i].data = (void*)i;
            g_sa.curveDevGraphs[i].disableEditingEndPoints = 1;
            DevGui_AddGraph(devguiPath, &g_sa.curveDevGraphs[i]);
        }
    }
}

void Com_InitSoundDevGuiGraphs_FastFile()
{
    int counter; // [esp+0h] [ebp-4h] BYREF

    counter = 0;
    DB_EnumXAssets(ASSET_TYPE_SOUND_CURVE, (void(__cdecl*)(XAssetHeader, void*))Com_GetGraphList, &counter, 0);
}

void __cdecl Com_InitSoundAliasHash(unsigned int aliasCount)
{
    g_sa.hashUsed = 0;
    g_sa.hashSize = (3 * aliasCount + 1) >> 1;
    g_sa.hash = (snd_alias_list_t**)CM_Hunk_Alloc(4 * ((3 * aliasCount + 1) >> 1), "Com_InitSoundAliasHash", 15);
    memset((unsigned __int8*)g_sa.hash, 0, 4 * ((3 * aliasCount + 1) >> 1));
}

void Com_InitSpeakerMaps()
{
    int fileIndex; // [esp+10h] [ebp-58h]
    const char** fileNames; // [esp+14h] [ebp-54h]
    char name[68]; // [esp+18h] [ebp-50h] BYREF
    int len; // [esp+60h] [ebp-8h]
    int fileCount; // [esp+64h] [ebp-4h] BYREF

    if (g_sa.speakerMapsInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 360, 0, "%s", "!g_sa.speakerMapsInitialized");
    Com_InitDefaultSpeakerMap();
    fileNames = FS_ListFiles("soundaliases", "spkrmap", FS_LIST_PURE_ONLY, &fileCount);
    if (fileCount > 15)
        Com_Error(ERR_DROP, &byte_8C1120, fileCount, 15);
    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
    {
        len = strlen(fileNames[fileIndex]) - 8;
        if (len >= 64)
            Com_Error(ERR_DROP, &byte_8C1100, fileNames[fileIndex]);
        strncpy((unsigned __int8*)name, (unsigned __int8*)fileNames[fileIndex], len);
        name[len] = 0;
        if (!Com_LoadSpkrMapFile(name, &g_sa.speakerMaps[fileIndex + 1]))
            Com_Error(ERR_DROP, &byte_8C10DC, fileNames[fileIndex]);
    }
    FS_FreeFileList(fileNames);
    Cmd_AddCommandInternal("snd_refreshSpeakerMaps", Com_RefreshSpeakerMaps_f, &Com_RefreshSpeakerMaps_f_VAR);
    g_sa.speakerMapsInitialized = 1;
}

void Com_InitDefaultSpeakerMap()
{
    Com_InitDefaultSoundAliasSpeakerMap(g_sa.speakerMaps);
}

void Com_InitCurves()
{
    int fileIndex; // [esp+10h] [ebp-10h]
    const char** fileNames; // [esp+14h] [ebp-Ch]
    char* name; // [esp+18h] [ebp-8h]
    int fileCount; // [esp+1Ch] [ebp-4h] BYREF

    if (g_sa.curvesInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 867, 0, "%s", "!g_sa.curvesInitialized");
    memset((unsigned __int8*)g_sa.volumeFalloffCurves, 0, sizeof(g_sa.volumeFalloffCurves));
    Com_InitDefaultSoundAliasVolumeFalloffCurve(g_sa.volumeFalloffCurves);
    fileNames = FS_ListFiles("soundaliases", "vfcurve", FS_LIST_PURE_ONLY, &fileCount);
    if (fileCount > 15)
        Com_Error(ERR_DROP, &byte_8C1120, fileCount, 15);
    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
    {
        name = g_sa.volumeFalloffCurveNames[fileIndex + 1];
        I_strncpyz(name, (char*)fileNames[fileIndex], strlen(fileNames[fileIndex]) - 7);
        if (!Com_LoadVolumeFalloffCurve(name, &g_sa.volumeFalloffCurves[fileIndex + 1]))
            Com_Error(ERR_FATAL, &byte_8C134C, fileNames[fileIndex]);
    }
    FS_FreeFileList(fileNames);
    g_sa.curvesInitialized = 1;
}

void __cdecl Com_InitSoundAlias()
{
    saLoadObjGlob.tempAliases = 0;
    saLoadObjGlob.tempAliasCount = 0;
}

void __thiscall Com_InitEntChannels(char* file_1)
{
    char* file; // [esp+0h] [ebp-4h] BYREF

    file = file_1;
    if (FS_ReadFile("soundaliases/channels.def", (void**)&file) < 0)
        Com_Error(ERR_DROP, "unable to load entity channel file [%s].\n", "soundaliases/channels.def");
    Com_ParseEntChannelFile(file);
}

void __cdecl Com_InitDefaultSoundAliasVolumeFalloffCurve(SndCurve* sndCurve)
{
    sndCurve->filename = "";
    sndCurve->knots[0][0] = 0.0;
    sndCurve->knots[0][1] = 1.0;
    sndCurve->knots[1][0] = 1.0;
    sndCurve->knots[1][1] = 0.0;
    sndCurve->knotCount = 2;
}

void __cdecl Com_InitDefaultSoundAliasSpeakerMap(SpeakerMapInfo* info)
{
    Com_PreLoadSpkrMapFile(info);
    info->speakerMap.isDefault = 1;
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[0], 0, 0, 0.5);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[0], 0, 1u, 0.5);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 0, 0, 1.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 1u, 0, 0.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 0, 1u, 0.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 1u, 1u, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 0, 0.5);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 1u, 0.5);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 5u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 0, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 0, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 1u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 1u, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 5u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 5u, 0.0);
}

void __cdecl Com_InitParse()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 3; ++i)
        Com_InitParseInfo(g_parse[i].parseInfo);
}

void __cdecl Com_InitParseInfo(parseInfo_t* pi)
{
    pi->lines = 1;
    pi->ungetToken = 0;
    pi->spaceDelimited = 1;
    pi->keepStringQuotes = 0;
    pi->csv = 0;
    pi->negativeNumbers = 0;
    pi->errorPrefix = "";
    pi->warningPrefix = "";
    pi->backup_lines = 0;
    pi->backup_text = 0;
}

void __cdecl Com_InitThreadData(int threadContext)
{
    Sys_SetValue(1, &va_info[threadContext]);
    Sys_SetValue(2, g_com_error[threadContext]);
    Sys_SetValue(3, &g_traceThreadInfo[threadContext]);
}

void __cdecl Com_FreeEvent(char* ptr)
{
    Z_Free(ptr, 10);
}

