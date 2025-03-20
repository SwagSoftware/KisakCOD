#pragma once

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

struct cmd_function_s
{
    cmd_function_s *next;
    const char *name;
    const char *autoCompleteDir;
    const char *autoCompleteExt;
    void(__cdecl *function)();
};

struct CmdText // sizeof=0xC
{                                       // ...
    unsigned __int8 *data;              // ...
    int maxsize;                        // ...
    int cmdsize;                        // ...
};

extern CmdArgs cmd_args;

const char *__cdecl Cmd_Argv(int argIndex);
int __cdecl Cmd_Argc();

int __cdecl SV_Cmd_Argc();
const char *__cdecl SV_Cmd_Argv(int argIndex);

void __cdecl TRACK_cmd();
void __cdecl Cmd_Wait_f();
void __cdecl Cbuf_Init();
void __cdecl Cbuf_AddText(int localClientNum, const char *text);
void __cdecl memcpy_noncrt(void *dst, const void *src, unsigned int length);
int __cdecl strlen_noncrt(const char *str);
void __cdecl Cbuf_InsertText(int localClientNum, const char *text);
void __cdecl Cbuf_AddServerText_f();
void __cdecl Cmd_ExecuteServerString(char *text);
void __cdecl Cbuf_SV_Execute();
void __cdecl Cmd_AddServerCommandInternal(const char *cmdName, void(__cdecl *function)(), cmd_function_s *allocedCmd);
void __cdecl Cbuf_ExecuteBuffer(int localClientNum, int controllerIndex, char *buffer);
void __cdecl Cbuf_Execute(int localClientNum, int controllerIndex);
void __cdecl Cbuf_ExecuteInternal(int localClientNum, int controllerIndex);
void __cdecl Cmd_Vstr_f();
void __cdecl SVCmd_ArgvBuffer(int arg, char *buffer, int bufferLength);
void __cdecl Cmd_ArgsBuffer(int start, char *buffer, int bufLength);
void __cdecl Cmd_TokenizeStringWithLimit(char *text_in, int max_tokens);
void __cdecl Cmd_TokenizeStringKernel(char *text_in, int max_tokens, CmdArgs *args, CmdArgsPrivate *argsPriv);
int __cdecl Cmd_TokenizeStringInternal(char *text_in, int max_tokens, const char **argv, CmdArgsPrivate *argsPriv);
bool __cdecl Cmd_IsWhiteSpaceChar(unsigned __int8 letter);
void __cdecl AssertCmdArgsConsistency(const CmdArgs *args, const CmdArgsPrivate *argsPriv);
void __cdecl Cmd_TokenizeString(char *text_in);
void __cdecl Cmd_EndTokenizedString();
void __cdecl Cmd_EndTokenizedStringKernel(CmdArgs *args, CmdArgsPrivate *argsPriv);
void __cdecl SV_Cmd_TokenizeString(char *text_in);
void __cdecl SV_Cmd_EndTokenizedString();
cmd_function_s *__cdecl Cmd_FindCommand(const char *cmdName);
void __cdecl Cmd_AddCommandInternal(const char *cmdName, void(__cdecl *function)(), cmd_function_s *allocedCmd);
void __cdecl Cmd_RemoveCommand(const char *cmdName);
void __cdecl Cmd_SetAutoComplete(const char *cmdName, const char *dir, const char *ext);
void __cdecl Cmd_Shutdown();
void __cdecl Cmd_ForEach(void(__cdecl *callback)(const char *));
void __cdecl Cmd_ComErrorCleanup();
void __cdecl Cmd_ExecuteSingleCommand(int localClientNum, int controllerIndex, char *text);
void __cdecl SV_Cmd_ExecuteString(int localClientNum, int controllerIndex, char *text);
void __cdecl Cmd_List_f();
void __cdecl Cmd_Init();
void __cdecl Cmd_Exec_f();
char __cdecl Cmd_ExecFromDisk(int localClientNum, int controllerIndex, const char *filename);
char __cdecl Cmd_ExecFromFastFile(int localClientNum, int controllerIndex, const char *filename);

