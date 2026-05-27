#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "savedevice.h"
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>
#include <universal/com_files.h>
#include <client/client.h>      // SaveHeader
#include "../server/server.h"
#include <script/scr_readwrite.h>   // Scr_SaveSourceImmediate

// void __cdecl SaveDevice_Init(void)    8227fb88 f   savedevice_xenon.obj
// void __cdecl SV_DisplaySaveErrorUI(void) 8227fba8 f   savedevice_xenon.obj
// BOOL __cdecl BuildCleanSavePath(char *, unsigned int, char const *, enum SaveType) 8227fbf8 f   savedevice_xenon.obj
// BOOL __cdecl SaveDevice_IsAccessingDevice(void) 8227fdf0 f   savedevice_xenon.obj
// void __cdecl WriteSaveToDeviceCleanup(void) 8227fe00 f   savedevice_xenon.obj
// BOOL __cdecl SaveDevice_IsSaveSuccessful(void) 822800c8 f   savedevice_xenon.obj
// int __cdecl ReadFromDevice(void *, int, void *) 822800d8 f   savedevice_xenon.obj
// int __cdecl OpenDevice(char const *, void **) 82280140 f   savedevice_xenon.obj
// void __cdecl CloseDevice(void *)     822801e0 f   savedevice_xenon.obj
// BOOL __cdecl SaveExists(char const *)     82280280 f   savedevice_xenon.obj
// int __cdecl WriteSaveToDevice(unsigned char *, struct SaveHeader const *, BOOL) 82280448 f   savedevice_xenon.obj
// void __cdecl SV_ForceSelectSaveDevice_f(void) 823cfd38 f   sv_ccmds.obj
// void __cdecl SV_SelectSaveDevice_f(void) 823cfd90 f   sv_ccmds.obj
// char const *const CONSOLE_DEFAULT_SAVE_NAME 826969a4     savedevice_xenon.obj
// int marker_savedevice_pc 82e2081c     savedevice_pc.obj
// int marker_savedevice_xenon 82e208b4     savedevice_xenon.obj
// BOOL __cdecl SaveDevice_CheckForError(struct MemcardError const *) 8227fba0 f   savedevice_xenon.obj
// int __cdecl WriteSaveToDeviceInternal(struct SaveHeader const *) 8227fe48 f   savedevice_xenon.obj
// void __cdecl BeginScreenUpdateIfSupported(void) 82280218 f   savedevice_xenon.obj
// void __cdecl EndScreenUpdateIfSupported(void) 82280220 f   savedevice_xenon.obj
// void *__cdecl SaveExists_OpenContextFile(char const *) 82280228 f   savedevice_xenon.obj


static bool g_saveDevice_lastSaveSucceeded = true;

void __cdecl Memcard_InitializeSystem(void)
{
}

void __cdecl SaveDevice_Init(void)
{
	g_saveDevice_lastSaveSucceeded = true;
}

void __cdecl SV_DisplaySaveErrorUI(void)
{
	Com_PrintError(10, "SV_DisplaySaveErrorUI: a save operation failed\n");
}

bool __cdecl BuildCleanSavePath(char *cleanSavePath, unsigned int cleanSavePathSize, char const *filename, enum SaveType saveType)
{
	if (!filename)
		MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savedevice_xenon.cpp", 83, 0, "%s", "filename");
	if (!cleanSavePath)
		MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savedevice_xenon.cpp", 84, 0, "%s", "cleanSavePath");

	if (saveType == SAVE_TYPE_AUTOSAVE)
	{
		char *dst = cleanSavePath;
		const char *src = CONSOLE_DEFAULT_SAVE_NAME;
		int c;
		do
		{
			c = (unsigned char)*src++;
			*dst++ = (char)c;
		} while (c);
		return true;
	}

	const char *p = filename;
	while (*(unsigned char *)p++)
		;
	unsigned int len = (unsigned int)(p - filename - 1);

	if (len >= cleanSavePathSize || len >= 0x40u)
	{
		Com_Printf(10, "filename '%s' is too long.\n", filename);
		return false;
	}

	char buf[144];
	int i = 0;
	if (len)
	{
		while (1)
		{
			char ch = filename[i];
			if (ch == '/' || ch == '\\')
			{
				if (saveType && (i != 8 || I_strnicmp(filename, "autosave", 8)))
					buf[i] = '-';
				else
					buf[i] = '/';
			}
			else
			{
				if (!I_isforfilename(ch))
				{
					Com_Printf(
						10,
						"filename '%s' has invalid character (%c) in filename.  Must use alphanumeric characters only.\n",
						filename,
						filename[i]);
					return false;
				}
				buf[i] = filename[i];
			}
			if (++i >= (int)len)
				break;
		}
	}
	buf[i] = 0;
	Com_sprintf(cleanSavePath, 64, "save/%s.svg", buf);
	return true;
}

bool __cdecl SaveDevice_IsAccessingDevice(void)
{
	return false;
}

void __cdecl WriteSaveToDeviceCleanup(void)
{
}

bool __cdecl SaveDevice_IsSaveSuccessful(void)
{
	return g_saveDevice_lastSaveSucceeded;
}

int __cdecl OpenDevice(char const *name, void **fileHandle)
{
	if (!fileHandle)
		return -1;
	int handle = 0;
	unsigned int size = FS_FOpenFileRead(name, &handle);
	if (!handle)
	{
		*fileHandle = 0;
		return -1;
	}
	*fileHandle = (void *)(intptr_t)handle;
	return (int)size;
}

void __cdecl CloseDevice(void *fileHandle)
{
	int handle = (int)(intptr_t)fileHandle;
	if (handle)
		FS_FCloseFile(handle);
}

int __cdecl ReadFromDevice(void *buffer, int size, void *fileHandle)
{
	int handle = (int)(intptr_t)fileHandle;
	if (!handle || !buffer || size <= 0)
		return 0;
	return (int)FS_Read((unsigned char *)buffer, (unsigned int)size, handle);
}

bool __cdecl SaveExists(char const *savename)
{
	if (!savename || !*savename)
		return false;
	int handle = 0;
	FS_FOpenFileRead(savename, &handle);
	if (!handle)
		return false;
	FS_FCloseFile(handle);
	return true;
}

int __cdecl WriteSaveToDevice(unsigned char *data, struct SaveHeader const *saveHeader, bool /*suppressPlayerNotify*/)
{
	if (!data || !saveHeader)
	{
		g_saveDevice_lastSaveSucceeded = false;
		return -1;
	}

	int handle = FS_FOpenFileWrite(saveHeader->filename);
	if (!handle)
	{
		Com_PrintError(10, "WriteSaveToDevice: failed to open '%s' for writing\n", saveHeader->filename);
		g_saveDevice_lastSaveSucceeded = false;
		return -1;
	}

	const unsigned int headerSize = (unsigned int)sizeof(SaveHeader);
	const unsigned int bodySize = (unsigned int)saveHeader->bodySize;

	unsigned int wroteHeader = FS_Write((const char *)saveHeader, headerSize, handle);
	unsigned int wroteBody = (bodySize > 0)
		? FS_Write((const char *)data, bodySize, handle)
		: 0;


	SaveImmediate saveImmediate;
	saveImmediate.f = (void *)(intptr_t)handle;
	Scr_SaveSourceImmediate(&saveImmediate);
	if (saveHeader->demoPlayback)
		SV_SaveDemoImmediate(&saveImmediate);

	FS_FCloseFile(handle);

	bool ok = (wroteHeader == headerSize) && (wroteBody == bodySize);
	g_saveDevice_lastSaveSucceeded = ok;
	if (!ok)
	{
		Com_PrintError(10,
			"WriteSaveToDevice: short write to '%s' (header %u/%u, body %u/%u)\n",
			saveHeader->filename,
			wroteHeader, headerSize,
			wroteBody, bodySize);
		return -1;
	}
	return 0;
}

bool __cdecl SaveDevice_CheckForError(struct MemcardError const *)
{
	return false;
}

int __cdecl WriteSaveToDeviceInternal(struct SaveHeader const *saveHeader)
{
	if (!saveHeader)
		return -1;

	Com_PrintError(10, "WriteSaveToDeviceInternal: not used on PC port\n");
	return -1;
}

void __cdecl BeginScreenUpdateIfSupported(void)
{
}

void __cdecl EndScreenUpdateIfSupported(void)
{
}

void *__cdecl SaveExists_OpenContextFile(char const *savename)
{
	if (!savename || !*savename)
		return 0;
	int handle = 0;
	FS_FOpenFileRead(savename, &handle);
	if (!handle)
		return 0;
	return (void *)(intptr_t)handle;
}
