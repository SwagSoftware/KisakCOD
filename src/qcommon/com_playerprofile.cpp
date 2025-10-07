#include "com_playerprofile.h"
#include <universal/assertive.h>
#include <cstdarg>
#include <universal/q_shared.h>
#include "qcommon.h"
#include <universal/q_parse.h>
#include <universal/com_files.h>
#include "cmd.h"
#include <win32/win_storage.h>
#include <win32/win_local.h>
#include <win32/win_localize.h>

const dvar_t *ui_playerProfileAlreadyChosen;
const dvar_t *com_playerProfile;

const dvar_t *sys_configureGHz;
const dvar_t *sys_sysMB;
const dvar_t *sys_gpu;
const dvar_t *sys_configSum;
const dvar_t *sys_SSE;


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

    iassert( path );
    iassert( (pathSize > 0) );
    iassert( playerName );
    iassert( playerName[0] );
    iassert( format );
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
    iassert( com_playerProfile );
    return *(char *)com_playerProfile->current.integer != 0;
}


int Com_BuildPlayerProfilePath(char *path, int pathSize, const char *format, ...)
{
    va_list va; // [esp+1Ch] [ebp+14h] BYREF

    va_start(va, format);
    iassert( com_playerProfile );
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

    iassert( profileName );
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

void __cdecl Com_SetPlayerProfile(int localClientNum, char *profileName)
{
    char configFile[64]; // [esp+0h] [ebp-48h] BYREF
    const char *name; // [esp+44h] [ebp-4h]

    iassert( profileName );
    iassert( profileName[0] );
    iassert( com_playerProfile );
    Dvar_SetString((dvar_s *)com_playerProfile, profileName);
#ifdef KISAK_MP
    Com_BuildPlayerProfilePath(configFile, 64, "config_mp.cfg");
#elif KISAK_SP
    Com_BuildPlayerProfilePath(configFile, 64, "config.cfg");
#endif
    Com_ExecStartupConfigs(localClientNum, configFile);
    name = Dvar_GetVariantString("name");
    if (!name || !*name)
        Dvar_SetStringByName("name", profileName);
    LiveStorage_NewUser();
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


char __cdecl Com_DeletePlayerProfile(const char *profileName)
{
    char profilePath[64]; // [esp+0h] [ebp-148h] BYREF
    char osPath[260]; // [esp+40h] [ebp-108h] BYREF

    if (!Com_IsValidPlayerProfileDir(profileName))
        return 0;
    Com_BuildPlayerProfilePathForPlayer(profilePath, 64, profileName, "");
    FS_BuildOSPath((char *)fs_basepath->current.integer, (char*)"players", profilePath, osPath);
    if (!Sys_RemoveDirTree(osPath))
        return 0;
    if (!I_stricmp(profileName, com_playerProfile->current.string))
        Dvar_SetString((dvar_s *)com_playerProfile, (char *)"");
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
        DVAR_AUTOEXEC,
        "true if player profile has been selected.");
    Dvar_ChangeResetValue((dvar_t*)ui_playerProfileAlreadyChosen, 1);
    com_playerProfile = Dvar_RegisterString("com_playerProfile", (char *)"", DVAR_ROM, "Player profile");
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
        Com_BuildPlayerProfilePathForPlayer(profilePath, 64, profileName, "");
        FS_BuildOSPath((char *)fs_basepath->current.integer, (char*)"players", profilePath, osPath);
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

void __cdecl Sys_GetInfo(SysInfo *info)
{
    qmemcpy(info, &sys_info, sizeof(SysInfo));
}

int __cdecl Com_GetConfigureDvarNames(const char **text, char (*dvarNames)[32])
{
    int dvarCount; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    for (dvarCount = 0; ; ++dvarCount)
    {
        token = Com_ParseOnLine(text);
        if (!*text)
            Com_Error(ERR_FATAL, "configure_mp.csv: unexpected end-of-file");
        if (!token->token[0])
            break;
        if (strlen(token->token) > 0x1F)
            Com_Error(ERR_FATAL, "configure_mp.csv: dvar name %s longer than %i", token, 31);
        if (dvarCount >= 64)
            Com_Error(ERR_FATAL, "configure_mp.csv: more than %i dvars", 64);
        I_strncpyz(&(*dvarNames)[32 * dvarCount], token->token, 32);
    }
    return dvarCount;
}

void __cdecl Com_GetConfigureDvarValues(int dvarCount, const char **text, char (*dvarValues)[32])
{
    int dvarIndex; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]
    parseInfo_t *tokena; // [esp+14h] [ebp-4h]

    for (dvarIndex = 0; dvarIndex < dvarCount; ++dvarIndex)
    {
        token = Com_ParseOnLine(text);
        if (!*text)
            Com_Error(ERR_FATAL, "configure_mp.csv: unexpected end-of-file");
        if (!token->token[0])
            Com_Error(ERR_FATAL, "onfigure_mp.csv: missing entry in dvar value column %i", dvarIndex);
        if (strlen(token->token) > 0x1F)
            Com_Error(ERR_FATAL, "configure_mp.csv: entry % s in dvar value column %i is longer than %i", token, dvarIndex, 31);
        if (dvarValues)
            I_strncpyz(&(*dvarValues)[32 * dvarIndex], token->token, 32);
    }
    tokena = Com_ParseOnLine(text);
    if (tokena->token[0])
        Com_Error(ERR_FATAL, "configure_mp.csv: extra dvar value column(s): value = %s", tokena);
}

void __cdecl Com_SetConfigureDvars(int dvarCount, const char (*dvarNames)[32], const char (*dvarValues)[32])
{
    int dvarIndex; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    for (dvarIndex = 0; dvarIndex < dvarCount; ++dvarIndex)
    {
        Dvar_SetFromStringByNameFromSource(
            &(*dvarNames)[32 * dvarIndex],
            &(*dvarValues)[32 * dvarIndex],
            DVAR_SOURCE_EXTERNAL);
        dvar = Dvar_FindVar(&(*dvarNames)[32 * dvarIndex]);
        iassert( dvar );
        Dvar_AddFlags(dvar, 1);
    }
}

char __cdecl Com_SetRecommendedCpu(int localClientNum, const SysInfo *info, char **text)
{
    char dvarValues[64][32]{ 0 }; // [esp+14h] [ebp-14D8h] BYREF
    char dvarNames[64][32]{ 0 }; // [esp+814h] [ebp-CD8h] BYREF
    int dvarCount; // [esp+1018h] [ebp-4D4h]
    double v7[76]; // [esp+101Ch] [ebp-4D0h] BYREF
    char v8; // [esp+1282h] [ebp-26Ah]
    char v9; // [esp+1283h] [ebp-269h]
    double v10[76]; // [esp+1284h] [ebp-268h] BYREF
    const char *s0; // [esp+14E8h] [ebp-4h]

    dvarCount = 0;
    v7[0] = -1.0;
    LODWORD(v7[3]) = 0;
    v8 = 0;
    while (1)
    {
        s0 = Com_Parse(text)->token;
        if (!text)
            break;
        if (*s0 && *s0 != '#')
        {
            if (!I_stricmp(s0, "gpu"))
            {
                Com_UngetToken();
                break;
            }
            if (dvarCount)
            {
                v10[0] = atof(s0);
                if (v10[0] < 0.0)
                    Com_Error(ERR_FATAL, "configure_mp.csv: cpu ghz %g not allowed to be less than 0", v10[0]);
                s0 = Com_ParseOnLine(text)->token;
                LODWORD(v10[3]) = atoi(s0);
                if (SLODWORD(v10[3]) < 128)
                    Com_Error(ERR_FATAL, "configure_mp.csv: sys mb %i not allowed to be less than 128", LODWORD(v10[3]));
                v9 = 0;
                if (v10[0] <= info->configureGHz
                    && info->sysMB >= SLODWORD(v10[3])
                    && (v10[0] > v7[0] || v10[0] == v7[0] && SLODWORD(v7[3]) < SLODWORD(v10[3])))
                {
                    v9 = 1;
                    qmemcpy(v7, v10, sizeof(v7));
                    v8 = 1;
                }
                Com_GetConfigureDvarValues(dvarCount, (const char**)text, v9 != 0 ? dvarValues : 0);
            }
            else
            {
                if (I_stricmp(s0, "cpu ghz"))
                    Com_Error(ERR_FATAL, "configure_mp.csv: \"cpu ghz\" should be the first column");
                s0 = Com_ParseOnLine(text)->token;
                if (I_stricmp(s0, "sys mb"))
                    Com_Error(ERR_FATAL, "configure_mp.csv: \"sys mb\" should be the second column");
                dvarCount = Com_GetConfigureDvarNames((const char **)text, dvarNames);
            }
        }
        else
        {
            Com_SkipRestOfLine((const char **)text);
        }
    }
    if (!v8)
        return 0;
    Com_Printf(16, "configure_mp.csv: using CPU configuration %.0f GHz %i MB\n", v7[0], LODWORD(v7[3]));
    Cbuf_AddText(localClientNum, "exec configure_mp.cfg");
    Cbuf_Execute(localClientNum, 0);
    Com_SetConfigureDvars(dvarCount, dvarNames, dvarValues);
    return 1;
}

int __cdecl Com_GpuStringCompare(const char *wild, const char *s)
{
    int v3; // esi
    char charWild; // [esp+7h] [ebp-9h]
    int delta; // [esp+8h] [ebp-8h]
    char charRef; // [esp+Fh] [ebp-1h]

    iassert( wild );
    iassert( s );
    do
    {
        charWild = *wild++;
        if (charWild == 42)
        {
            if (!*wild)
                return 0;
            if (*s && !Com_GpuStringCompare(wild - 1, s + 1))
                return 0;
        }
        else if (charWild == 32)
        {
            if (*s && !isdigit(*s) && !Com_GpuStringCompare(wild - 1, s + 1))
                return 0;
        }
        else
        {
            charRef = *s++;
            if (charWild != charRef && charWild != 63)
            {
                v3 = tolower(charWild);
                delta = v3 - tolower(charRef);
                if (delta)
                    return 2 * (delta >= 0) - 1;
            }
        }
    } while (charWild);
    return 0;
}

BOOL __cdecl Com_DoesGpuStringMatch(const char *find, const char *ref)
{
    int wildcardLen; // [esp+0h] [ebp-40Ch]
    char wildcardTemplate[1024]; // [esp+4h] [ebp-408h] BYREF
    int findLen; // [esp+408h] [ebp-4h]

    wildcardTemplate[0] = 42;
    wildcardLen = 1;
    for (findLen = 0; find[findLen]; ++findLen)
    {
        if (!isspace(find[findLen]))
        {
            wildcardTemplate[wildcardLen] = find[findLen];
        LABEL_10:
            if (++wildcardLen == 1023)
                Com_Error(ERR_FATAL, "configure_mp.csv: \"find\" string is too long");
            continue;
        }
        iassert( (wildcardLen >= 1) );
        if (wildcardTemplate[wildcardLen - 1] != 32)
        {
            wildcardTemplate[wildcardLen] = 32;
            goto LABEL_10;
        }
    }
    if (wildcardTemplate[wildcardLen - 1] != 42)
        wildcardTemplate[wildcardLen++] = 42;
    wildcardTemplate[wildcardLen] = 0;
    return Com_GpuStringCompare(wildcardTemplate, ref) == 0;
}

char __cdecl Com_SetRecommendedGpu(const SysInfo *info, char **text)
{
    char dvarValues[64][32]; // [esp+0h] [ebp-1010h] BYREF
    char dvarNames[64][32]; // [esp+800h] [ebp-810h] BYREF
    int dvarCount; // [esp+1004h] [ebp-Ch]
    char v6; // [esp+100Bh] [ebp-5h]
    char *s0; // [esp+100Ch] [ebp-4h]

    s0 = Com_Parse(text)->token;
    if (I_stricmp(s0, "gpu"))
    {
        Com_UngetToken();
        return 0;
    }
    else
    {
        dvarCount = Com_GetConfigureDvarNames((const char **)text, dvarNames);
        v6 = 0;
        while (1)
        {
            s0 = Com_Parse(text)->token;
            if (!*text)
                break;
            if (*s0 && *s0 != 35)
            {
                if (v6 || !Com_DoesGpuStringMatch(s0, info->gpuDescription))
                {
                    Com_GetConfigureDvarValues(dvarCount, (const char **)text, 0);
                }
                else
                {
                    Com_Printf(16, "configure_mp.csv: using GPU configuration \"%s\"\n", s0);
                    Com_GetConfigureDvarValues(dvarCount, (const char **)text, dvarValues);
                    Com_SetConfigureDvars(dvarCount, dvarNames, dvarValues);
                    v6 = 1;
                }
            }
            else
            {
                Com_SkipRestOfLine((const char **)text);
            }
        }
        return v6;
    }
}

int __cdecl Com_ConfigureChecksum(const char *csv, int filesize)
{
    int checksum; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    checksum = 0;
    for (i = 0; i < filesize; ++i)
        checksum = csv[i] + 0x7A69 * checksum;
    return (checksum & 0xFFFFFFF) + 1;
}

void Sys_RegisterInfoDvars()
{
    DvarLimits min; // [esp+4h] [ebp-14h]
    DvarLimits mina; // [esp+4h] [ebp-14h]
    float value; // [esp+14h] [ebp-4h]

    min.value.max = FLT_MAX;
    min.value.min = -FLT_MAX;
    sys_configureGHz = Dvar_RegisterFloat(
        "sys_configureGHz",
        0.0,
        min,
        DVAR_INIT | DVAR_ARCHIVE,
        "Normalized total CPU power, based on cpu type, count, and speed; used in autoconfigure");
    sys_sysMB = Dvar_RegisterInt("sys_sysMB", 0, 0x7FFFFFFF80000000LL, DVAR_INIT | DVAR_ARCHIVE, "Physical memory in the system");
    sys_gpu = Dvar_RegisterString("sys_gpu", "", DVAR_INIT | DVAR_ARCHIVE, "GPU description");
    sys_configSum = Dvar_RegisterInt("sys_configSum", 0, 0x7FFFFFFF80000000LL, DVAR_INIT | DVAR_ARCHIVE, "Configuration checksum");
    sys_SSE = Dvar_RegisterBool("sys_SSE", sys_info.SSE, DVAR_ROM, "Operating system allows Streaming SIMD Extensions");
    mina.value.max = FLT_MAX;
    mina.value.min = -FLT_MAX;
    value = sys_info.cpuGHz;
    Dvar_RegisterFloat("sys_cpuGHz", value, mina, DVAR_ROM, "Measured CPU speed");
    Dvar_RegisterString("sys_cpuName", sys_info.cpuName, DVAR_ROM, "CPU name description");
}

void __cdecl Sys_ArchiveInfo(int checksum)
{
    float value; // [esp+4h] [ebp-4h]

    Sys_RegisterInfoDvars();
    value = sys_info.configureGHz;
    Dvar_SetFloat(sys_configureGHz, value);
    Dvar_SetInt(sys_sysMB, sys_info.sysMB);
    Dvar_SetString(sys_gpu, sys_info.gpuDescription);
    Dvar_SetInt(sys_configSum, checksum);
}

void __cdecl Com_SetRecommended(int localClientNum, int restart)
{
    int filesize; // [esp+Ch] [ebp-274h]
    SysInfo info; // [esp+10h] [ebp-270h] BYREF
    int checksum; // [esp+274h] [ebp-Ch]
    char *csv; // [esp+278h] [ebp-8h] BYREF
    char *text; // [esp+27Ch] [ebp-4h] BYREF

    Com_Printf(16, "========= autoconfigure\n");
    Sys_GetInfo(&info);
    info.configureGHz = info.configureGHz * 1.02;
    if (info.sysMB >= 128)
        info.sysMB += 8;
    else
        info.sysMB = 128;
    filesize = FS_ReadFile("configure_mp.csv", (void**)&csv);
    if (filesize < 0)
        Com_Error(ERR_FATAL, "EXE_ERR_NOT_FOUND");
    text = csv;
    Com_BeginParseSession("configure_mp.csv");
    Com_SetCSV(1);
    if (!Com_SetRecommendedCpu(localClientNum, &info, &text))
    {
        Sys_GetInfo(&info);
        Com_Error(ERR_FATAL, "KISAK GHZ %f, %d", info.configureGHz, info.sysMB);
    }
    if (!Com_SetRecommendedGpu(&info, &text))
        Com_Error(ERR_FATAL, "KISAK GPU %s", info.gpuDescription);
    Com_EndParseSession();
    checksum = Com_ConfigureChecksum(csv, filesize);
    FS_FreeFile(csv);
    Sys_ArchiveInfo(checksum);
    if (restart)
    {
        if (Dvar_AnyLatchedValues())
            Cbuf_AddText(localClientNum, "snd_restart\n");
    }
}

bool __cdecl Sys_ShouldUpdateForInfoChange()
{
    HWND ActiveWindow; // eax
    char *v2; // [esp-Ch] [ebp-Ch]
    char *v3; // [esp-8h] [ebp-8h]

    Sys_ArchiveInfo(0);
    v3 = Win_LocalizeRef("WIN_COMPUTER_CHANGE_TITLE");
    v2 = Win_LocalizeRef("WIN_COMPUTER_CHANGE_BODY");
    ActiveWindow = GetActiveWindow();
    return MessageBoxA(ActiveWindow, v2, v3, 0x44u) == 6;
}

bool __cdecl Sys_ShouldUpdateForConfigChange()
{
    HWND ActiveWindow; // eax
    char *v2; // [esp-Ch] [ebp-Ch]
    char *v3; // [esp-8h] [ebp-8h]

    v3 = Win_LocalizeRef("WIN_CONFIGURE_UPDATED_TITLE");
    v2 = Win_LocalizeRef("WIN_CONFIGURE_UPDATED_BODY");
    ActiveWindow = GetActiveWindow();
    return MessageBoxA(ActiveWindow, v2, v3, 0x44u) == 6;
}

bool __cdecl Sys_HasInfoChanged()
{
    Sys_RegisterInfoDvars();
    return (sys_info.configureGHz * 1.100000023841858 < sys_configureGHz->current.value
        || sys_info.configureGHz * 0.8999999761581421 > sys_configureGHz->current.value
        || sys_sysMB->current.integer > sys_info.sysMB + 32
        || sys_sysMB->current.integer < sys_info.sysMB - 32
        || strcmp(sys_gpu->current.string, sys_info.gpuDescription))
        && Sys_ShouldUpdateForInfoChange();
}

bool __cdecl Sys_HasConfigureChecksumChanged(int checksum)
{
    bool changed; // [esp+3h] [ebp-1h]

    Sys_RegisterInfoDvars();
    changed = 0;
    if (sys_configSum->current.integer && sys_configSum->current.integer != checksum)
        changed = Sys_ShouldUpdateForConfigChange();
    if (!sys_configSum->current.integer || sys_configSum->current.integer != checksum)
        Dvar_SetInt(sys_configSum, checksum);
    return changed;
}

bool __cdecl Com_HasConfigureFileChanged()
{
    int filesize; // [esp+0h] [ebp-Ch]
    int checksum; // [esp+4h] [ebp-8h]
    char *csv; // [esp+8h] [ebp-4h] BYREF

    filesize = FS_ReadFile("configure_mp.csv", (void**)&csv);
    if (filesize < 0)
        Com_Error(ERR_FATAL, "EXE ERR NOT FOUND! [KISAK]");
    checksum = Com_ConfigureChecksum(csv, filesize);
    FS_FreeFile(csv);
    return Sys_HasConfigureChecksumChanged(checksum);
}

void __cdecl Com_CheckSetRecommended(int localClientNum)
{
    if (!com_recommendedSet->current.enabled || Com_HasConfigureFileChanged())
    {
        Com_SetRecommended(localClientNum, 0);
        Dvar_SetBool(com_recommendedSet, 1);
    }
    if (Sys_HasInfoChanged())
        Com_SetRecommended(localClientNum, 0);
}

void __cdecl Com_ChangePlayerProfile(int localClientNum, char *profileName)
{
    char cachedName[68]; // [esp+10h] [ebp-48h] BYREF

    iassert( profileName );
    iassert( com_playerProfile );
    if (I_stricmp(profileName, com_playerProfile->current.string))
    {
        I_strncpyz(cachedName, profileName, 64);
        if (Com_IsValidPlayerProfileDir(cachedName))
        {
            FS_WriteFileToDir(
                (char*)"profiles/active.txt",
                (char*)"players",
                cachedName,
                &cachedName[strlen(cachedName) + 1] - &cachedName[1]);
            Cmd_ExecuteSingleCommand(localClientNum, 0, (char*)"disconnect");
            Dvar_ResetDvars(0xFFFFu, DVAR_SOURCE_EXTERNAL);
            Com_SetPlayerProfile(localClientNum, cachedName);
            LiveStorage_ReadStats();
            Com_CheckSetRecommended(localClientNum);
            if (Dvar_AnyLatchedValues())
                Cbuf_AddText(localClientNum, "snd_restart\n");
        }
    }
}