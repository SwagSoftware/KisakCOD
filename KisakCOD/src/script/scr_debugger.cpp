void __cdecl Scr_ShutdownRemoteClient(int restart)
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8244, 0, "%s", "Sys_IsRemoteDebugClient()");
    Scr_ShutdownDebuggerSystem(restart);
    Scr_ShutdownDebugger();
    Scr_ShutdownDebuggerMain();
    Scr_ShutdownOpcodeLookup();
}