#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_ui.h"
#include <universal/q_shared.h>
#include "client.h"
#include <ui/ui.h>
#include <universal/com_sndalias.h>

void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen)
{
    const char *v5; // r3

    v5 = Key_KeynumToString(keynum, 1);
    I_strncpyz(buf, v5, buflen);
}

int __cdecl CL_ShutdownUI()
{
    if (!cls.uiStarted)
        return 0;

    // MP ADD
    Com_UnloadSoundAliases(SASYS_UI);
    // MP END

    Key_RemoveCatcher(0, -17);
    UI_Shutdown();
    cls.uiStarted = 0;

    return 1;
}

void __cdecl CL_InitUI()
{
    int remoteScreenUpdateNesting; // r3

    UI_Init();
    // LWSS ADD from MP ( UI_Component::InitAssets() needs to run so that UI_Component::g members are set. 
    // Otherwise some UI panels in the script debugger will be size 0.0, which means they won't render (KISAKTODO: could probably run this at the start of Scr_InitDebugger()?)
    UI_Component_Init(); 
    // LWSS END
    remoteScreenUpdateNesting = R_PopRemoteScreenUpdate();
    cls.uiStarted = 1;
    R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
}

