#include "assertive.h"
#include <win32/win_local.h>
#include <cstdarg>

char message[1024];
int isHandlingAssert;

bool __cdecl QuitOnError();
void MyAssertHandler(const char *filename, int line, int type, const char *fmt, ...)
{
    __debugbreak(); // KISAKTODO
    //char shouldBreak; // [esp+3h] [ebp-5h]
    //va_list va; // [esp+20h] [ebp+18h] BYREF
    //
    //va_start(va, fmt);
    //Sys_EnterCriticalSection(CRITSECT_ASSERT);
    //_vsnprintf(message, 0x400u, fmt, va);
    //message[1023] = 0;
    //if (isHandlingAssert)
    //{
    //    CopyMessageToClipboard();
    //    AssertNotify(lastAssertType, RECURSIVE);
    //    BuildAssertMessage(message, filename, line, type, 1, assertMessage);
    //    if (isHandlingAssert == 1)
    //    {
    //        isHandlingAssert = 2;
    //        Com_Printf(16, "ASSERTBEGIN - ( Recursive assert )---------------------------------------------\n");
    //        Com_Printf(16, assertMessage);
    //        Com_Printf(16, "ASSERTEND - ( Recursive assert ) ----------------------------------------------\n\n");
    //    }
    //    exit(-1);
    //}
    //lastAssertType = type;
    //isHandlingAssert = 1;
    //FixWindowsDesktop();
    //BuildAssertMessage(message, filename, line, type, 1, assertMessage);
    //Com_Printf(16, "ASSERTBEGIN -------------------------------------------------------------------\n");
    //Com_Printf(16, "%s", assertMessage);
    //Com_Printf(16, "ASSERTEND ---------------------------------------------------------------------\n");
    //if (QuitOnError())
    //    ExitProcess(0xFFFFFFFF);
    //CopyMessageToClipboard();
    //shouldBreak = AssertNotify(type, FIRST_TIME);
    //isHandlingAssert = 0;
    //Sys_LeaveCriticalSection(CRITSECT_ASSERT);
    //if (shouldBreak)
    //    DebugBreak();
}