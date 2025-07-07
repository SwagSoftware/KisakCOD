#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_ui.h"
#include <universal/q_shared.h>
#include "client.h"
#include <ui/ui.h>

void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen)
{
    const char *v5; // r3

    v5 = Key_KeynumToString(keynum, 1);
    I_strncpyz(buf, v5, buflen);
}

int __cdecl CL_ShutdownUI()
{
    int result; // r3

    result = 0;
    if (cls.uiStarted)
    {
        Key_RemoveCatcher(0, -17);
        UI_Shutdown();
        result = 1;
        cls.uiStarted = 0;
    }
    return result;
}

void __cdecl CL_InitUI()
{
    int v0; // r3

    UI_Init();
    v0 = R_PopRemoteScreenUpdate();
    cls.uiStarted = 1;
    R_PushRemoteScreenUpdate(v0);
}

