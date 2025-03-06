#include <universal/assertive.h>

#include "qcommon.h"

static cmd_function_s* cmd_functions = NULL;

struct CmdArgs
{                                       
    int nesting;                        
    int localClientNum[8];              
    int controllerIndex[8];             
    int argc[8];                        
    const char **argv[8];               
};
struct CmdArgsPrivate
{                         
    char textPool[8192];
    const char *argvPool[512];
    int usedTextPool[8];
    int totalUsedArgvPool;
    int totalUsedTextPool;
};

static void Cmd_ResetArgs(CmdArgs* args, CmdArgsPrivate* argsPriv)
{
	args->nesting = -1;
	argsPriv->totalUsedArgvPool = 0;
	argsPriv->totalUsedTextPool = 0;
}

static CmdArgs cmd_args;
static int cmd_wait;

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
char* Cmd_Argv(int arg) {
	iassert(cmd_args.nesting > 0 && cmd_args.nesting < 8);
	iassert(arg >= 0);

	if ((unsigned)arg >= cmd_args.argc[cmd_args.nesting])
	{
		return (char*)"";
	}
	return (char*)(cmd_args.argv[cmd_args.nesting][arg]);
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