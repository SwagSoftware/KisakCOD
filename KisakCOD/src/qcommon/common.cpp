#include "qcommon.h"

#include "../win32/win_local.h"

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

void QDECL Com_Error(int code, const char* fmt, ...)
{
	// KISAKTODO
}