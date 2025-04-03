#include "com_playerprofile.h"
#include <universal/assertive.h>
#include <cstdarg>
#include <universal/q_shared.h>
#include "qcommon.h"
#include <universal/q_parse.h>
#include <universal/com_files.h>

const dvar_t *ui_playerProfileAlreadyChosen;
const dvar_t *com_playerProfile;

int __cdecl Com_BuildPlayerProfilePath_Internal(
    char *path,
    int pathSize,
    const char *playerName,
    const char *format,
    char *vargs)
{
    int totalLength; // [esp+0h] [ebp-Ch]
    int totalLengtha; // [esp+0h] [ebp-Ch]
    int nameLength; // [esp+4h] [ebp-8h]
    int prefixLength; // [esp+8h] [ebp-4h]

    if (!path)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 157, 0, "%s", "path");
    if (pathSize <= 0)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 158, 0, "%s\n\t(pathSize) = %i", "(pathSize > 0)", pathSize);
    if (!playerName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 159, 0, "%s", "playerName");
    if (!*playerName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 160, 0, "%s", "playerName[0]");
    if (!format)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 161, 0, "%s", "format");
    prefixLength = Com_sprintf(path, pathSize, "profiles/%s/", playerName);
    if (prefixLength < 0 || prefixLength >= pathSize)
        return pathSize;
    nameLength = _vsnprintf(&path[prefixLength], pathSize - prefixLength, format, vargs);
    totalLength = nameLength + prefixLength;
    if (nameLength >= 0 && totalLength < pathSize)
    {
        totalLengtha = nameLength + prefixLength;
        if (path[nameLength + prefixLength])
            MyAssertHandler(
                ".\\qcommon\\com_playerprofile.cpp",
                179,
                1,
                "%s\n\t(path[totalLength]) = %i",
                "(path[totalLength] == '\\0')",
                path[totalLengtha]);
    }
    else
    {
        if (totalLength == pathSize && !path[totalLength - 1])
            MyAssertHandler(
                ".\\qcommon\\com_playerprofile.cpp",
                172,
                1,
                "%s",
                "totalLength != pathSize || path[totalLength - 1] != '\\0'");
        path[pathSize - 1] = 0;
        return pathSize;
    }
    return totalLengtha;
}

bool __cdecl Com_HasPlayerProfile()
{
    if (!com_playerProfile)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 146, 0, "%s", "com_playerProfile");
    return *(char *)com_playerProfile->current.integer != 0;
}


int Com_BuildPlayerProfilePath(char *path, int pathSize, const char *format, ...)
{
    va_list va; // [esp+1Ch] [ebp+14h] BYREF

    va_start(va, format);
    if (!com_playerProfile)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 204, 0, "%s", "com_playerProfile");
    if (!Com_HasPlayerProfile())
        Com_Error(ERR_FATAL, "Tried to use a player profile before it was set.  This is probably a menu bug.\n");
    return Com_BuildPlayerProfilePath_Internal(path, pathSize, com_playerProfile->current.string, format, va);
}

int Com_BuildPlayerProfilePathForPlayer(char *path, int pathSize, const char *playerName, const char *format, ...)
{
    va_list va; // [esp+20h] [ebp+18h] BYREF

    va_start(va, format);
    return Com_BuildPlayerProfilePath_Internal(path, pathSize, playerName, format, va);
}

bool __cdecl Com_IsValidPlayerProfileDir(const char *profileName)
{
    int dirCount; // [esp+0h] [ebp-10h] BYREF
    bool isValid; // [esp+7h] [ebp-9h]
    const char **dirs; // [esp+8h] [ebp-8h]
    int dirIndex; // [esp+Ch] [ebp-4h]

    if (!profileName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 29, 0, "%s", "profileName");
    if (!*profileName)
        return 0;
    isValid = 0;
    dirs = FS_ListFiles("profiles", "/", FS_LIST_ALL, &dirCount);
    for (dirIndex = 0; dirIndex < dirCount; ++dirIndex)
    {
        if (!I_stricmp(dirs[dirIndex], profileName))
        {
            isValid = 1;
            break;
        }
    }
    FS_FreeFileList(dirs);
    return isValid;
}

char __cdecl Com_SetInitialPlayerProfile(int localClientNum)
{
    parseInfo_t *activeProfileName; // [esp+0h] [ebp-Ch]
    const char *parse; // [esp+4h] [ebp-8h] BYREF
    char *activeProfileFile; // [esp+8h] [ebp-4h] BYREF

    if (FS_ReadFile("profiles/active.txt", (void **)&activeProfileFile) < 0)
        return 0;
    parse = activeProfileFile;
    activeProfileName = Com_Parse(&parse);
    FS_FreeFile(activeProfileFile);
    if (!Com_IsValidPlayerProfileDir(activeProfileName->token))
        return 0;
    Com_SetPlayerProfile(localClientNum, activeProfileName->token);
    return 1;
}

void __cdecl Com_SetPlayerProfile(int localClientNum, char *profileName)
{
    char configFile[64]; // [esp+0h] [ebp-48h] BYREF
    const char *name; // [esp+44h] [ebp-4h]

    if (!profileName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 53, 0, "%s", "profileName");
    if (!*profileName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 54, 0, "%s", "profileName[0]");
    if (!com_playerProfile)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 55, 0, "%s", "com_playerProfile");
    Dvar_SetString((dvar_s *)com_playerProfile, profileName);
    Com_BuildPlayerProfilePath(configFile, 64, "config_mp.cfg");
    Com_ExecStartupConfigs(localClientNum, configFile);
    name = Dvar_GetVariantString("name");
    if (!name || !*name)
        Dvar_SetStringByName("name", profileName);
    LiveStorage_NewUser();
}

char __cdecl Com_DeletePlayerProfile(const char *profileName)
{
    char profilePath[64]; // [esp+0h] [ebp-148h] BYREF
    char osPath[260]; // [esp+40h] [ebp-108h] BYREF

    if (!Com_IsValidPlayerProfileDir(profileName))
        return 0;
    Com_BuildPlayerProfilePathForPlayer(profilePath, 64, profileName, "");
    FS_BuildOSPath((char *)fs_basepath->current.integer, "players", profilePath, osPath);
    if (!Sys_RemoveDirTree(osPath))
        return 0;
    if (!I_stricmp(profileName, com_playerProfile->current.string))
        Dvar_SetString((dvar_s *)com_playerProfile, (char *)&String);
    return 1;
}

void __cdecl Com_InitPlayerProfiles(int localClientNum)
{
    DvarValue v1; // [esp-10h] [ebp-24h]
    unsigned int value_4; // [esp+4h] [ebp-10h]
    __int64 value_8; // [esp+8h] [ebp-Ch]

    ui_playerProfileAlreadyChosen = Dvar_RegisterInt(
        "ui_playerProfileAlreadyChosen",
        0,
        (DvarLimits)0x100000000LL,
        0x200u,
        "true if player profile has been selected.");
    *(_QWORD *)&v1.enabled = __PAIR64__(value_4, 1);
    *(_QWORD *)&v1.color[8] = value_8;
    Dvar_ChangeResetValue(ui_playerProfileAlreadyChosen, v1);
    com_playerProfile = Dvar_RegisterString("com_playerProfile", (char *)"", 0x40u, "Player profile");
    if (!Com_SetInitialPlayerProfile(localClientNum))
        Com_ExecStartupConfigs(localClientNum, 0);
}

char __cdecl Com_NewPlayerProfile(const char *profileName)
{
    char profilePath[64]; // [esp+0h] [ebp-148h] BYREF
    char osPath[260]; // [esp+40h] [ebp-108h] BYREF

    if (Com_IsValidPlayerProfileDir(profileName))
    {
        Com_Printf(16, "Profile '%s' already exists\n", profileName);
        return 0;
    }
    else
    {
        Com_BuildPlayerProfilePathForPlayer(profilePath, 64, profileName, &String);
        FS_BuildOSPath((char *)fs_basepath->current.integer, "players", profilePath, osPath);
        if (FS_CreatePath(osPath))
        {
            Com_Printf(16, "Unable to create new profile path: %s\n", osPath);
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

void __cdecl Com_ChangePlayerProfile(int localClientNum, char *profileName)
{
    char cachedName[68]; // [esp+10h] [ebp-48h] BYREF

    if (!profileName)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 76, 0, "%s", "profileName");
    if (!com_playerProfile)
        MyAssertHandler(".\\qcommon\\com_playerprofile.cpp", 77, 0, "%s", "com_playerProfile");
    if (I_stricmp(profileName, com_playerProfile->current.string))
    {
        I_strncpyz(cachedName, profileName, 64);
        if (Com_IsValidPlayerProfileDir(cachedName))
        {
            FS_WriteFileToDir(
                "profiles/active.txt",
                "players",
                cachedName,
                &cachedName[strlen(cachedName) + 1] - &cachedName[1]);
            Cmd_ExecuteSingleCommand(localClientNum, 0, "disconnect");
            Dvar_ResetDvars(0xFFFFu, DVAR_SOURCE_EXTERNAL);
            Com_SetPlayerProfile(localClientNum, cachedName);
            LiveStorage_ReadStats();
            Com_CheckSetRecommended(localClientNum);
            if (Dvar_AnyLatchedValues())
                Cbuf_AddText(localClientNum, "snd_restart\n");
        }
    }
}