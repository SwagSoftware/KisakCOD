#include "client.h"
#include <qcommon/cmd.h>
#include <gfx_d3d/r_devgui.h>
#include <universal/com_files.h>
#include <cgame/cg_local.h>
#include <universal/com_sndalias.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#endif

struct ClGuiGlob // $1629A71A7F425F55F16AD3AC356BC9AE // sizeof=0x248
{                                       // ...
    bool inited;                        // ...
    // padding byte
    // padding byte
    // padding byte
    int32_t mapDirCount;                    // ...
    char mapDirs[8][64];                // ...
    const char **mapNames[8];           // ...
    const dvar_s *mapEnumDvar[8];       // ...
};

#define $1629A71A7F425F55F16AD3AC356BC9AE ClGuiGlob

ClGuiGlob clGuiGlob;

void __cdecl CL_DevGuiFrame(int32_t localClientNum)
{
    int32_t v1; // eax
    int32_t dvarIndex; // [esp+0h] [ebp-14h]
    const char *name; // [esp+8h] [ebp-Ch]
    const dvar_s *dvar; // [esp+Ch] [ebp-8h]
    char *cmd; // [esp+10h] [ebp-4h]

    for (dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex)
    {
        dvar = clGuiGlob.mapEnumDvar[dvarIndex];
        if (dvar->modified)
        {
            Dvar_ClearModified((dvar_s*)dvar);
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

const char *emptyEnumList[1];
const dvar_s *CL_RegisterDevGuiDvars()
{
    const dvar_s *result; // eax

    clGuiGlob.mapEnumDvar[0] = Dvar_RegisterEnum("mapEnum0", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[1] = Dvar_RegisterEnum("mapEnum1", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[2] = Dvar_RegisterEnum("mapEnum2", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[3] = Dvar_RegisterEnum("mapEnum3", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[4] = Dvar_RegisterEnum("mapEnum4", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[5] = Dvar_RegisterEnum("mapEnum5", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[6] = Dvar_RegisterEnum("mapEnum6", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    result = Dvar_RegisterEnum("mapEnum7", emptyEnumList, 0, DVAR_SERVERINFO | DVAR_ROM | DVAR_NORESTART, "");
    clGuiGlob.mapEnumDvar[7] = result;
    return result;
}

int32_t CL_UnregisterDevGuiDvars()
{
    int32_t result; // eax
    int32_t dvarIndex; // [esp+0h] [ebp-4h]

    for (dvarIndex = 0; dvarIndex < 8; ++dvarIndex)
    {
        Dvar_UpdateEnumDomain((dvar_s*)clGuiGlob.mapEnumDvar[dvarIndex], emptyEnumList);
        result = dvarIndex + 1;
    }
    return result;
}

void __cdecl CL_AddMapDirSlider(char *dir, int32_t locationFlags, const char *locationName)
{
    char *devguiPath; // [esp+0h] [ebp-18h]
    char *fullDir; // [esp+4h] [ebp-14h]
    int32_t fileIndex; // [esp+8h] [ebp-10h]
    const char **fileList; // [esp+Ch] [ebp-Ch]
    const dvar_s *dvar; // [esp+10h] [ebp-8h]
    int32_t fileCount; // [esp+14h] [ebp-4h] BYREF

    fullDir = va("maps/%s", dir);
    fileList = FS_ListFilesInLocation(fullDir, "d3dbsp", FS_LIST_PURE_ONLY, &fileCount, locationFlags);
    if (fileCount < 0)
        MyAssertHandler(".\\client\\cl_devgui.cpp", 89, 0, "%s", "fileCount >= 0");
    if (fileCount)
    {
        if (!fileList)
            MyAssertHandler(".\\client\\cl_devgui.cpp", 93, 0, "%s", "fileList");
        if (clGuiGlob.mapDirCount >= 8)
            MyAssertHandler(".\\client\\cl_devgui.cpp", 94, 0, "%s", "clGuiGlob.mapDirCount < MAX_MAP_DIRS");
        for (fileIndex = 0; fileIndex != fileCount; ++fileIndex)
            Com_StripExtension((char*)fileList[fileIndex], (char*)fileList[fileIndex]);
        I_strncpyz(clGuiGlob.mapDirs[clGuiGlob.mapDirCount], dir, 64);
        clGuiGlob.mapNames[clGuiGlob.mapDirCount] = fileList;
        dvar = clGuiGlob.mapEnumDvar[clGuiGlob.mapDirCount];
        Dvar_UpdateEnumDomain((dvar_s*)dvar, fileList);
        Dvar_SetInt(dvar, 0);
        devguiPath = va("Main:1/Maps:3/%s//maps//%s", locationName, dir);
        DevGui_AddDvar(devguiPath, dvar);
        ++clGuiGlob.mapDirCount;
    }
}

void __cdecl CL_CreateMapMenuEntriesForLocation(int32_t locationFlags, const char *locationName)
{
    int32_t dirCount; // [esp+0h] [ebp-Ch] BYREF
    const char **dirList; // [esp+4h] [ebp-8h]
    int32_t dirIndex; // [esp+8h] [ebp-4h]

    CL_AddMapDirSlider((char*)"", locationFlags, locationName);
    dirList = FS_ListFiles("maps", "/", FS_LIST_PURE_ONLY, &dirCount);
    for (dirIndex = 0; dirIndex != dirCount; ++dirIndex)
        CL_AddMapDirSlider((char*)dirList[dirIndex], locationFlags, locationName);
    FS_FreeFileList(dirList);
}

void CL_CreateMapMenuEntries()
{
    if (!clGuiGlob.inited)
        MyAssertHandler(".\\client\\cl_devgui.cpp", 154, 0, "%s", "clGuiGlob.inited");
    clGuiGlob.mapDirCount = 0;
    CL_CreateMapMenuEntriesForLocation(1, "main*");
    CL_CreateMapMenuEntriesForLocation(2, "dev*");
    CL_CreateMapMenuEntriesForLocation(4, "temp*");
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
    int32_t dvarIndex; // [esp+0h] [ebp-4h]

    if (!clGuiGlob.inited)
        MyAssertHandler(".\\client\\cl_devgui.cpp", 219, 0, "%s", "clGuiGlob.inited");
    clGuiGlob.inited = 0;
    DevGui_RemoveMenu("Main:1/Maps:3");
    CL_UnregisterDevGuiDvars();
    for (dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex)
        FS_FreeFileList(clGuiGlob.mapNames[dvarIndex]);
}

