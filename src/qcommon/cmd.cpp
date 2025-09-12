#include "cmd.h"

#include <universal/assertive.h>

#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"

#include <database/database.h>
#include <win32/win_local.h>
#include <universal/com_files.h>
#include <script/scr_debugger.h>
#include <server/sv_game.h>
#include <universal/profile.h>

static cmd_function_s* cmd_functions = NULL;


static cmd_function_s Cmd_Wait_f_VAR;
static cmd_function_s Cmd_List_f_VAR;
static cmd_function_s Cmd_Exec_f_VAR;
static cmd_function_s Cmd_Vstr_f_VAR;
static cmd_function_s Cmd_Dumpraw_f_VAR;

static void Cmd_ResetArgs(CmdArgs* args, CmdArgsPrivate* argsPriv)
{
	args->nesting = -1;
	argsPriv->totalUsedArgvPool = 0;
	argsPriv->totalUsedTextPool = 0;
}

CmdArgs sv_cmd_args;
CmdArgs cmd_args;
CmdArgsPrivate cmd_argsPrivate;
CmdArgsPrivate sv_cmd_argsPrivate;
CmdText sv_cmd_text;
CmdText cmd_textArray[1];
uint8_t cmd_text_buf[1][65536];
uint8_t sv_cmd_text_buf[65536];

int32_t  marker_cmd;
int32_t  cmd_wait;
bool cmd_insideCBufExecute[1];


cmd_function_s *__cdecl _Cmd_FindCommand(const char *cmdName)
{
    cmd_function_s *cmd; // [esp+14h] [ebp-4h]

    for (cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if (!strcmp(cmdName, cmd->name))
            return cmd;
    }
    return 0;
}

const char **__cdecl Cmd_GetAutoCompleteFileList(const char *cmdName, int32_t  *fileCount)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    iassert( cmdName );
    iassert( fileCount );
    *fileCount = 0;
    cmd = _Cmd_FindCommand(cmdName);
    if (cmd && cmd->autoCompleteDir && cmd->autoCompleteExt)
        return FS_ListFiles(cmd->autoCompleteDir, cmd->autoCompleteExt, FS_LIST_PURE_ONLY, fileCount);
    else
        return 0;
}

int Cmd_LocalClientNum()
{
    bcassert(cmd_args.nesting, CMD_MAX_NESTING);
    return 0;
}

/*
============
Cmd_Argc
============
*/
int32_t 	Cmd_Argc(void) {
	iassert(cmd_args.nesting >= 0 && cmd_args.nesting < 8);
	return cmd_args.argc[cmd_args.nesting];
}

/*
============
Cmd_Argv
============
*/
const char* Cmd_Argv(int32_t  arg) {
	iassert(cmd_args.nesting < 8);
	iassert(arg >= 0);

	if ((unsigned)arg >= cmd_args.argc[cmd_args.nesting])
	{
		return (char*)"";
	}
	return (char*)(cmd_args.argv[cmd_args.nesting][arg]);
}

int32_t  __cdecl SV_Cmd_Argc()
{
    if (sv_cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);

    return sv_cmd_args.argc[sv_cmd_args.nesting];
}

const char *__cdecl SV_Cmd_Argv(int32_t  argIndex)
{
    if (sv_cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\../qcommon/cmd.h",
            182,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
    if (argIndex < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\../qcommon/cmd.h",
            183,
            0,
            "%s\n\t(argIndex) = %i",
            "(argIndex >= 0)",
            argIndex);
    if (argIndex >= sv_cmd_args.argc[sv_cmd_args.nesting])
        return "";
    else
        return sv_cmd_args.argv[sv_cmd_args.nesting][argIndex];
}

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "cmd use rocket ; +attack ; wait ; -attack ; cmd use blaster"
============
*/
void Cmd_Wait_f(void) {
	if (Cmd_Argc() == 2) {
		cmd_wait = atoi(Cmd_Argv(1));
	}
	else {
		cmd_wait = 1;
	}
}
/*
===============
Cmd_Vstr_f

Inserts the current value of a variable as command text
===============
*/
void Cmd_Vstr_f(void) {
	char* v;

	if (Cmd_Argc() != 2) {
		Com_Printf(0, "vstr <variablename> : execute a variable command\n");
		return;
	}

	const char* dvarName = Cmd_Argv(1);
	const dvar_s* dvar = Dvar_FindVar(dvarName);
	if (dvar)
	{
		if (dvar->type == 7 || dvar->type == 6)
		{
			const char* v0 = va("%s\n", dvar->current.string);
			Cbuf_InsertText(0, v0);
		}
		else
		{
			Com_Printf(0, "%s is not a string-based dvar\n", dvar->name);
		}
	}
	else
	{
		Com_Printf(0, "%s doesn't exist\n", dvarName);
	}
}

// avail add
#include <universal/com_files.h>
#include <format>
#include <filesystem>
#include <vector>

static void LoadXAssets()
{



#if 0
    zoneInfo[0].name = gfxCfg.codeFastFileName;
    zoneInfo[0].allocFlags = 2;
    zoneInfo[0].freeFlags = 0;
    zoneCount = 1;
    if (gfxCfg.localizedCodeFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.localizedCodeFastFileName;
        zoneInfo[zoneCount].allocFlags = 0;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    if (gfxCfg.uiFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.uiFastFileName;
        zoneInfo[zoneCount].allocFlags = 8;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    zoneInfo[zoneCount].name = gfxCfg.commonFastFileName;
    zoneInfo[zoneCount].allocFlags = 4;
    zoneInfo[zoneCount++].freeFlags = 0;
    if (gfxCfg.localizedCommonFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.localizedCommonFastFileName;
        zoneInfo[zoneCount].allocFlags = 1;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    if (gfxCfg.modFastFileName)
    {
        zoneInfo[zoneCount].name = gfxCfg.modFastFileName;
        zoneInfo[zoneCount].allocFlags = 16;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    DB_LoadXAssets(zoneInfo, zoneCount, 0);
#endif
}
void Cmd_Dumpraw_f(void)
{
    auto DumpFileType = [](XAssetType type) -> void
    {
		auto rawDir = std::format("{}\\raw\\", (char*)fs_basepath->current.integer);

        XAssetHeader files[10000]{ 0 };
		auto read = DB_GetAllXAssetOfType_FastFile(type, files, 10000);
		for (auto file : files)
		{
			if (!file.rawfile)
			{
				continue;
			}

			std::filesystem::path p(file.rawfile->name);

			auto dir = p.parent_path();
			std::filesystem::create_directories(rawDir / dir);

			FILE* f;
			fopen_s(&f, std::format("{}\\{}", rawDir, file.rawfile->name).c_str(), "wb");
			fwrite(file.rawfile->buffer, file.rawfile->len, 1, f);
			fflush(f);
			fclose(f);
		}
    }; 

    auto zoneDir = std::format("{}\\zone\\english\\", (char *)fs_basepath->current.integer);

    for (const auto &entry : std::filesystem::directory_iterator(zoneDir))
    {
        DB_ResetZoneSize(0);

        XZoneInfo zinfo;

        std::string tmp = entry.path().filename().generic_string();
        size_t ext = tmp.find(".ff");
        if (ext == std::string::npos)
        {
            continue;
        }
        if (tmp.find("mp_") == std::string::npos)
        {
            continue;
        }
        tmp = tmp.substr(0, ext);
        zinfo.name = tmp.c_str();
        zinfo.allocFlags = 64;
        zinfo.freeFlags = 0;

        Com_SyncThreads();

        DB_LoadXAssets(&zinfo, 1, 0);
        DB_SyncXAssets();
        DumpFileType(ASSET_TYPE_RAWFILE);
    }

    //DumpFileType(ASSET_TYPE_RAWFILE);
    //DumpFileType(ASSET_TYPE_MENU);
}
// avail end

void Cmd_Init()
{
	Cmd_ResetArgs(&cmd_args, &cmd_argsPrivate);
	Cmd_ResetArgs(&sv_cmd_args, &sv_cmd_argsPrivate);
	Cmd_AddCommandInternal("cmdlist", Cmd_List_f, &Cmd_List_f_VAR);
	Cmd_AddCommandInternal("exec", Cmd_Exec_f, &Cmd_Exec_f_VAR);
	Cmd_AddCommandInternal("vstr", Cmd_Vstr_f, &Cmd_Vstr_f_VAR);
	Cmd_AddCommandInternal("wait", Cmd_Wait_f, &Cmd_Wait_f_VAR);
    // avail add
    Cmd_AddCommandInternal("dumpraw", Cmd_Dumpraw_f, &Cmd_Dumpraw_f_VAR);
    // avail end
}

void Cmd_AddCommandInternal(const char* cmdName, void(__cdecl* function)(), cmd_function_s* allocedCmd)
{
	iassert(cmdName);

	cmd_function_s* cmd = Cmd_FindCommand(cmdName);
	if (cmd)
	{
		iassert(cmd == allocedCmd);

		if (function)
		{
			Com_Printf(16, "Cmd_AddCommand: %s already defined\n", cmdName);
		}
	}
	else
	{
		allocedCmd->name = cmdName;
		allocedCmd->function = function;
		allocedCmd->next = cmd_functions;
		cmd_functions = allocedCmd;
	}
}

cmd_function_s* Cmd_FindCommand(const char* cmdName)
{
	cmd_function_s* cmd;

	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strcmp(cmdName, cmd->name))
		{
			return cmd;
		}
	}

	return NULL;
}

void __cdecl TRACK_cmd()
{
    track_static_alloc_internal(&cmd_args, 132, "cmd_args", 10);
    track_static_alloc_internal(&cmd_argsPrivate, 10280, "cmd_argsPrivate", 10);
    track_static_alloc_internal(cmd_textArray, 12, "cmd_textArray", 10);
    track_static_alloc_internal(cmd_text_buf, 0x10000, "cmd_text_buf", 10);
    track_static_alloc_internal(&sv_cmd_args, 132, "sv_cmd_args", 10);
    track_static_alloc_internal(&sv_cmd_argsPrivate, 10280, "sv_cmd_argsPrivate", 10);
    track_static_alloc_internal(&sv_cmd_text, 12, "sv_cmd_text", 10);
    track_static_alloc_internal(sv_cmd_text_buf, 0x10000, "sv_cmd_text_buf", 10);
}

void __cdecl _Cmd_Wait_f()
{
    const char *v0; // eax

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        cmd_wait = atoi(v0);
    }
    else
    {
        cmd_wait = 1;
    }
}

void __cdecl Cbuf_Init()
{
    int32_t  client; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    for (client = 0; client < 1; ++client)
    {
        cmd_textArray[client].data = cmd_text_buf[client];
        cmd_textArray[client].maxsize = 0x10000;
        cmd_textArray[client].cmdsize = 0;
        cmd_insideCBufExecute[client] = 0;
    }
    sv_cmd_text.data = sv_cmd_text_buf;
    sv_cmd_text.maxsize = 0x10000;
    sv_cmd_text.cmdsize = 0;
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl Cbuf_AddText(int32_t  localClientNum, const char *text)
{
    CmdText *cmd_text; // [esp+0h] [ebp-8h]
    int32_t  length; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    if ((*text == 112 || *text == 80) && text[1] == 48)
    {
        localClientNum = text[1] - 48;
        for (text += 2; *text == 32; ++text)
            ;
    }
    cmd_text = &cmd_textArray[localClientNum];
    length = strlen_noncrt(text);
    if (cmd_text->cmdsize + length < cmd_text->maxsize)
    {
        memcpy_noncrt(&cmd_text->data[cmd_text->cmdsize], text, length + 1);
        cmd_text->cmdsize += length;
        Scr_MonitorCommand(text);
    }
    else
    {
        Com_Printf(16, "Cbuf_AddText: overflow\n");
    }
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl memcpy_noncrt(void *dst, const void *src, uint32_t length)
{
    memcpy(dst, src, length);
}

int32_t  __cdecl strlen_noncrt(const char *str)
{
    int32_t  count; // [esp+0h] [ebp-4h]

    count = 0;
    while (*str)
    {
        ++count;
        ++str;
    }
    return count;
}

void __cdecl Cbuf_InsertText(int32_t  localClientNum, const char *text)
{
    uint32_t v2; // [esp+4h] [ebp-1Ch]
    CmdText *cmd_text; // [esp+14h] [ebp-Ch]
    int32_t  i; // [esp+18h] [ebp-8h]
    int32_t  length; // [esp+1Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    cmd_text = &cmd_textArray[localClientNum];
    v2 = strlen(text);
    length = v2 + 1;
    if ((int32_t )(cmd_text->cmdsize + v2 + 1) <= cmd_text->maxsize)
    {
        for (i = cmd_text->cmdsize - 1; i >= 0; --i)
            cmd_text->data[length + i] = cmd_text->data[i];
        memcpy(cmd_text->data, (uint8_t *)text, v2);
        cmd_text->data[v2] = 10;
        cmd_text->cmdsize += length;
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
    }
    else
    {
        Com_PrintError(1, "Cbuf_InsertText overflowed\n");
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
    }
}

void __cdecl Cbuf_AddServerText_f()
{
    if (!alwaysfails)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 309, 0, "Cbuf_AddServerText_f was called.");
}

static cmd_function_s *sv_cmd_functions;

void __cdecl Cmd_ExecuteServerString(char *text)
{
    const char *arg0; // [esp+0h] [ebp-Ch]

   // if (!PbTrapPreExecCmd(text)) // LWSS: Punk buster
    {
        SV_Cmd_TokenizeString(text);
        if (SV_Cmd_Argc())
        {
            arg0 = SV_Cmd_Argv(0);
            for (cmd_function_s *itr= sv_cmd_functions; itr->next; itr = itr->next)
            {
                if (!I_stricmp(arg0, itr->name))
                {
                    //*prev = cmd->next;
                    //cmd->next = sv_cmd_functions;
                    //sv_cmd_functions = cmd;
                    if (itr->function)
                        itr->function();
                    break;
                }
            }
        }
        SV_Cmd_EndTokenizedString();
    }
}

void __cdecl Cbuf_SV_Execute()
{
    char v0; // [esp+0h] [ebp-1010h]
    int32_t  count; // [esp+4h] [ebp-100Ch]
    uint32_t counta; // [esp+4h] [ebp-100Ch]
    char dst[4096]; // [esp+8h] [ebp-1008h] BYREF
    uint8_t *src; // [esp+100Ch] [ebp-4h]

    while (sv_cmd_text.cmdsize)
    {
        src = sv_cmd_text.data;
        v0 = 0;
        for (count = 0; count < sv_cmd_text.cmdsize; ++count)
        {
            if (src[count] == 34)
                ++v0;
            if ((v0 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13)
                break;
        }
        if (count >= 4095)
            count = 4095;
        memcpy((uint8_t *)dst, src, count);
        dst[count] = 0;
        if (count == sv_cmd_text.cmdsize)
        {
            sv_cmd_text.cmdsize = 0;
        }
        else
        {
            counta = count + 1;
            sv_cmd_text.cmdsize -= counta;
            memmove(src, &src[counta], sv_cmd_text.cmdsize);
        }
        SV_WaitServer();

        iassert( !com_inServerFrame );

        Cmd_ExecuteServerString(dst);
    }
}

void __cdecl Cmd_AddServerCommandInternal(const char *cmdName, void(__cdecl *function)(), cmd_function_s *allocedCmd)
{
    cmd_function_s *cmd; // [esp+14h] [ebp-4h]

    iassert( cmdName );
    for (cmd = sv_cmd_functions; ; cmd = cmd->next)
    {
        if (!cmd)
        {
            allocedCmd->name = cmdName;
            allocedCmd->function = function;
            allocedCmd->next = sv_cmd_functions;
            sv_cmd_functions = allocedCmd;
            return;
        }
        if (!strcmp(cmdName, cmd->name))
            break;
    }
    iassert( cmd == allocedCmd );
    if (function)
        Com_Printf(16, "Cmd_AddServerCommand: %s already defined\n", cmdName);
}

void __cdecl Cbuf_ExecuteBuffer(int32_t  localClientNum, int32_t  controllerIndex, const char *buffer)
{
    char v3; // [esp+10h] [ebp-1018h]
    int32_t  v4; // [esp+14h] [ebp-1014h]
    char dst[4100]; // [esp+18h] [ebp-1010h] BYREF
    uint32_t count; // [esp+1020h] [ebp-8h]
    uint8_t *src; // [esp+1024h] [ebp-4h]

    iassert( buffer );
    src = (uint8_t *)buffer;
    v4 = strlen(buffer);
    while (v4)
    {
        v3 = 0;
        for (count = 0; (int32_t )count < v4; ++count)
        {
            if (src[count] == 34)
                ++v3;
            if ((v3 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13)
                break;
        }
        if ((int32_t )count >= 4095)
            count = 4095;
        memcpy((uint8_t *)dst, src, count);
        dst[count] = 0;
        if (count != v4)
            ++count;
        src += count;
        v4 -= count;
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, dst);
    }
}

void __cdecl Cbuf_Execute(int32_t  localClientNum, int32_t  controllerIndex)
{
    PROF_SCOPED("Cbuf_Execute");
    if (cmd_insideCBufExecute[localClientNum])
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            583,
            0,
            "%s\n\t%s",
            "!cmd_insideCBufExecute[localClientNum]",
            "Nesting Cbuf_Execute() is not allowed.");
    cmd_insideCBufExecute[localClientNum] = 1;
    Cbuf_ExecuteInternal(localClientNum, controllerIndex);
    cmd_insideCBufExecute[localClientNum] = 0;
    Cbuf_SV_Execute();
}

void __cdecl Cbuf_ExecuteInternal(int32_t  localClientNum, int32_t  controllerIndex)
{
    char v2; // [esp+0h] [ebp-1014h]
    CmdText *v3; // [esp+4h] [ebp-1010h]
    int32_t  count; // [esp+8h] [ebp-100Ch]
    uint32_t counta; // [esp+8h] [ebp-100Ch]
    char dst[4096]; // [esp+Ch] [ebp-1008h] BYREF
    uint8_t *src; // [esp+1010h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    v3 = &cmd_textArray[localClientNum];
    while (v3->cmdsize)
    {
        if (cmd_wait)
        {
            --cmd_wait;
            break;
        }
        src = v3->data;
        v2 = 0;
        for (count = 0; count < v3->cmdsize; ++count)
        {
            if (src[count] == 34)
                ++v2;
            if ((v2 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13)
                break;
        }
        if (count >= 4095)
            count = 4095;
        memcpy((uint8_t *)dst, src, count);
        dst[count] = 0;
        if (count == v3->cmdsize)
        {
            v3->cmdsize = 0;
        }
        else
        {
            counta = count + 1;
            v3->cmdsize -= counta;
            memmove(src, &src[counta], v3->cmdsize);
        }
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, dst);
        Sys_EnterCriticalSection(CRITSECT_CBUF);
    }
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl _Cmd_Vstr_f()
{
    const char *v0; // eax
    const char *dvarName; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        dvarName = Cmd_Argv(1);
        dvar = Dvar_FindVar(dvarName);
        if (dvar)
        {
            if (dvar->type == 7 || dvar->type == 6)
            {
                v0 = va("%s\n", dvar->current.string);
                Cbuf_InsertText(0, v0);
            }
            else
            {
                Com_Printf(0, "%s is not a string-based dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "%s doesn't exist\n", dvarName);
        }
    }
    else
    {
        Com_Printf(0, "vstr <variablename> : execute a variable command\n");
    }
}

void __cdecl SVCmd_ArgvBuffer(int32_t  arg, char *buffer, int32_t  bufferLength)
{
    char *v3; // eax

    v3 = (char *)SV_Cmd_Argv(arg);
    I_strncpyz(buffer, v3, bufferLength);
}

void __cdecl Cmd_ArgsBuffer(int32_t  start, char *buffer, int32_t  bufLength)
{
    const char *src; // [esp+0h] [ebp-14h]
    int32_t  argIndex; // [esp+4h] [ebp-10h]
    const char **argv; // [esp+8h] [ebp-Ch]
    char *dst; // [esp+Ch] [ebp-8h]
    int32_t  argc; // [esp+10h] [ebp-4h]
    int32_t  bufLengtha; // [esp+24h] [ebp+10h]

    iassert( Sys_IsMainThread() );
    iassert( start >= 0 );
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            775,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    iassert( buffer );
    iassert( bufLength >= 2 );
    dst = buffer;
    bufLengtha = bufLength - 1;
    argc = cmd_args.argc[cmd_args.nesting];
    argv = cmd_args.argv[cmd_args.nesting];
    for (argIndex = start; argIndex < argc; ++argIndex)
    {
        for (src = argv[argIndex]; *src; ++src)
        {
            *dst++ = *src;
            if (!--bufLengtha)
            {
                *dst = 0;
                return;
            }
        }
        if (bufLengtha == 1)
        {
            *dst = 0;
            return;
        }
        if (argIndex != argc - 1)
        {
            *dst++ = 32;
            --bufLengtha;
        }
    }
    *dst = 0;
}

void __cdecl Cmd_TokenizeStringWithLimit(char *text_in, int32_t  max_tokens)
{
    Cmd_TokenizeStringKernel(text_in, max_tokens, &cmd_args, &cmd_argsPrivate);
}

void __cdecl Cmd_TokenizeStringKernel(char *text_in, int32_t  max_tokens, CmdArgs *args, CmdArgsPrivate *argsPriv)
{
    if (max_tokens > 512 - argsPriv->totalUsedArgvPool)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            989,
            0,
            "max_tokens <= MAX_TOKENS_SIZE - argsPriv->totalUsedArgvPool\n\t%i, %i",
            max_tokens,
            512 - argsPriv->totalUsedArgvPool);
    AssertCmdArgsConsistency(args, argsPriv);
    if (++args->nesting >= 8u)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            994,
            0,
            "args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            args->nesting,
            8);
    argsPriv->usedTextPool[args->nesting] = -argsPriv->totalUsedTextPool;
    args->localClientNum[args->nesting] = -1;
    args->controllerIndex[args->nesting] = 0;
    args->argv[args->nesting] = &argsPriv->argvPool[argsPriv->totalUsedArgvPool];
    args->argc[args->nesting] = Cmd_TokenizeStringInternal(text_in, max_tokens, args->argv[args->nesting], argsPriv);
    argsPriv->totalUsedArgvPool += args->argc[args->nesting];
    argsPriv->usedTextPool[args->nesting] += argsPriv->totalUsedTextPool;
    AssertCmdArgsConsistency(args, argsPriv);
}

int32_t  __cdecl Cmd_TokenizeStringInternal(char *text_in, int32_t  max_tokens, const char **argv, CmdArgsPrivate *argsPriv)
{
    int32_t  v5; // [esp+0h] [ebp-44h]
    int32_t  v6; // [esp+4h] [ebp-40h]
    int32_t  v7; // [esp+8h] [ebp-3Ch]
    int32_t  v8; // [esp+Ch] [ebp-38h]
    int32_t  v9; // [esp+10h] [ebp-34h]
    int32_t  v10; // [esp+14h] [ebp-30h]
    uint8_t *text; // [esp+3Ch] [ebp-8h]
    const char *texta; // [esp+3Ch] [ebp-8h]
    const char *textb; // [esp+3Ch] [ebp-8h]
    int32_t  argc; // [esp+40h] [ebp-4h]

    iassert( text_in );
    argc = 0;
    text = (uint8_t *)text_in;
    while (1)
    {
        while (1)
        {
            while (1)
            {
                if (!*text)
                    return argc;
                if (!Cmd_IsWhiteSpaceChar(*text))
                    break;
                ++text;
            }
            if (*text == 47 && text[1] == 47)
                return argc;
            if (*text != 47 || text[1] != 42)
                break;
            for (texta = (const char *)(text + 2); *texta && (*texta != 42 || texta[1] != 47); ++texta)
                ;
            if (!*texta)
                return argc;
            text = (uint8_t *)(texta + 2);
        }
        argv[argc++] = &argsPriv->textPool[argsPriv->totalUsedTextPool];
        if (!--max_tokens)
            break;
        if (*text == 34)
        {
            for (textb = (const char *)(text + 1); *textb && *textb != 34; ++textb)
            {
                if (*textb == 92 && textb[1] == 34)
                    ++textb;
                argsPriv->textPool[argsPriv->totalUsedTextPool] = *textb;
                if (argsPriv->totalUsedTextPool + 1 < 8190)
                    v9 = argsPriv->totalUsedTextPool + 1;
                else
                    v9 = 8190;
                argsPriv->totalUsedTextPool = v9;
            }
            argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
            if (argsPriv->totalUsedTextPool + 1 < 8190)
                v8 = argsPriv->totalUsedTextPool + 1;
            else
                v8 = 8190;
            argsPriv->totalUsedTextPool = v8;
            if (!*textb)
                return argc;
            text = (uint8_t *)(textb + 1);
            if (!*text)
                return argc;
        }
        else
        {
            do
            {
                argsPriv->textPool[argsPriv->totalUsedTextPool] = *text;
                if (argsPriv->totalUsedTextPool + 1 < 8190)
                    v7 = argsPriv->totalUsedTextPool + 1;
                else
                    v7 = 8190;
                argsPriv->totalUsedTextPool = v7;
                if (!*++text)
                {
                    argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
                    if (argsPriv->totalUsedTextPool + 1 < 8190)
                        v6 = argsPriv->totalUsedTextPool + 1;
                    else
                        v6 = 8190;
                    argsPriv->totalUsedTextPool = v6;
                    return argc;
                }
            } while (!Cmd_IsWhiteSpaceChar(*text) && (*text != 47 || text[1] != 47 && text[1] != 42));
            argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
            if (argsPriv->totalUsedTextPool + 1 < 8190)
                v5 = argsPriv->totalUsedTextPool + 1;
            else
                v5 = 8190;
            argsPriv->totalUsedTextPool = v5;
        }
    }
    while (1)
    {
        argsPriv->textPool[argsPriv->totalUsedTextPool] = *text;
        v10 = argsPriv->totalUsedTextPool + 1 < 8190 ? argsPriv->totalUsedTextPool + 1 : 8190;
        argsPriv->totalUsedTextPool = v10;
        if (!*text)
            break;
        ++text;
    }
    return argc;
}

bool __cdecl Cmd_IsWhiteSpaceChar(uint8_t letter)
{
    iassert( letter != '\\0' );
    return letter != 20 && letter != 21 && letter != 22 && letter <= 0x20u;
}

void __cdecl AssertCmdArgsConsistency(const CmdArgs *args, const CmdArgsPrivate *argsPriv)
{
    const char *v2; // eax
    int32_t  totalUsedTextPool; // [esp+0h] [ebp-10h]
    int32_t  totalUsedArgvPool; // [esp+4h] [ebp-Ch]
    int32_t  arg; // [esp+8h] [ebp-8h]
    int32_t  nesting; // [esp+Ch] [ebp-4h]
    int32_t  nestinga; // [esp+Ch] [ebp-4h]

    totalUsedArgvPool = 0;
    totalUsedTextPool = 0;
    for (nesting = 0; nesting <= args->nesting; ++nesting)
    {
        totalUsedArgvPool += args->argc[nesting];
        totalUsedTextPool += argsPriv->usedTextPool[nesting];
    }
    if (totalUsedArgvPool != argsPriv->totalUsedArgvPool)
    {
        Com_Printf(16, "About to assert totalUsedArgvPool\n");
        for (nestinga = 0; nestinga <= args->nesting; ++nestinga)
        {
            for (arg = 0; arg < args->argc[nestinga]; ++arg)
                Com_Printf(16, "nesting %i, arg %i: '%s'\n", nestinga, arg, args->argv[nestinga][arg]);
        }
    }
    if (totalUsedArgvPool != argsPriv->totalUsedArgvPool)
    {
        v2 = va(
            "totalUsedArgvPool is %i, argsPriv->totalUsedArgvPool is %i, args->nesting is %i, args->argc[0] is %i, args->a"
            "rgc[1] is %i, args->argc[2] is %i, args->argc[3] is %i",
            totalUsedArgvPool,
            argsPriv->totalUsedArgvPool,
            args->nesting,
            args->argc[0],
            args->argc[1],
            args->argc[2],
            args->argc[3]);
        MyAssertHandler(".\\qcommon\\cmd.cpp", 979, 0, "%s\n\t%s", "totalUsedArgvPool == argsPriv->totalUsedArgvPool", v2);
    }
    if (totalUsedTextPool != argsPriv->totalUsedTextPool)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            980,
            0,
            "totalUsedTextPool == argsPriv->totalUsedTextPool\n\t%i, %i",
            totalUsedTextPool,
            argsPriv->totalUsedTextPool);
}

void __cdecl Cmd_TokenizeString(char *text_in)
{
    Cmd_TokenizeStringWithLimit(text_in, 512 - cmd_argsPrivate.totalUsedArgvPool);
}

void __cdecl Cmd_EndTokenizedString()
{
    Cmd_EndTokenizedStringKernel(&cmd_args, &cmd_argsPrivate);
}

void __cdecl Cmd_EndTokenizedStringKernel(CmdArgs *args, CmdArgsPrivate *argsPriv)
{
    AssertCmdArgsConsistency(args, argsPriv);
    if (args->nesting >= 8u)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            1014,
            0,
            "args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            args->nesting,
            8);
    argsPriv->totalUsedArgvPool -= args->argc[args->nesting];
    argsPriv->totalUsedTextPool -= argsPriv->usedTextPool[args->nesting--];
    AssertCmdArgsConsistency(args, argsPriv);
}

void __cdecl SV_Cmd_TokenizeString(char *text_in)
{
    Cmd_TokenizeStringKernel(text_in, 512 - sv_cmd_argsPrivate.totalUsedArgvPool, &sv_cmd_args, &sv_cmd_argsPrivate);
}

void __cdecl SV_Cmd_EndTokenizedString()
{
    Cmd_EndTokenizedStringKernel(&sv_cmd_args, &sv_cmd_argsPrivate);
}

void __cdecl Cmd_RemoveCommand(const char *cmdName)
{
    cmd_function_s **back; // [esp+14h] [ebp-8h]
    cmd_function_s *cmd; // [esp+18h] [ebp-4h]

    for (back = &cmd_functions; ; back = (cmd_function_s **)*back)
    {
        cmd = *back;
        if (!*back)
            break;
        if (!strcmp(cmdName, cmd->name))
        {
            *back = cmd->next;
            return;
        }
    }
}

void __cdecl Cmd_SetAutoComplete(const char *cmdName, const char *dir, const char *ext)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    iassert( cmdName );
    iassert( dir );
    iassert( ext );
    cmd = _Cmd_FindCommand(cmdName);
    iassert( cmd );
    iassert( cmd->autoCompleteDir == NULL );
    iassert( cmd->autoCompleteExt == NULL );
    cmd->autoCompleteDir = dir;
    cmd->autoCompleteExt = ext;
}

void __cdecl Cmd_Shutdown()
{
    cmd_functions = 0;
    sv_cmd_functions = 0;
}

void __cdecl Cmd_ForEach(void(__cdecl *callback)(const char *))
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    for (cmd = cmd_functions; cmd; cmd = cmd->next)
        callback(cmd->name);
}

void __cdecl Cmd_ComErrorCleanup()
{
    int32_t  client; // [esp+0h] [ebp-4h]

    Cmd_ResetArgs(&cmd_args, &cmd_argsPrivate);
    Cmd_ResetArgs(&sv_cmd_args, &sv_cmd_argsPrivate);
    for (client = 0; client < 1; ++client)
        cmd_insideCBufExecute[client] = 0;
}

void __cdecl Cmd_ExecuteSingleCommand(int32_t  localClientNum, int32_t  controllerIndex, char *text)
{
    const char *arg0; // [esp+20h] [ebp-Ch]
    cmd_function_s *itr; // [esp+28h] [ebp-4h]

    iassert( Sys_IsMainThread() );
    if (localClientNum)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            1333,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    //if (!PbTrapPreExecCmd(text))
    {
        Cmd_TokenizeString(text);
        if (Cmd_Argc())
        {
            cmd_args.localClientNum[cmd_args.nesting] = localClientNum;
            cmd_args.controllerIndex[cmd_args.nesting] = controllerIndex;
            arg0 = Cmd_Argv(0);
            for (itr = cmd_functions; itr->next; itr = itr->next)
            {
                if (!I_stricmp(arg0, itr->name))
                {
                    //prev->next = cmd->next;
                    //cmd->next = cmd_functions;
                    //cmd_functions = cmd;
                    if (itr->function)
                    {
                        if (itr->function == Cbuf_AddServerText_f)
                        {
                            SV_WaitServer();
                            iassert( !com_inServerFrame );
                            Cmd_ExecuteServerString(text);
                        }
                        else
                        {
                            itr->function();
                        }
                        goto LABEL_26;
                    }
                    break;
                }
            }
            if (I_strnicmp(text, "pb_", 3))
            {
                if (!Dvar_Command() && (!com_sv_running || !com_sv_running->current.enabled || !SV_GameCommand()))
                {
                    CL_ForwardCommandToServer(localClientNum, text);
                    Cmd_EndTokenizedString();
                    return;
                }
            }
            else if (I_strnicmp(text + 3, "sv_", 3))
            {
                //PbClAddEvent(14, strlen(text) + 1, text);
            }
            else
            {
                //PbSvAddEvent(14, -1, strlen(text) + 1, text);
            }
        }
    LABEL_26:
        Cmd_EndTokenizedString();
    }
}

void __cdecl SV_Cmd_ExecuteString(int32_t  localClientNum, int32_t  controllerIndex, char *text)
{
    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, text);
}

void __cdecl Cmd_List_f()
{
    const char *match; // [esp+0h] [ebp-Ch]
    int32_t  i; // [esp+4h] [ebp-8h]
    cmd_function_s *cmd; // [esp+8h] [ebp-4h]

    if (Cmd_Argc() <= 1)
        match = 0;
    else
        match = Cmd_Argv(1);
    i = 0;
    for (cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if (!match || Com_Filter(match, (char *)cmd->name, 0))
        {
            Com_Printf(0, "%s\n", cmd->name);
            ++i;
        }
    }
    Com_Printf(0, "%i commands\n", i);
}

void __cdecl Cmd_Exec_f()
{
    char *v0; // eax
    const char *v1; // eax
    char *pathname; // [esp+4h] [ebp-4Ch]
    char filename[64]; // [esp+8h] [ebp-48h] BYREF
    int32_t  localClientNum; // [esp+4Ch] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        v0 = (char *)Cmd_Argv(1);
        I_strncpyz(filename, v0, 64);
        Com_DefaultExtension(filename, 0x40u, ".cfg");
        localClientNum = 0;
        pathname = (char *)Com_GetFilenameSubString(filename);
        if (I_stricmp(pathname, "config_mp.cfg"))
        {
            if ((!IsFastFileLoad() || !Cmd_ExecFromFastFile(localClientNum, 0, filename))
                && !Cmd_ExecFromDisk(localClientNum, 0, filename))
            {
                v1 = Cmd_Argv(1);
                Com_PrintError(1, "couldn't exec %s\n", v1);
            }
        }
        else
        {
            Cmd_ExecFromDisk(localClientNum, 0, filename);
        }
    }
    else
    {
        Com_Printf(0, "exec <filename> : execute a script file\n");
    }
}

char __cdecl Cmd_ExecFromDisk(int32_t  localClientNum, int32_t  controllerIndex, const char *filename)
{
    char *text; // [esp+0h] [ebp-4h] BYREF

    FS_ReadFile(filename, (void **)&text);
    if (!text)
        return 0;
    Com_Printf(16, "execing %s from disk\n", filename);
    Cbuf_ExecuteBuffer(localClientNum, controllerIndex, text);
    FS_FreeFile(text);
    return 1;
}

char __cdecl Cmd_ExecFromFastFile(int32_t  localClientNum, int32_t  controllerIndex, const char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    if (!DB_IsMinimumFastFileLoaded())
        return 0;
    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename).rawfile;
    if (!rawfile)
        return 0;
    Com_Printf(16, "execing %s from fastfile\n", filename);
    Cbuf_ExecuteBuffer(localClientNum, controllerIndex, (char *)rawfile->buffer);
    return 1;
}

void __cdecl SV_Cmd_ArgvBuffer(int32_t  arg, char *buffer, int32_t  bufferLength)
{
    char *v3; // eax

    v3 = (char *)SV_Cmd_Argv(arg);
    I_strncpyz(buffer, v3, bufferLength);
}

#ifdef KISAK_SP
#include <script/scr_vm.h>
#include <game/g_local.h>

struct CmdScriptNotify
{
    unsigned __int16 command;
    unsigned __int16 notify;
};

int dword_8359AD3C;
CmdScriptNotify cmd_notify[64]{ 0 };

void Cmd_RegisterNotification(const char *commandString, const char *notifyString)
{
    unsigned int LowercaseString; // r30
    unsigned int String; // r3
    unsigned int v5; // r26
    unsigned int v6; // r9
    int v7; // r11
    unsigned __int16 *p_notify; // r10

    LowercaseString = SL_GetLowercaseString(commandString, 0);
    String = SL_GetString(notifyString, 0);
    v5 = String;
    v6 = 0;
    v7 = dword_8359AD3C;
    if (dword_8359AD3C)
    {
        p_notify = &cmd_notify[0].notify;
        while (*(p_notify - 1) != LowercaseString || *p_notify != String)
        {
            ++v6;
            p_notify += 2;
            if (v6 >= dword_8359AD3C)
                goto LABEL_6;
        }
        SL_RemoveRefToString(LowercaseString);
        SL_RemoveRefToString(v5);
    }
    else
    {
    LABEL_6:
        if (dword_8359AD3C == 64)
        {
            Scr_Error(va("Cannot currently register more than %i commands\n", 64));
            v7 = dword_8359AD3C;
        }

        if (LowercaseString != (unsigned __int16)LowercaseString)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                LowercaseString,
                (unsigned __int16)LowercaseString);
            v7 = dword_8359AD3C;
        }
        cmd_notify[v7].command = LowercaseString;
        if (v5 != (unsigned __int16)v5)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                v5,
                (unsigned __int16)v5);
            v7 = dword_8359AD3C;
        }
        cmd_notify[v7].notify = v5;
        dword_8359AD3C = v7 + 1;
    }
}
void Cmd_CheckNotify()
{
    const char *v0; // r3
    unsigned int LowercaseString; // r28
    unsigned int v2; // r11
    unsigned int v3; // r30
    unsigned __int16 *p_notify; // r31

    iassert(Sys_IsMainThread());

    if (dword_8359AD3C)
    {
        if (!cl_paused->current.integer)
        {
            v0 = Cmd_Argv(0);
            LowercaseString = SL_FindLowercaseString(v0);
            if (LowercaseString)
            {
                v2 = dword_8359AD3C;
                v3 = 0;
                if (dword_8359AD3C)
                {
                    p_notify = &cmd_notify[0].notify;
                    do
                    {
                        if (LowercaseString == *(p_notify - 1))
                        {
                            G_AddCommandNotify(*p_notify);
                            v2 = dword_8359AD3C;
                        }
                        ++v3;
                        p_notify += 2;
                    } while (v3 < v2);
                }
            }
        }
    }
}
void Cmd_LoadNotifications(MemoryFile *memFile)
{
    int v2; // r24
    int v3; // r26
    unsigned __int16 *p_notify; // r30
    const char *CString; // r3
    unsigned int String; // r7
    unsigned __int16 v7; // r31
    const char *v8; // r3
    unsigned int v9; // r7
    unsigned __int16 v10; // r31
    int v11; // [sp+50h] [-50h] BYREF

    dword_8359AD3C = 0;
    MemFile_ReadData(memFile, 4, (unsigned char*)&v11);
    v2 = v11;
    if (v11)
    {
        v3 = v11;
        p_notify = &cmd_notify[0].notify;
        do
        {
            CString = MemFile_ReadCString(memFile);
            String = SL_GetString(CString, 0);
            v7 = String;
            if (String != (unsigned __int16)String)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
                    281,
                    0,
                    "i == static_cast< Type >( i )\n\t%i, %i",
                    String,
                    (unsigned __int16)String);
            *(p_notify - 1) = v7;
            v8 = MemFile_ReadCString(memFile);
            v9 = SL_GetString(v8, 0);
            v10 = v9;
            if (v9 != (unsigned __int16)v9)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
                    281,
                    0,
                    "i == static_cast< Type >( i )\n\t%i, %i",
                    v9,
                    (unsigned __int16)v9);
            --v3;
            *p_notify = v10;
            p_notify += 2;
        } while (v3);
    }
    dword_8359AD3C = v2;
}
void Cmd_SaveNotifications(MemoryFile *memFile)
{
    unsigned int v2; // r30
    unsigned __int16 *p_notify; // r31
    const char *v4; // r3
    const char *v5; // r3
    int v6; // [sp+50h] [-30h] BYREF

    v6 = dword_8359AD3C;
    MemFile_WriteData(memFile, 4, &v6);
    v2 = 0;
    if (dword_8359AD3C)
    {
        p_notify = &cmd_notify[0].notify;
        do
        {
            v4 = SL_ConvertToString(*(p_notify - 1));
            MemFile_WriteCString(memFile, v4);
            v5 = SL_ConvertToString(*p_notify);
            MemFile_WriteCString(memFile, v5);
            ++v2;
            p_notify += 2;
        } while (v2 < dword_8359AD3C);
    }
}
void Cmd_UnregisterAllNotifications()
{
    unsigned int v0; // r30
    unsigned __int16 *p_notify; // r31

    iassert(Sys_IsMainThread());

    v0 = 0;
    if (dword_8359AD3C)
    {
        p_notify = &cmd_notify[0].notify;
        do
        {
            SL_RemoveRefToString(*(p_notify - 1));
            SL_RemoveRefToString(*p_notify);
            ++v0;
            p_notify += 2;
        } while (v0 < dword_8359AD3C);
    }
    dword_8359AD3C = 0;
}
#endif