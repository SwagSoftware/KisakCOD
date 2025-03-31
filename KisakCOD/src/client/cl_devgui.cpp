#include "client.h"


void __cdecl CL_DevGuiFrame(int localClientNum)
{
    int v1; // eax
    int dvarIndex; // [esp+0h] [ebp-14h]
    const char *name; // [esp+8h] [ebp-Ch]
    const dvar_s *dvar; // [esp+Ch] [ebp-8h]
    char *cmd; // [esp+10h] [ebp-4h]

    for (dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex)
    {
        dvar = clGuiGlob.mapEnumDvar[dvarIndex];
        if (dvar->modified)
        {
            Dvar_ClearModified(dvar);
            name = Dvar_EnumToString(dvar);
            if (clGuiGlob.mapDirs[dvarIndex][0])
                cmd = va("devmap %s/%s", clGuiGlob.mapDirs[dvarIndex], name);
            else
                cmd = va("devmap %s", name);
            v1 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, v1, cmd);
        }
    }
}

void __cdecl CL_CreateDevGui()
{
    if (clGuiGlob.inited)
        MyAssertHandler(".\\client\\cl_devgui.cpp", 179, 0, "%s", "!clGuiGlob.inited");
    clGuiGlob.inited = 1;
    CL_RegisterDevGuiDvars();
    CL_CreateMapMenuEntries();
    R_CreateDevGui();
    Cbuf_InsertText(0, "exec devgui_main");
    Com_InitSoundDevGuiGraphs();
    CG_InitVisionSetsMenu();
}

void __cdecl CL_DestroyDevGui()
{
    int dvarIndex; // [esp+0h] [ebp-4h]

    if (!clGuiGlob.inited)
        MyAssertHandler(".\\client\\cl_devgui.cpp", 219, 0, "%s", "clGuiGlob.inited");
    clGuiGlob.inited = 0;
    DevGui_RemoveMenu("Main:1/Maps:3");
    CL_UnregisterDevGuiDvars();
    for (dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex)
        FS_FreeFileList(clGuiGlob.mapNames[dvarIndex]);
}

