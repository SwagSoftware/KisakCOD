#include "cmd.h"

#include <universal/assertive.h>

#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"
#include <win32/win_local.h>

static cmd_function_s* cmd_functions = NULL;


static cmd_function_s Cmd_Wait_f_VAR;
static cmd_function_s Cmd_List_f_VAR;
static cmd_function_s Cmd_Exec_f_VAR;
static cmd_function_s Cmd_Vstr_f_VAR;

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
unsigned __int8 cmd_text_buf[1][65536];
unsigned __int8 sv_cmd_text_buf[65536];

int marker_cmd;
int cmd_wait;
BOOL *cmd_insideCBufExecute;

/*
============
Cmd_Argc
============
*/
int	Cmd_Argc(void) {
	iassert(cmd_args.nesting > 0 && cmd_args.nesting < 8);
	return cmd_args.argc[cmd_args.nesting];
}

/*
============
Cmd_Argv
============
*/
const char* Cmd_Argv(int arg) {
	iassert(cmd_args.nesting > 0 && cmd_args.nesting < 8);
	iassert(arg >= 0);

	if ((unsigned)arg >= cmd_args.argc[cmd_args.nesting])
	{
		return (char*)"";
	}
	return (char*)(cmd_args.argv[cmd_args.nesting][arg]);
}

int __cdecl SV_Cmd_Argc()
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

const char *__cdecl SV_Cmd_Argv(int argIndex)
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

void Cmd_Init()
{
	Cmd_ResetArgs(&cmd_args, &cmd_argsPrivate);
	Cmd_ResetArgs(&sv_cmd_args, &sv_cmd_argsPrivate);
	Cmd_AddCommandInternal("cmdlist", Cmd_List_f, &Cmd_List_f_VAR);
	Cmd_AddCommandInternal("exec", Cmd_Exec_f, &Cmd_Exec_f_VAR);
	Cmd_AddCommandInternal("vstr", Cmd_Vstr_f, &Cmd_Vstr_f_VAR);
	Cmd_AddCommandInternal("wait", Cmd_Wait_f, &Cmd_Wait_f_VAR);
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
    int client; // [esp+0h] [ebp-4h]

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

void __cdecl Cbuf_AddText(int localClientNum, const char *text)
{
    CmdText *cmd_text; // [esp+0h] [ebp-8h]
    int length; // [esp+4h] [ebp-4h]

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

void __cdecl memcpy_noncrt(void *dst, const void *src, unsigned int length)
{
    memcpy(dst, src, length);
}

int __cdecl strlen_noncrt(const char *str)
{
    int count; // [esp+0h] [ebp-4h]

    count = 0;
    while (*str)
    {
        ++count;
        ++str;
    }
    return count;
}

void __cdecl Cbuf_InsertText(int localClientNum, char *text)
{
    unsigned int v2; // [esp+4h] [ebp-1Ch]
    CmdText *cmd_text; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int length; // [esp+1Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    cmd_text = &cmd_textArray[localClientNum];
    v2 = strlen(text);
    length = v2 + 1;
    if ((signed int)(cmd_text->cmdsize + v2 + 1) <= cmd_text->maxsize)
    {
        for (i = cmd_text->cmdsize - 1; i >= 0; --i)
            cmd_text->data[length + i] = cmd_text->data[i];
        memcpy(cmd_text->data, (unsigned __int8 *)text, v2);
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
    cmd_function_s **prev; // [esp+4h] [ebp-8h]
    cmd_function_s *cmd; // [esp+8h] [ebp-4h]

   // if (!PbTrapPreExecCmd(text)) // LWSS: Punk buster
    {
        SV_Cmd_TokenizeString(text);
        if (SV_Cmd_Argc())
        {
            arg0 = SV_Cmd_Argv(0);
            for (prev = &sv_cmd_functions; *prev; prev = &cmd->next)
            {
                cmd = *prev;
                if (!I_stricmp(arg0, (*prev)->name))
                {
                    *prev = cmd->next;
                    cmd->next = sv_cmd_functions;
                    sv_cmd_functions = cmd;
                    if (cmd->function)
                        cmd->function();
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
    int count; // [esp+4h] [ebp-100Ch]
    unsigned int counta; // [esp+4h] [ebp-100Ch]
    char dst[4096]; // [esp+8h] [ebp-1008h] BYREF
    unsigned __int8 *src; // [esp+100Ch] [ebp-4h]

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
        memcpy((unsigned __int8 *)dst, src, count);
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

        if (com_inServerFrame)
            MyAssertHandler(".\\qcommon\\cmd.cpp", 419, 0, "%s", "!com_inServerFrame");

        Cmd_ExecuteServerString(dst);
    }
}

void __cdecl Cmd_AddServerCommandInternal(const char *cmdName, void(__cdecl *function)(), cmd_function_s *allocedCmd)
{
    cmd_function_s *cmd; // [esp+14h] [ebp-4h]

    if (!cmdName)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 436, 0, "%s", "cmdName");
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
    if (cmd != allocedCmd)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 443, 0, "%s", "cmd == allocedCmd");
    if (function)
        Com_Printf(16, "Cmd_AddServerCommand: %s already defined\n", cmdName);
}

void __cdecl Cbuf_ExecuteBuffer(int localClientNum, int controllerIndex, char *buffer)
{
    char v3; // [esp+10h] [ebp-1018h]
    signed int v4; // [esp+14h] [ebp-1014h]
    char dst[4100]; // [esp+18h] [ebp-1010h] BYREF
    unsigned int count; // [esp+1020h] [ebp-8h]
    unsigned __int8 *src; // [esp+1024h] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 467, 0, "%s", "buffer");
    src = (unsigned __int8 *)buffer;
    v4 = strlen(buffer);
    while (v4)
    {
        v3 = 0;
        for (count = 0; (int)count < v4; ++count)
        {
            if (src[count] == 34)
                ++v3;
            if ((v3 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13)
                break;
        }
        if ((int)count >= 4095)
            count = 4095;
        memcpy((unsigned __int8 *)dst, src, count);
        dst[count] = 0;
        if (count != v4)
            ++count;
        src += count;
        v4 -= count;
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, dst);
    }
}

void __cdecl Cbuf_Execute(int localClientNum, int controllerIndex)
{
    //Profile_Begin(28);
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
    //Profile_EndInternal(0);
}

void __cdecl Cbuf_ExecuteInternal(int localClientNum, int controllerIndex)
{
    char v2; // [esp+0h] [ebp-1014h]
    CmdText *v3; // [esp+4h] [ebp-1010h]
    int count; // [esp+8h] [ebp-100Ch]
    unsigned int counta; // [esp+8h] [ebp-100Ch]
    char dst[4096]; // [esp+Ch] [ebp-1008h] BYREF
    unsigned __int8 *src; // [esp+1010h] [ebp-4h]

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
        memcpy((unsigned __int8 *)dst, src, count);
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

void __cdecl SVCmd_ArgvBuffer(int arg, char *buffer, int bufferLength)
{
    char *v3; // eax

    v3 = (char *)SV_Cmd_Argv(arg);
    I_strncpyz(buffer, v3, bufferLength);
}

void __cdecl Cmd_ArgsBuffer(int start, char *buffer, int bufLength)
{
    const char *src; // [esp+0h] [ebp-14h]
    int argIndex; // [esp+4h] [ebp-10h]
    const char **argv; // [esp+8h] [ebp-Ch]
    char *dst; // [esp+Ch] [ebp-8h]
    int argc; // [esp+10h] [ebp-4h]
    int bufLengtha; // [esp+24h] [ebp+10h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\cmd.cpp", 773, 0, "%s", "Sys_IsMainThread()");
    if (start < 0)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 774, 0, "%s", "start >= 0");
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            775,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    if (!buffer)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 776, 0, "%s", "buffer");
    if (bufLength < 2)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 777, 0, "%s", "bufLength >= 2");
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

void __cdecl Cmd_TokenizeStringWithLimit(char *text_in, int max_tokens)
{
    Cmd_TokenizeStringKernel(text_in, max_tokens, &cmd_args, &cmd_argsPrivate);
}

void __cdecl Cmd_TokenizeStringKernel(char *text_in, int max_tokens, CmdArgs *args, CmdArgsPrivate *argsPriv)
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

int __cdecl Cmd_TokenizeStringInternal(char *text_in, int max_tokens, const char **argv, CmdArgsPrivate *argsPriv)
{
    int v5; // [esp+0h] [ebp-44h]
    int v6; // [esp+4h] [ebp-40h]
    int v7; // [esp+8h] [ebp-3Ch]
    int v8; // [esp+Ch] [ebp-38h]
    int v9; // [esp+10h] [ebp-34h]
    int v10; // [esp+14h] [ebp-30h]
    unsigned __int8 *text; // [esp+3Ch] [ebp-8h]
    const char *texta; // [esp+3Ch] [ebp-8h]
    const char *textb; // [esp+3Ch] [ebp-8h]
    int argc; // [esp+40h] [ebp-4h]

    if (!text_in)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 858, 0, "%s", "text_in");
    argc = 0;
    text = (unsigned __int8 *)text_in;
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
            text = (unsigned __int8 *)(texta + 2);
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
            text = (unsigned __int8 *)(textb + 1);
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

bool __cdecl Cmd_IsWhiteSpaceChar(unsigned __int8 letter)
{
    if (!letter)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 815, 1, "%s", "letter != '\\0'");
    return letter != 20 && letter != 21 && letter != 22 && letter <= 0x20u;
}

void __cdecl AssertCmdArgsConsistency(const CmdArgs *args, const CmdArgsPrivate *argsPriv)
{
    const char *v2; // eax
    int totalUsedTextPool; // [esp+0h] [ebp-10h]
    int totalUsedArgvPool; // [esp+4h] [ebp-Ch]
    int arg; // [esp+8h] [ebp-8h]
    int nesting; // [esp+Ch] [ebp-4h]
    int nestinga; // [esp+Ch] [ebp-4h]

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

void __cdecl Cmd_AddCommandInternal(const char *cmdName, void(__cdecl *function)(), cmd_function_s *allocedCmd)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    if (!cmdName)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1080, 0, "%s", "cmdName");
    cmd = _Cmd_FindCommand(cmdName);
    if (cmd)
    {
        if (cmd != allocedCmd)
            MyAssertHandler(".\\qcommon\\cmd.cpp", 1085, 0, "%s", "cmd == allocedCmd");
        if (function)
            Com_Printf(16, "Cmd_AddCommand: %s already defined\n", cmdName);
    }
    else
    {
        allocedCmd->name = cmdName;
        allocedCmd->function = function;
        allocedCmd->next = cmd_functions;
        cmd_functions = allocedCmd;
    }
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

    if (!cmdName)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1128, 0, "%s", "cmdName");
    if (!dir)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1129, 0, "%s", "dir");
    if (!ext)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1130, 0, "%s", "ext");
    cmd = _Cmd_FindCommand(cmdName);
    if (!cmd)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1134, 0, "%s", "cmd");
    if (cmd->autoCompleteDir)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1135, 0, "%s", "cmd->autoCompleteDir == NULL");
    if (cmd->autoCompleteExt)
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1136, 0, "%s", "cmd->autoCompleteExt == NULL");
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
    int client; // [esp+0h] [ebp-4h]

    Cmd_ResetArgs(&cmd_args, &cmd_argsPrivate);
    Cmd_ResetArgs(&sv_cmd_args, &sv_cmd_argsPrivate);
    for (client = 0; client < 1; ++client)
        cmd_insideCBufExecute[client] = 0;
}

void __cdecl Cmd_ExecuteSingleCommand(int localClientNum, int controllerIndex, char *text)
{
    const char *arg0; // [esp+20h] [ebp-Ch]
    cmd_function_s **prev; // [esp+24h] [ebp-8h]
    cmd_function_s *cmd; // [esp+28h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\cmd.cpp", 1331, 0, "%s", "Sys_IsMainThread()");
    if (localClientNum)
        MyAssertHandler(
            ".\\qcommon\\cmd.cpp",
            1333,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    //if (!PbTrapPreExecCmd(text)) // LWSS: punkbuster
    {
        Cmd_TokenizeString(text);
        if (Cmd_Argc())
        {
            cmd_args.localClientNum[cmd_args.nesting] = localClientNum;
            cmd_args.controllerIndex[cmd_args.nesting] = controllerIndex;
            arg0 = Cmd_Argv(0);
            for (prev = &cmd_functions; *prev; prev = &cmd->next)
            {
                cmd = *prev;
                if (!I_stricmp(arg0, (*prev)->name))
                {
                    *prev = cmd->next;
                    cmd->next = cmd_functions;
                    cmd_functions = cmd;
                    if (cmd->function)
                    {
                        if (cmd->function == Cbuf_AddServerText_f)
                        {
                            SV_WaitServer();
                            if (com_inServerFrame)
                                MyAssertHandler(".\\qcommon\\cmd.cpp", 1379, 0, "%s", "!com_inServerFrame");
                            Cmd_ExecuteServerString(text);
                        }
                        else
                        {
                            cmd->function();
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
                //PbClAddEvent(14, strlen(text) + 1, text); // LWSS: punkbuster
            }
            else
            {
                //PbSvAddEvent(14, -1, strlen(text) + 1, text); // LWSS: punkbuster
            }
        }
    LABEL_26:
        Cmd_EndTokenizedString();
    }
}

void __cdecl SV_Cmd_ExecuteString(int localClientNum, int controllerIndex, char *text)
{
    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, text);
}

void __cdecl Cmd_List_f()
{
    const char *match; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
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
    int localClientNum; // [esp+4Ch] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        v0 = (char *)Cmd_Argv(1);
        I_strncpyz(filename, v0, 64);
        Com_DefaultExtension(filename, 0x40u, ".cfg");
        localClientNum = 0;
        pathname = (char *)Com_GetFilenameSubString(filename);
        if (I_stricmp(pathname, "config_mp.cfg"))
        {
            if ((!useFastFile->current.enabled || !Cmd_ExecFromFastFile(localClientNum, 0, filename))
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

char __cdecl Cmd_ExecFromDisk(int localClientNum, int controllerIndex, const char *filename)
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

char __cdecl Cmd_ExecFromFastFile(int localClientNum, int controllerIndex, const char *filename)
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

