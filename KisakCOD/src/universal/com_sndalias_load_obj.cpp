#include "com_sndalias.h"
#include "q_parse.h"
#include "com_files.h"
#include <qcommon/cmd.h>
#include "com_memory.h"
#include <devgui/devgui.h>

SoundAliasLoadGlobals saLoadObjGlob;

cmd_function_s Com_RefreshVolumeModGroups_f_VAR;
void __cdecl Com_RefreshVolumeModGroups_f()
{
    saLoadObjGlob.volumeModGroupsInitialized = 0;
}

void __cdecl Com_LoadVolumeModGroups(VolumeModGroup *volumeModGroups)
{
    float v1; // [esp+8h] [ebp-208Ch]
    char v2; // [esp+Fh] [ebp-2085h]
    VolumeModGroup *v3; // [esp+14h] [ebp-2080h]
    char *v4; // [esp+18h] [ebp-207Ch]
    char filename[68]; // [esp+2Ch] [ebp-2068h] BYREF
    signed int v6; // [esp+70h] [ebp-2024h]
    char buffer[8196]; // [esp+74h] [ebp-2020h] BYREF
    int i; // [esp+207Ch] [ebp-18h]
    const char *last; // [esp+2080h] [ebp-14h]
    int file; // [esp+2084h] [ebp-10h] BYREF
    char *nptr; // [esp+2088h] [ebp-Ch]
    char *data_p; // [esp+208Ch] [ebp-8h] BYREF
    int len; // [esp+2090h] [ebp-4h]

    last = "VOLUMEMODGROUPS";
    len = strlen("VOLUMEMODGROUPS");
    if (!volumeModGroups)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1042, 0, "%s", "volumeModGroups");
    strcpy(filename, "soundaliases/volumemodgroups.def");
    v6 = FS_FOpenFileRead(filename, &file);
    if (v6 >= 0)
    {
        if (v6)
        {
            FS_Read((unsigned char*)buffer, len, file);
            buffer[len] = 0;
            if (!strncmp(buffer, last, len))
            {
                if (v6 - len < 0x2000)
                {
                    memset(buffer, 0, 0x2000u);
                    FS_Read((unsigned char *)buffer, v6 - len, file);
                    buffer[v6 - len] = 0;
                    FS_FCloseFile(file);
                    Com_BeginParseSession(filename);
                    data_p = buffer;
                    for (i = 0; ; ++i)
                    {
                        nptr = (char*)Com_Parse(&data_p);
                        if (!*nptr || *nptr == 125)
                        {
                            Com_EndParseSession();
                            return;
                        }
                        if (i >= 32)
                        {
                            Com_EndParseSession();
                            Com_Error(
                                ERR_DROP,
                                "ERROR: volumemodgroups parse failure on file \"%s\": groups parsed (%d) is greater than or equal to maxGroups(%d)\n",
                                filename,
                                i,
                                32);
                            return;
                        }
                        v4 = nptr;
                        v3 = &volumeModGroups[i];
                        do
                        {
                            v2 = *v4;
                            v3->name[0] = *v4++;
                            v3 = (v3 + 1);
                        } while (v2);
                        nptr = (char*)Com_Parse(&data_p);
                        if (!*nptr || *nptr == 125)
                            break;
                        v1 = atof(nptr);
                        volumeModGroups[i].value = v1;
                    }
                    Com_EndParseSession();
                    Com_Error(
                        ERR_DROP,
                        "ERROR: volumemodgroups parse failure on file \"%s\": groupname '%s' missing a matching value\n",
                        filename,
                        volumeModGroups[i].name);
                }
                else
                {
                    FS_FCloseFile(file);
                    Com_Error(ERR_DROP, "ERROR: \"%s\" Is too long of a volumemodgroups file to parse\n", filename);
                }
            }
            else
            {
                FS_FCloseFile(file);
                Com_Error(ERR_DROP, "ERROR: \"%s\" does not appear to be a volumemodgroups file\n", filename);
            }
        }
        else
        {
            FS_FCloseFile(file);
            Com_Error(ERR_DROP, "ERROR: '%s' is empty\n", filename);
        }
    }
    else
    {
        Com_Error(ERR_DROP, "ERROR: Could not find '%s'\n", filename);
    }
}

SndCurve *__cdecl Com_GetDefaultSoundAliasVolumeFalloffCurve()
{
    return g_sa.volumeFalloffCurves;
}

SpeakerMap *__cdecl Com_GetDefaultSoundAliasSpeakerMap()
{
    return &g_sa.speakerMaps[0].speakerMap;
}

void __cdecl Com_LoadSoundAliasDefaults(snd_alias_build_s *alias, const char *sourceFile, const char *loadspec)
{
    char v3; // al
    snd_alias_build_s *v4; // [esp+1Ch] [ebp-8h]

    v4 = alias;
    do
    {
        v3 = *sourceFile;
        v4->szSourceFile[0] = *sourceFile++;
        v4 = (v4 + 1);
    } while (v3);
    alias->aliasName[0] = 0;
    alias->secondaryAliasName[0] = 0;
    alias->chainAliasName[0] = 0;
    alias->sequence = 0;
    alias->soundFile[0] = 0;
    alias->subtitleText = 0;
    alias->volMin = 1.0;
    alias->volMax = 1.0;
    alias->fVolMod = 1.0;
    alias->pitchMin = 1.0;
    alias->pitchMax = 1.0;
    alias->distMin = 120.0;
    alias->distMax = 0.0;
    alias->iChannel = 0;
    alias->eType = SAT_LOADED;
    alias->bLooping = 0;
    alias->bRandomLooping = 0;
    alias->probability = 1.0;
    alias->lfePercentage = 0.0;
    alias->centerPercentage = 0.0;
    alias->error = 0;
    alias->keep = strcmp(loadspec, "menu") != 0;
    alias->bMaster = 0;
    alias->bSlave = 0;
    alias->bFullDryLevel = 0;
    alias->bNoWetLevel = 0;
    alias->slavePercentage = 1.0;
    alias->startDelay = 0;
    alias->volumeFalloffCurve = Com_GetDefaultSoundAliasVolumeFalloffCurve();
    alias->speakerMap = Com_GetDefaultSoundAliasSpeakerMap();
    alias->pNext = 0;
}

BOOL __cdecl Com_IsValidName(
    const char *token,
    unsigned int maxLength,
    int(__cdecl *validityFunction)(const char *),
    snd_alias_build_s *alias,
    snd_alias_members_t field,
    const char *sourceFile)
{
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 675, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 676, 0, "%s", "sourceFile");
    if (strlen(token) <= maxLength)
    {
        if (validityFunction)
            alias->error = validityFunction(token) == 0;
    }
    else
    {
        Com_PrintError(
            9,
            "ERROR: Sound alias file %s: %s '%s' is longer than %i characters\n",
            sourceFile,
            g_pszSndAliasKeyNames[field],
            token,
            maxLength - 1);
        alias->error = 1;
    }
    return !alias->error;
}

int __cdecl Com_IsValidAliasName(const char *pszName)
{
    const char *pszNamea; // [esp+8h] [ebp+8h]

    if (!pszName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 359, 0, "%s", "pszName");
    if (*pszName < 32 || !isalnum(*pszName) && *pszName != 95)
        return 0;
    for (pszNamea = pszName + 1; *pszNamea; ++pszNamea)
    {
        if (*pszNamea < 32 || !isalnum(*pszNamea) && *pszNamea != 95)
            return 0;
    }
    return 1;
}

double __cdecl Com_ParseFloatInRange(
    const char *token,
    float min,
    float max,
    snd_alias_build_s *alias,
    snd_alias_members_t field,
    const char *sourceFile)
{
    float v8; // [esp+1Ch] [ebp-10h]
    float v9; // [esp+20h] [ebp-Ch]
    float v10; // [esp+24h] [ebp-8h]
    float val; // [esp+28h] [ebp-4h]

    if (!token)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 659, 0, "%s", "token");
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 660, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 661, 0, "%s", "sourceFile");
    val = atof(token);
    if (min > val || max < val)
    {
        Com_PrintError(
            9,
            "ERROR: Sound alias file %s: %s '%f' is not within the range of '%f'-'%f'.\n",
            sourceFile,
            g_pszSndAliasKeyNames[field],
            val,
            min,
            max);
        alias->error = 1;
    }
    v9 = val - max;
    if (v9 < 0.0)
        v10 = val;
    else
        v10 = max;
    v8 = min - val;
    if (v8 < 0.0)
        return v10;
    else
        return min;
}

double __cdecl Com_GetSoundAliasVolumeModGroupValue(const char *volumeModGroupName, const char *sourceFile)
{
    int i; // [esp+0h] [ebp-4h]

    if (!volumeModGroupName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 289, 0, "%s", "volumeModGroupName");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 290, 0, "%s", "sourceFile");
    for (i = 0; i < 32; ++i)
    {
        if (!I_stricmp(volumeModGroupName, saLoadObjGlob.volumeModGroups[i].name))
            return saLoadObjGlob.volumeModGroups[i].value;
    }
    Com_Error(ERR_DROP, "Sound alias file %s: Volume Mod Group %s not found", sourceFile, volumeModGroupName);
    return 0.0;
}

void __cdecl Com_SoundAliasChannelForName(char *name, const char *sourceFile, snd_alias_build_s *alias)
{
    int v3; // [esp+0h] [ebp-400Ch]
    char string[16384]; // [esp+4h] [ebp-4008h] BYREF
    int i; // [esp+4008h] [ebp-4h]

    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 387, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 388, 0, "%s", "sourceFile");
    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 389, 0, "%s", "name");
    for (i = 0; i < saLoadObjGlob.entChannelCount; ++i)
    {
        if (!I_stricmp(name, saLoadObjGlob.entChannels[i]))
        {
            alias->iChannel = i;
            return;
        }
    }
    v3 = 0;
    for (i = 0; i < saLoadObjGlob.entChannelCount; ++i)
    {
        v3 += sprintf(&string[v3], "%s", saLoadObjGlob.entChannels[i]);
        if (i >= saLoadObjGlob.entChannelCount - 2)
        {
            if (i == saLoadObjGlob.entChannelCount - 2)
                v3 += sprintf(&string[v3], " or ");
        }
        else
        {
            v3 += sprintf(&string[v3], ", ");
        }
    }
    Com_PrintError(9, "ERROR: Sound alias file %s: Unknown sound channel '%s'; should be %s\n", sourceFile, name, string);
    alias->error = 1;
}

void __cdecl Com_SoundAliasTypeForName(const char *name, const char *sourceFile, snd_alias_build_s *alias)
{
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 422, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 423, 0, "%s", "sourceFile");
    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 424, 0, "%s", "name");
    if (I_stricmp(name, "streamed"))
    {
        if (I_stricmp(name, "loaded"))
        {
            Com_PrintError(
                9,
                "ERROR: Sound alias file %s: Unknown sound type '%s'; should be primed, streamed or loaded\n",
                sourceFile,
                name);
            alias->error = 1;
        }
        else
        {
            alias->eType = SAT_LOADED;
        }
    }
    else
    {
        alias->eType = SAT_STREAMED;
    }
}

void __cdecl Com_SoundAliasLoop(const char *token, const char *sourceFile, snd_alias_build_s *alias)
{
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 449, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 450, 0, "%s", "sourceFile");
    if (!token)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 451, 0, "%s", "token");
    if (I_stricmp(token, "looping"))
    {
        if (I_stricmp(token, "rlooping"))
        {
            if (I_stricmp(token, "nonlooping"))
            {
                Com_PrintError(
                    9,
                    "ERROR: Sound alias file %s: Unknown sound looping type '%s'; valid values are 'looping, rlooping, nonlooping'.\n",
                    sourceFile,
                    token);
                alias->error = 1;
            }
            else
            {
                alias->bLooping = 0;
            }
        }
        else
        {
            alias->bLooping = 1;
            alias->bRandomLooping = 1;
        }
    }
    else
    {
        alias->bLooping = 1;
    }
}

char __cdecl Com_SoundAliasLoadSpec(
    const char *loadspecLevel,
    const char *loadspecCurGame,
    const char *loadlist,
    const char *sourceFile)
{
    bool hasLoadSpecMenu; // [esp+94h] [ebp-118h]
    bool keepIfMatchGame; // [esp+95h] [ebp-117h]
    bool hasLoadSpecAllMp; // [esp+96h] [ebp-116h]
    bool keepIfMatchLevel; // [esp+97h] [ebp-115h]
    const char *parsePos; // [esp+98h] [ebp-114h]
    bool didMatchGame; // [esp+9Dh] [ebp-10Fh]
    bool hasLoadSpecLevel; // [esp+9Eh] [ebp-10Eh]
    bool hasLoadSpecAllSp; // [esp+9Fh] [ebp-10Dh]
    int tokenLen; // [esp+A0h] [ebp-10Ch]
    char token[256]; // [esp+A4h] [ebp-108h] BYREF
    bool didMatchLevel; // [esp+1ABh] [ebp-1h]

    if (*loadlist == 33)
    {
        keepIfMatchLevel = 0;
        parsePos = loadlist + 1;
    }
    else
    {
        keepIfMatchLevel = 1;
        parsePos = loadlist;
    }
    keepIfMatchGame = 1;
    if (*loadspecCurGame == 33)
    {
        keepIfMatchGame = 0;
        ++loadspecCurGame;
    }
    hasLoadSpecLevel = 0;
    hasLoadSpecAllSp = 0;
    hasLoadSpecAllMp = 0;
    hasLoadSpecMenu = 0;
    didMatchLevel = 0;
    didMatchGame = 0;
    while (1)
    {
        while (*parsePos == 32)
            ++parsePos;
        if (!*parsePos)
            break;
        if (*parsePos == 33)
            Com_PrintError(
                9,
                "ERROR: Sound alias file %s: '!' can only be the very first character in a loadspec: %s\n",
                sourceFile,
                loadlist);
        tokenLen = 0;
        while (*parsePos != 32 && *parsePos)
        {
            if (*parsePos == 33)
                Com_PrintError(
                    9,
                    "ERROR: Sound alias file %s: '!' can only be the very first character in a loadspec: %s\n",
                    sourceFile,
                    loadlist);
            token[tokenLen++] = tolower(*parsePos);
            if (tokenLen == 256)
                Com_PrintError(9, "ERROR: Sound alias file %s: Load spec token too long: %s\n", sourceFile, loadlist);
            ++parsePos;
        }
        token[tokenLen] = 0;
        if (!strcmp(token, "all_sp"))
        {
            hasLoadSpecAllSp = 1;
            if (!strcmp(token, loadspecCurGame))
                didMatchGame = 1;
        }
        else if (!strcmp(token, "all_mp"))
        {
            hasLoadSpecAllMp = 1;
            if (!strcmp(token, loadspecCurGame))
                didMatchGame = 1;
        }
        else
        {
            if (!strcmp(token, "menu"))
                hasLoadSpecMenu = 1;
            else
                hasLoadSpecLevel = 1;
            if (!strcmp(token, loadspecLevel))
                didMatchLevel = 1;
        }
    }
    if (hasLoadSpecAllSp && hasLoadSpecAllMp)
        Com_PrintError(
            9,
            "ERROR: Sound alias file %s: 'all_sp' and 'all_mp' load specs are mutually exclusive: %s\n",
            sourceFile,
            loadlist);
    if (hasLoadSpecMenu)
    {
        if (hasLoadSpecAllSp || hasLoadSpecAllMp || hasLoadSpecLevel)
            Com_PrintError(9, "ERROR: Sound alias file %s: 'menu' load spec must be by itself: %s\n", sourceFile, loadlist);
        if (didMatchLevel || !*loadspecLevel)
            return keepIfMatchLevel;
        else
            return !keepIfMatchLevel;
    }
    else
    {
        if (didMatchLevel)
            return keepIfMatchLevel;
        if (didMatchGame)
        {
            return keepIfMatchGame && keepIfMatchLevel;
        }
        else if (hasLoadSpecLevel)
        {
            return *loadspecLevel && strcmp(loadspecLevel, "menu") && !keepIfMatchLevel;
        }
        else
        {
            if (keepIfMatchGame)
            {
                if (keepIfMatchLevel)
                {
                    if (!hasLoadSpecAllSp && !hasLoadSpecAllMp)
                        return 1;
                }
                else if (hasLoadSpecAllSp || hasLoadSpecAllMp)
                {
                    return 1;
                }
            }
            return 0;
        }
    }
}

void __cdecl Com_SoundAliasMasterSlave(const char *token, const char *sourceFile, snd_alias_build_s *alias)
{
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 615, 0, "%s", "alias");
    if (!sourceFile)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 616, 0, "%s", "sourceFile");
    if (!token)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 617, 0, "%s", "token");
    if (I_stricmp(token, "master"))
    {
        alias->bMaster = 0;
        alias->bSlave = 1;
        alias->slavePercentage = atof(token);
        if (alias->slavePercentage < 0.0 || alias->slavePercentage > 1.0)
        {
            Com_PrintError(
                9,
                "ERROR: Sound alias file %s: SlavePercentage'%f' is not within the range of '%f'-'%f'.\n",
                sourceFile,
                alias->slavePercentage,
                0.0,
                1.0);
            alias->error = 1;
        }
    }
    else
    {
        alias->bMaster = 1;
        alias->bSlave = 0;
    }
}

char __cdecl Com_FinishBuildingSoundAlias(snd_alias_build_s *build)
{
    float fSwapTemp; // [esp+10h] [ebp-8h]
    float fSwapTempa; // [esp+10h] [ebp-8h]
    float adjustedVol; // [esp+14h] [ebp-4h]
    float adjustedVola; // [esp+14h] [ebp-4h]

    if (!build)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 846, 0, "%s", "build");
    if (build->pitchMax < build->pitchMin)
    {
        fSwapTemp = build->pitchMax;
        build->pitchMax = build->pitchMin;
        build->pitchMin = fSwapTemp;
    }
    if (build->pitchMin > 0.0)
    {
        if (build->volMax < build->volMin)
        {
            fSwapTempa = build->volMax;
            build->volMax = build->volMin;
            build->volMin = fSwapTempa;
        }
        if (build->volMin >= 0.0)
        {
            if (build->distMax == 0.0)
                build->distMax = build->distMin * 5.0;
            if (build->distMin < build->distMax)
            {
                if (build->distMin > 0.0)
                {
                    if (build->fVolMod != 1.0)
                    {
                        adjustedVol = build->volMin * build->fVolMod;
                        if (adjustedVol >= 0.0)
                        {
                            if (adjustedVol > 1.0)
                                adjustedVol = 1.0;
                        }
                        else
                        {
                            adjustedVol = 0.0;
                        }
                        build->volMin = adjustedVol;
                        adjustedVola = build->volMax * build->fVolMod;
                        if (adjustedVola >= 0.0)
                        {
                            if (adjustedVola > 1.0)
                                adjustedVola = 1.0;
                        }
                        else
                        {
                            adjustedVola = 0.0;
                        }
                        build->volMax = adjustedVola;
                    }
                    return 1;
                }
                else
                {
                    Com_PrintError(9, "ERROR: sound alias '%s' has dist_min %g <= 0\n", build->aliasName, build->distMin);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(
                    9,
                    "ERROR: sound alias '%s' has dist_min %g >= dist_max %g\n",
                    build->aliasName,
                    build->distMin,
                    build->distMax);
                return 0;
            }
        }
        else
        {
            Com_PrintError(9, "ERROR: sound alias '%s' has vol_min %g < 0\n", build->aliasName, build->volMin);
            return 0;
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: sound alias '%s' has pitch_min %g <= 0\n", build->aliasName, build->pitchMin);
        return 0;
    }
}

SndCurve *__cdecl Com_RegisterSoundAliasVolumeFalloffCurve(const char *filename, const char *sourceFile)
{
    int i; // [esp+0h] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 1173, 0, "%s", "filename");
    for (i = 0; i < 16; ++i)
    {
        // KISAKTODO: PSYCHO NEGA-ARRAY
        if (*&g_sa.volumeFalloffCurveNames[-18][72 * i]
            && !I_stricmp(filename, (char*)*&g_sa.volumeFalloffCurveNames[-18][72 * i]))
        {
            return (SndCurve *)&g_sa.volumeFalloffCurveNames[-18][72 * i];
        }
    }
    Com_Error(ERR_DROP, "Sound alias file %s: Volume Falloff Curve %s not found.", sourceFile, filename);
    return 0;
}

SpeakerMap *__cdecl Com_RegisterSoundAliasSpeakerMap(const char *token, const char *sourceFile)
{
    int i; // [esp+0h] [ebp-4h]

    if (!token)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 1197, 0, "%s", "token");
    for (i = 0; i < 16; ++i)
    {
        if (!I_stricmp(token, g_sa.speakerMaps[i].speakerMap.name))
            return &g_sa.speakerMaps[i].speakerMap;
    }
    Com_Error(ERR_DROP, "Sound alias file %s: Speaker Map %s not found.", sourceFile, token);
    return 0;
}

void __cdecl Com_SoundAliasReverb(char *token, snd_alias_build_s *alias)
{
    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 644, 0, "%s", "alias");
    if (!token)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 645, 0, "%s", "token");
    if (strstr(token, "fulldrylevel"))
        alias->bFullDryLevel = 1;
    if (strstr(token, "nowetlevel"))
        alias->bNoWetLevel = 1;
}

void __cdecl Com_LoadSoundAliasField(
    const char *loadspec,
    const char *loadspecCurGame,
    const char *sourceFile,
    char *token,
    snd_alias_members_t field,
    char *isFieldSet,
    snd_alias_build_s *alias)
{
    char v7; // al
    char v8; // dl
    char v9; // dl
    char *soundFile; // [esp+1Ch] [ebp-64h]
    char *v11; // [esp+20h] [ebp-60h]
    char v12; // [esp+27h] [ebp-59h]
    char *chainAliasName; // [esp+2Ch] [ebp-54h]
    char *v14; // [esp+30h] [ebp-50h]
    char *secondaryAliasName; // [esp+3Ch] [ebp-44h]
    char *v16; // [esp+40h] [ebp-40h]
    char *aliasName; // [esp+4Ch] [ebp-34h]
    char *v18; // [esp+50h] [ebp-30h]
    unsigned int i; // [esp+7Ch] [ebp-4h]

    if (field > SA_ENVELOPPERCENTAGE)
        MyAssertHandler(
            ".\\universal\\com_sndalias_load_obj.cpp",
            702,
            0,
            "%s",
            "field >= SA_INVALID && field < SA_NUMFIELDS");
    if (field)
    {
        if (isFieldSet[field])
        {
            Com_PrintError(
                9,
                "ERROR: Sound alias file %s: Duplicate entries for the '%s' column\n",
                sourceFile,
                g_pszSndAliasKeyNames[field]);
            alias->error = 1;
        }
        else
        {
            isFieldSet[field] = 1;
            if (!alias)
                MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 715, 0, "%s", "alias");
            if (!token)
                MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 716, 0, "%s", "token");
            switch (field)
            {
            case SA_NAME:
                if (Com_IsValidName(token, 0x40u, Com_IsValidAliasName, alias, field, sourceFile))
                {
                    v18 = token;
                    aliasName = alias->aliasName;
                    do
                    {
                        v7 = *v18;
                        *aliasName++ = *v18++;
                    } while (v7);
                }
                break;
            case SA_SEQUENCE:
                alias->sequence = atoi(token);
                break;
            case SA_FILE:
                if (Com_IsValidName(token, 0x40u, 0, alias, field, sourceFile))
                {
                    v11 = token;
                    soundFile = alias->soundFile;
                    do
                    {
                        v9 = *v11;
                        *soundFile++ = *v11++;
                    } while (v9);
                }
                break;
            case SA_SUBTITLE:
                for (i = 0; token[i]; ++i)
                {
                    if (token[i] < 0)
                    {
                        Com_PrintError(
                            9,
                            "ERROR: Sound alias file %s: Subtitle '%s' has invalid character '%c' ascii %i\n",
                            sourceFile,
                            token,
                            token[i],
                            token[i]);
                        alias->error = 1;
                        return;
                    }
                }
                alias->subtitleText = (char*)Hunk_AllocateTempMemory(i + 1, "Com_LoadSoundAliasField, SA_SUBTITLE");
                memcpy(alias->subtitleText, token, i);
                alias->subtitleText[i] = 0;
                break;
            case SA_VOL_MIN:
                alias->volMin = Com_ParseFloatInRange(token, 0.0, 1.0, alias, field, sourceFile);
                if (!isFieldSet[6])
                    alias->volMax = alias->volMin;
                break;
            case SA_VOL_MAX:
                alias->volMax = Com_ParseFloatInRange(token, 0.0, 1.0, alias, field, sourceFile);
                break;
            case SA_VOL_MOD:
                alias->fVolMod = Com_GetSoundAliasVolumeModGroupValue(token, sourceFile);
                break;
            case SA_PITCH_MIN:
                alias->pitchMin = atof(token);
                if (!isFieldSet[9])
                    alias->pitchMax = alias->pitchMin;
                break;
            case SA_PITCH_MAX:
                alias->pitchMax = atof(token);
                break;
            case SA_DIST_MIN:
                alias->distMin = atof(token);
                break;
            case SA_DIST_MAX:
                alias->distMax = atof(token);
                break;
            case SA_CHANNEL:
                Com_SoundAliasChannelForName(token, sourceFile, alias);
                break;
            case SA_TYPE:
                Com_SoundAliasTypeForName(token, sourceFile, alias);
                break;
            case SA_LOOP:
                Com_SoundAliasLoop(token, sourceFile, alias);
                break;
            case SA_PROBABILITY:
                alias->probability = atof(token);
                break;
            case SA_LOADSPEC:
                alias->keep = Com_SoundAliasLoadSpec(loadspec, loadspecCurGame, token, sourceFile);
                break;
            case SA_MASTERSLAVE:
                Com_SoundAliasMasterSlave(token, sourceFile, alias);
                break;
            case SA_SECONDARYALIASNAME:
                if (Com_IsValidName(token, 0x40u, Com_IsValidAliasName, alias, field, sourceFile))
                {
                    v16 = token;
                    secondaryAliasName = alias->secondaryAliasName;
                    do
                    {
                        v8 = *v16;
                        *secondaryAliasName++ = *v16++;
                    } while (v8);
                }
                break;
            case SA_CHAINALIASNAME:
                if (Com_IsValidName(token, 0x40u, Com_IsValidAliasName, alias, field, sourceFile))
                {
                    v14 = token;
                    chainAliasName = alias->chainAliasName;
                    do
                    {
                        v12 = *v14;
                        *chainAliasName++ = *v14++;
                    } while (v12);
                }
                break;
            case SA_VOLUMEFALLOFFCURVE:
                alias->volumeFalloffCurve = Com_RegisterSoundAliasVolumeFalloffCurve(token, sourceFile);
                break;
            case SA_STARTDELAY:
                alias->startDelay = atoi(token);
                break;
            case SA_SPEAKERMAP:
                alias->speakerMap = Com_RegisterSoundAliasSpeakerMap(token, sourceFile);
                break;
            case SA_REVERB:
                Com_SoundAliasReverb(token, alias);
                break;
            case SA_LFEPERCENTAGE:
                alias->lfePercentage = Com_ParseFloatInRange(token, 0.0, 1.0, alias, field, sourceFile);
                break;
            case SA_CENTERPERCENTAGE:
                alias->centerPercentage = Com_ParseFloatInRange(token, 0.0, 1.0, alias, field, sourceFile);
                break;
            case SA_ENVELOPMIN:
            case SA_ENVELOPMAX:
            case SA_ENVELOPPERCENTAGE:
                return;
            default:
                if (!alwaysfails)
                    MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 831, 0, "unhandled case");
                break;
            }
        }
    }
}

unsigned int *__cdecl Com_AllocateTempSoundMemory(int size, const char *name)
{
    return Hunk_AllocateTempMemory(size, name);
}

void __cdecl Com_AddBuildSoundAlias(snd_alias_build_s *build)
{
    snd_alias_build_s *alias; // [esp+8h] [ebp-4h]

    alias = (snd_alias_build_s*)Com_AllocateTempSoundMemory(412, "Com_AddBuildSoundAlias");
    qmemcpy(alias, build, sizeof(snd_alias_build_s));
    alias->pNext = saLoadObjGlob.tempAliases;
    saLoadObjGlob.tempAliases = alias;
    ++saLoadObjGlob.tempAliasCount;
}

void __cdecl Com_LoadSoundAliasFile(const char *loadspec, const char *loadspecCurGame, const char *sourceFile)
{
    int bHasName; // [esp+4h] [ebp-620h]
    int bHasFile; // [esp+8h] [ebp-61Ch]
    char filename[64]; // [esp+Ch] [ebp-618h] BYREF
    char *file; // [esp+4Ch] [ebp-5D8h] BYREF
    const char *ptr[257]; // [esp+50h] [ebp-5D4h] BYREF
    snd_alias_build_s alias; // [esp+454h] [ebp-1D0h] BYREF
    const char *token; // [esp+5F4h] [ebp-30h]
    int i; // [esp+5F8h] [ebp-2Ch]
    char isFieldSet[32]; // [esp+5FCh] [ebp-28h] BYREF
    int iColCount; // [esp+620h] [ebp-4h]

    memset((unsigned __int8 *)&alias, 0, sizeof(alias));
    Com_sprintf(filename, 0x40u, "soundaliases/%s", sourceFile);
    if (FS_ReadFile(filename, (void **)&file) >= 0)
    {
        if (!saLoadObjGlob.volumeModGroupsInitialized)
        {
            if (!saLoadObjGlob.refreshVolumeModGroupsCommandInitialized)
            {
                Cmd_AddCommandInternal(
                    "snd_refreshVolumeModGroups",
                    Com_RefreshVolumeModGroups_f,
                    &Com_RefreshVolumeModGroups_f_VAR);
                saLoadObjGlob.refreshVolumeModGroupsCommandInitialized = 1;
            }
            Com_LoadVolumeModGroups(saLoadObjGlob.volumeModGroups);
            saLoadObjGlob.volumeModGroupsInitialized = 1;
        }
        Com_BeginParseSession(filename);
        Com_SetCSV(1);
        ptr[0] = file;
        iColCount = 0;
        while (1)
        {
            token = (const char *)Com_Parse(ptr);
            if (!ptr[0])
                break;
            if (*token && *token != 35)
            {
                if (iColCount)
                {
                    memset(isFieldSet, 0, 29);
                    Com_LoadSoundAliasDefaults(&alias, sourceFile, loadspec);
                    i = 0;
                    while (1)
                    {
                        if (*token)
                            Com_LoadSoundAliasField(
                                loadspec,
                                loadspecCurGame,
                                sourceFile,
                                (char *)token,
                                (snd_alias_members_t)(int)ptr[i + 1],
                                isFieldSet,
                                &alias);
                        if (++i == iColCount)
                            break;
                        token = (const char *)Com_ParseOnLine(ptr);
                    }
                    if (!isFieldSet[1] || !isFieldSet[3])
                    {
                        if (saLoadObjGlob.tempAliases)
                            Com_PrintError(
                                9,
                                "ERROR: Sound alias file '%s': alias entry missing name and/or file. Error details:\n"
                                "\talias name: '%s', sound file: '%s', previous alias in file: '%s'\n",
                                sourceFile,
                                alias.aliasName,
                                alias.soundFile,
                                saLoadObjGlob.tempAliases->aliasName);
                        else
                            Com_PrintError(
                                9,
                                "ERROR: Sound alias file '%s': alias entry missing name and/or file. Error details:\n"
                                "\talias name: '%s', sound file: '%s', previous alias in file: '%s'\n",
                                sourceFile,
                                alias.aliasName,
                                alias.soundFile,
                                "");
                        break;
                    }
                    if (!isFieldSet[16])
                        alias.keep = Com_SoundAliasLoadSpec(loadspec, loadspecCurGame, "", sourceFile);
                    if (alias.keep && !alias.error && Com_FinishBuildingSoundAlias(&alias))
                        Com_AddBuildSoundAlias(&alias);
                }
                else
                {
                    bHasName = 0;
                    bHasFile = 0;
                    while (2)
                    {
                        ptr[iColCount + 1] = 0;
                        for (i = 1; i < 29; ++i)
                        {
                            if (!I_stricmp(g_pszSndAliasKeyNames[i], token))
                            {
                                ptr[iColCount + 1] = (const char *)i;
                                if (i == 1)
                                {
                                    bHasName = 1;
                                }
                                else if (i == 3)
                                {
                                    bHasFile = 1;
                                }
                                break;
                            }
                        }
                        if (++iColCount != 256 && ptr[0] && *ptr[0] != 10)
                        {
                            token = (const char *)Com_ParseOnLine(ptr);
                            continue;
                        }
                        break;
                    }
                    if (!bHasName || !bHasFile)
                    {
                        Com_PrintError(9, "ERROR: Sound alias file %s: missing 'name' and/or 'file' columns\n", sourceFile);
                        Com_EndParseSession();
                        return;
                    }
                }
            }
            Com_SkipRestOfLine(ptr);
        }
        Com_EndParseSession();
    }
}

bool __cdecl Com_ParseSndCurveFile(const char *buffer, const char *fileName, SndCurve *curve)
{
    int v3; // eax
    long double v5; // st7
    long double v6; // st7
    int knotCountIndex; // [esp+8h] [ebp-8h]
    int knotCountIndexa; // [esp+8h] [ebp-8h]
    parseInfo_t *tokenb; // [esp+Ch] [ebp-4h]
    parseInfo_t *token; // [esp+Ch] [ebp-4h]
    parseInfo_t *tokena; // [esp+Ch] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1898, 0, "%s", "buffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1899, 0, "%s", "fileName");
    if (!curve)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1900, 0, "%s", "curve");
    Com_BeginParseSession(fileName);
    tokenb = Com_Parse(&buffer);
    v3 = atoi(tokenb->token);
    curve->knotCount = v3;
    if (curve->knotCount >= 2)
    {
        if (curve->knotCount <= 8)
        {
            for (knotCountIndex = 0; ; ++knotCountIndex)
            {
                token = Com_Parse(&buffer);
                if (!token->token[0] || token->token[0] == 125)
                    break;
                if (knotCountIndex >= 8)
                {
                    Com_EndParseSession();
                    Com_PrintError(
                        9,
                        "ERROR: sndcurve parse failure on file \"%s\": knots parsed (%d) is greater than or equal to maxKnots (%d)\n",
                        fileName,
                        knotCountIndex,
                        8);
                    return 0;
                }
                v5 = atof(token->token);
                curve->knots[knotCountIndex][0] = v5;
                if (curve->knots[knotCountIndex][0] < 0.0 || curve->knots[knotCountIndex][0] > 1.0)
                {
                    Com_EndParseSession();
                    Com_PrintError(
                        9,
                        "ERROR: sndcurve parse failure on file \"%s\": knot x-coord '%f' is not in the range 0-1.\n",
                        fileName,
                        curve->knots[knotCountIndex][0]);
                    return 0;
                }
                tokena = Com_Parse(&buffer);
                if (!tokena->token[0] || tokena->token[0] == 125)
                    break;
                v6 = atof(tokena->token);
                curve->knots[knotCountIndex][1] = v6;
                if (curve->knots[knotCountIndex][1] < 0.0 || curve->knots[knotCountIndex][1] > 1.0)
                {
                    Com_EndParseSession();
                    Com_PrintError(
                        9,
                        "ERROR: sndcurve parse failure on file \"%s\": knot x-coord '%f' is not in the range 0-1.\n",
                        fileName,
                        curve->knots[knotCountIndex][1]);
                    return 0;
                }
            }
            Com_EndParseSession();
            if (knotCountIndex == curve->knotCount)
            {
                knotCountIndexa = knotCountIndex - 1;
                if (curve->knots[0][0] != 0.0
                    || curve->knots[0][1] != 1.0
                    || curve->knots[knotCountIndexa][0] != 1.0
                    || curve->knots[knotCountIndexa][1] != 0.0)
                {
                    curve->knots[0][0] = 0.0;
                    curve->knots[0][1] = 1.0;
                    curve->knots[knotCountIndexa][0] = 1.0;
                    curve->knots[knotCountIndexa][1] = 0.0;
                    Com_PrintWarning(
                        9,
                        "WARNING^7: sndcurve parse on file \"%s\": the first point must be '0.0000 1.0000' and the last point must be"
                        " '1.0000 0.0000'.\n"
                        "adjusting sndcurve endpoints.\n",
                        fileName);
                }
                return 1;
            }
            else
            {
                Com_PrintError(
                    9,
                    "ERROR: sndcurve parse failure on file \"%s\": knot count (%d) does not match knots parsed (%d).\n",
                    fileName,
                    curve->knotCount,
                    knotCountIndex);
                return 0;
            }
        }
        else
        {
            Com_EndParseSession();
            Com_PrintError(
                9,
                "ERROR: sndcurve parse failure on file \"%s\": knot count (%d) is greater than maxKnots (%d)\n",
                fileName,
                curve->knotCount,
                8);
            return 0;
        }
    }
    else
    {
        Com_EndParseSession();
        Com_PrintError(
            9,
            "ERROR: sndcurve parse failure on file \"%s\": knot count (%d) is less than 2\n",
            fileName,
            curve->knotCount);
        return 0;
    }
}

snd_alias_build_s *__cdecl Com_SortTempSoundAliases_r(
    snd_alias_build_s *pAliasList,
    int *piAliasCount,
    int(__cdecl *test)(snd_alias_build_s *, snd_alias_build_s *),
    bool isRemovingDups)
{
    snd_alias_build_s *pFrontList; // [esp+0h] [ebp-18h]
    int iFrontCount; // [esp+4h] [ebp-14h] BYREF
    snd_alias_build_s **ppListPos; // [esp+8h] [ebp-10h]
    int iBackCount; // [esp+Ch] [ebp-Ch] BYREF
    snd_alias_build_s *pBackList; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    if (!pAliasList)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1290, 0, "%s", "pAliasList");
    if (*piAliasCount <= 0)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1291, 0, "%s", "*piAliasCount > 0");
    if (*piAliasCount == 1)
    {
        pAliasList->pNext = 0;
        return pAliasList;
    }
    else
    {
        iFrontCount = *piAliasCount / 2;
        iBackCount = *piAliasCount - iFrontCount;
        i = 0;
        pBackList = pAliasList;
        while (i < iFrontCount)
        {
            ++i;
            pBackList = pBackList->pNext;
        }
        pFrontList = Com_SortTempSoundAliases_r(pAliasList, &iFrontCount, test, isRemovingDups);
        pBackList = Com_SortTempSoundAliases_r(pBackList, &iBackCount, test, isRemovingDups);
        *piAliasCount = 0;
        pAliasList = 0;
        ppListPos = &pAliasList;
        while (iFrontCount && iBackCount)
        {
            i = test(pFrontList, pBackList);
            if (!isRemovingDups || i || (i = pFrontList->sequence - pBackList->sequence) != 0)
            {
                if (i > 0)
                {
                    *ppListPos = pBackList;
                    pBackList = pBackList->pNext;
                    --iBackCount;
                }
                else
                {
                    *ppListPos = pFrontList;
                    pFrontList = pFrontList->pNext;
                    --iFrontCount;
                }
                ++ * piAliasCount;
                ppListPos = &(*ppListPos)->pNext;
            }
            else
            {
                i = I_stricmp(pFrontList->szSourceFile, pBackList->szSourceFile);
                if (i)
                {
                    if (i >= 0)
                    {
                        pBackList = pBackList->pNext;
                        --iBackCount;
                    }
                    else
                    {
                        pFrontList = pFrontList->pNext;
                        --iFrontCount;
                    }
                }
                else
                {
                    Com_PrintError(
                        9,
                        "ERROR: sound alias file %s: duplicate alias '%s'\n",
                        pFrontList->szSourceFile,
                        pFrontList->aliasName);
                    pFrontList = pFrontList->pNext;
                    --iFrontCount;
                    pBackList = pBackList->pNext;
                    --iBackCount;
                }
            }
        }
        if (iFrontCount)
        {
            *ppListPos = pFrontList;
            *piAliasCount += iFrontCount;
        }
        else
        {
            *ppListPos = pBackList;
            *piAliasCount += iBackCount;
        }
        return pAliasList;
    }
}

int __cdecl AliasNameCompare(snd_alias_build_s *pFrontList, snd_alias_build_s *pBackList)
{
    return I_stricmp(pFrontList->aliasName, pBackList->aliasName);
}

int __cdecl FileNameTypeCompare(snd_alias_build_s *frontList, snd_alias_build_s *backList)
{
    int fileComp; // [esp+4h] [ebp-4h]

    fileComp = I_stricmp(frontList->soundFile, backList->soundFile);
    if (fileComp)
        return fileComp;
    if (frontList->eType == backList->eType)
        return AliasNameCompare(frontList, backList);
    return frontList->eType - backList->eType;
}

void __cdecl Com_SameFileWarning(snd_alias_build_s *alias1, snd_alias_build_s *alias2)
{
    const char *alias1TypeString; // [esp+20h] [ebp-Ch]

    if (strcmp(alias1->soundFile, alias2->soundFile))
        MyAssertHandler(
            ".\\universal\\com_sndalias_load_obj.cpp",
            1427,
            0,
            "%s",
            "strcmp( alias1->soundFile, alias2->soundFile ) == 0");
    if (alias1->eType == alias2->eType)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1428, 0, "%s", "alias1->eType != alias2->eType");
    if (alias1->eType == SAT_STREAMED)
        alias1TypeString = "streamed";
    else
        alias1TypeString = "loaded";
    if (alias2->eType == SAT_STREAMED)
        Com_PrintWarning(
            9,
            "WARNING: sound file '%s' used as %s in alias '%s' and %s in alias '%s'\n",
            alias1->soundFile,
            alias1TypeString,
            alias1->aliasName,
            "streamed",
            alias2->aliasName);
    else
        Com_PrintWarning(
            9,
            "WARNING: sound file '%s' used as %s in alias '%s' and %s in alias '%s'\n",
            alias1->soundFile,
            alias1TypeString,
            alias1->aliasName,
            "loaded",
            alias2->aliasName);
}

void __cdecl Com_AddSoundAlias(
    snd_alias_build_s *build,
    snd_alias_t *alias,
    const char *aliasName,
    SoundFile *soundFile,
    const char *subtitle)
{
    int v5; // eax
    int v6; // eax
    int v7; // edx
    int v8; // ecx
    int v9; // eax
    char v10; // [esp+3h] [ebp-3Dh]
    char *v11; // [esp+8h] [ebp-38h]
    char *chainAliasName; // [esp+Ch] [ebp-34h]
    char v13; // [esp+23h] [ebp-1Dh]
    char *v14; // [esp+28h] [ebp-18h]
    char *secondaryAliasName; // [esp+2Ch] [ebp-14h]

    if (!build->aliasName[0])
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1538, 0, "%s", "build->aliasName[0]");
    if (!build->soundFile[0])
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1539, 0, "%s", "build->soundFile[0]");
    alias->aliasName = aliasName;
    if (build->secondaryAliasName[0])
    {
        alias->secondaryAliasName = (const char*)CM_Hunk_Alloc(strlen(build->secondaryAliasName) + 1, "_secondary alias strings", 15);
        secondaryAliasName = build->secondaryAliasName;
        v14 = (char*)alias->secondaryAliasName;
        do
        {
            v13 = *secondaryAliasName;
            *v14++ = *secondaryAliasName++;
        } while (v13);
    }
    else
    {
        alias->secondaryAliasName = 0;
    }
    if (build->chainAliasName[0])
    {
        alias->chainAliasName = (const char*)CM_Hunk_Alloc(strlen(build->chainAliasName) + 1, "_chain alias strings", 15);
        chainAliasName = build->chainAliasName;
        v11 = (char*)alias->chainAliasName;
        do
        {
            v10 = *chainAliasName;
            *v11++ = *chainAliasName++;
        } while (v10);
    }
    else
    {
        alias->chainAliasName = 0;
    }
    alias->soundFile = soundFile;
    alias->subtitle = subtitle;
    alias->sequence = 0;
    alias->volMin = build->volMin;
    alias->volMax = build->volMax;
    alias->pitchMin = build->pitchMin;
    alias->pitchMax = build->pitchMax;
    alias->distMin = build->distMin;
    alias->distMax = build->distMax;
    alias->flags = (build->iChannel << 8) | alias->flags & 0xFFFFC0FF;
    alias->flags = (build->eType << 6) | alias->flags & 0xFFFFFF3F;
    alias->volumeFalloffCurve = build->volumeFalloffCurve;
    alias->speakerMap = build->speakerMap;
    if (build->bLooping)
        v5 = alias->flags | 1;
    else
        v5 = alias->flags & 0xFFFFFFFE;
    alias->flags = v5;
    if (build->bRandomLooping)
    {
        if (!build->bLooping)
            MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1589, 0, "%s", "build->bLooping");
        alias->flags |= 0x20u;
    }
    else
    {
        alias->flags &= ~0x20u;
    }
    if (build->bMaster)
        v6 = alias->flags | 2;
    else
        v6 = alias->flags & 0xFFFFFFFD;
    alias->flags = v6;
    if (build->bSlave)
        v7 = alias->flags | 4;
    else
        v7 = alias->flags & 0xFFFFFFFB;
    alias->flags = v7;
    if (build->bFullDryLevel)
        v8 = alias->flags | 8;
    else
        v8 = alias->flags & 0xFFFFFFF7;
    alias->flags = v8;
    if (build->bNoWetLevel)
        v9 = alias->flags | 0x10;
    else
        v9 = alias->flags & 0xFFFFFFEF;
    alias->flags = v9;
    alias->slavePercentage = build->slavePercentage;
    alias->probability = build->probability;
    alias->lfePercentage = build->lfePercentage;
    alias->centerPercentage = build->centerPercentage;
    alias->startDelay = build->startDelay;
}

void __cdecl Com_AddLoadedSoundFile(SoundFile *soundFile, char *fileName)
{
    I_strlwr(fileName);
    soundFile->u.loadSnd = SND_LoadSoundFile(fileName);
    if (soundFile->u.loadSnd)
    {
        soundFile->exists = 1;
    }
    else
    {
        soundFile->exists = 0;
        soundFile->u.loadSnd = (LoadedSound*)CM_Hunk_Alloc(0x2Cu, "_loaded", 15);
        soundFile->u.loadSnd->name = fileName;
    }
}

void __cdecl Com_AddStreamedSoundFile(SoundFile *soundFile, char *fileName)
{
    const char *name; // [esp+0h] [ebp-4h]

    soundFile->exists = 1;
    name = Com_GetFilenameSubString(fileName);
    if (name == fileName)
        fileName = (char*)"";
    else
        *(char*)(name - 1) = 0;
    soundFile->u.loadSnd = (LoadedSound*)fileName;
    soundFile->u.streamSnd.filename.info.raw.name = name;
}

void __cdecl Com_MakeSoundAliasesPermanent(snd_alias_list_t *aliasInfo, SoundFileInfo *soundFileInfo)
{
    char v2; // [esp+13h] [ebp-D5h]
    char *v3; // [esp+18h] [ebp-D0h]
    char *soundFile; // [esp+1Ch] [ebp-CCh]
    char v5; // [esp+33h] [ebp-B5h]
    char *v6; // [esp+38h] [ebp-B0h]
    char *subtitleText; // [esp+3Ch] [ebp-ACh]
    char v8; // [esp+53h] [ebp-95h]
    char *v9; // [esp+58h] [ebp-90h]
    char *aliasName; // [esp+5Ch] [ebp-8Ch]
    bool v11; // [esp+80h] [ebp-68h]
    bool v12; // [esp+84h] [ebp-64h]
    bool v13; // [esp+88h] [ebp-60h]
    snd_alias_type_t eType; // [esp+9Ch] [ebp-4Ch]
    char *fileName; // [esp+A0h] [ebp-48h]
    int savedBytesCount; // [esp+A4h] [ebp-44h]
    char *strings; // [esp+B0h] [ebp-38h]
    SoundFile *currentSound; // [esp+B4h] [ebp-34h]
    char *currentName; // [esp+B8h] [ebp-30h]
    char *currentNamea; // [esp+B8h] [ebp-30h]
    char *currentNameb; // [esp+B8h] [ebp-30h]
    char *subtitle; // [esp+BCh] [ebp-2Ch]
    unsigned int aliasCount; // [esp+C0h] [ebp-28h]
    snd_alias_build_s *other; // [esp+C4h] [ebp-24h]
    snd_alias_list_t *aliasList; // [esp+C8h] [ebp-20h]
    snd_alias_build_s *build; // [esp+CCh] [ebp-1Ch]
    snd_alias_build_s *builda; // [esp+CCh] [ebp-1Ch]
    snd_alias_build_s *buildb; // [esp+CCh] [ebp-1Ch]
    snd_alias_t *alias; // [esp+D0h] [ebp-18h]
    int soundCount; // [esp+D4h] [ebp-14h]
    int bytesCount; // [esp+D8h] [ebp-10h]
    int bytesCounta; // [esp+D8h] [ebp-10h]
    unsigned int stringBytesCount; // [esp+E0h] [ebp-8h]

    soundFileInfo->count = 0;
    aliasInfo->count = 0;
    if (saLoadObjGlob.tempAliasCount)
    {
        saLoadObjGlob.tempAliases = Com_SortTempSoundAliases_r(
            saLoadObjGlob.tempAliases,
            &saLoadObjGlob.tempAliasCount,
            AliasNameCompare,
            1);
        if (saLoadObjGlob.tempAliases)
        {
            saLoadObjGlob.tempAliases = Com_SortTempSoundAliases_r(
                saLoadObjGlob.tempAliases,
                &saLoadObjGlob.tempAliasCount,
                FileNameTypeCompare,
                0);
            if (saLoadObjGlob.tempAliases)
            {
                stringBytesCount = 0;
                savedBytesCount = 0;
                other = 0;
                currentName = 0;
                soundCount = 0;
                for (build = saLoadObjGlob.tempAliases; build; build = build->pNext)
                {
                    bytesCount = strlen(build->soundFile) + 1;
                    v13 = other && build->eType == other->eType;
                    v12 = currentName && !I_stricmp(currentName, build->soundFile);
                    v11 = v12 && v13;
                    if (currentName && v11)
                    {
                        savedBytesCount += bytesCount;
                        build->pSameSoundFile = other;
                    }
                    else
                    {
                        if (v12 && !v13)
                            Com_SameFileWarning(build, other);
                        other = build;
                        currentName = build->soundFile;
                        build->pSameSoundFile = 0;
                        stringBytesCount += bytesCount;
                        ++soundCount;
                    }
                }
                saLoadObjGlob.tempAliases = Com_SortTempSoundAliases_r(
                    saLoadObjGlob.tempAliases,
                    &saLoadObjGlob.tempAliasCount,
                    AliasNameCompare,
                    1);
                if (saLoadObjGlob.tempAliases)
                {
                    aliasCount = 0;
                    currentNamea = 0;
                    for (builda = saLoadObjGlob.tempAliases; builda; builda = builda->pNext)
                    {
                        bytesCounta = strlen(builda->aliasName) + 1;
                        if (currentNamea && !I_stricmp(currentNamea, builda->aliasName))
                        {
                            savedBytesCount += bytesCounta;
                        }
                        else
                        {
                            ++aliasCount;
                            stringBytesCount += bytesCounta;
                            currentNamea = builda->aliasName;
                        }
                        if (builda->subtitleText)
                            stringBytesCount += strlen(builda->subtitleText) + 1;
                    }
                    Com_InitSoundAliasHash(aliasCount);
                    aliasInfo->head = (snd_alias_t*)CM_Hunk_Alloc(92 * saLoadObjGlob.tempAliasCount, "_aliases", 15);
                    soundFileInfo->files = (SoundFile*)CM_Hunk_Alloc(sizeof(SoundFile) * soundCount, "_sound files", 15);
                    strings = (char*)CM_Hunk_Alloc(stringBytesCount, "_strings", 15);
                    currentNameb = 0;
                    aliasList = 0;
                    for (buildb = saLoadObjGlob.tempAliases; ; buildb = buildb->pNext)
                    {
                        if (!buildb)
                            return;
                        if (!currentNameb || I_stricmp(currentNameb, buildb->aliasName))
                        {
                            currentNameb = strings;
                            aliasName = buildb->aliasName;
                            v9 = strings;
                            do
                            {
                                v8 = *aliasName;
                                *v9++ = *aliasName++;
                            } while (v8);
                            strings += strlen(strings) + 1;
                        }
                        if (buildb->subtitleText)
                        {
                            subtitle = strings;
                            subtitleText = buildb->subtitleText;
                            v6 = strings;
                            do
                            {
                                v5 = *subtitleText;
                                *v6++ = *subtitleText++;
                            } while (v5);
                            strings += strlen(strings) + 1;
                        }
                        else
                        {
                            subtitle = 0;
                        }
                        alias = &aliasInfo->head[aliasInfo->count];
                        if (!aliasList || I_stricmp(aliasList->head->aliasName, currentNameb))
                        {
                            aliasList = (snd_alias_list_t*)CM_Hunk_Alloc(0xCu, "_alias list", 15);
                            if (!Com_AddAliasList(currentNameb, aliasList))
                            {
                                aliasList = 0;
                                Com_PrintError(9, "ERROR: alias '%s' already added - ignoring\n", currentNameb);
                                continue;
                            }
                            aliasList->aliasName = currentNameb;
                            aliasList->head = alias;
                        }
                        if (buildb->pSameSoundFile)
                        {
                            currentSound = buildb->pSameSoundFile->permSoundFile;
                        }
                        else
                        {
                            fileName = strings;
                            soundFile = buildb->soundFile;
                            v3 = strings;
                            do
                            {
                                v2 = *soundFile;
                                *v3++ = *soundFile++;
                            } while (v2);
                            strings += strlen(strings) + 1;
                            currentSound = &soundFileInfo->files[soundFileInfo->count];
                            eType = buildb->eType;
                            if (!fileName)
                                MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 1521, 0, "%s", "fileName");
                            currentSound->type = eType;
                            if (eType == SAT_LOADED)
                                Com_AddLoadedSoundFile(currentSound, fileName);
                            else
                                Com_AddStreamedSoundFile(currentSound, fileName);
                            ++soundFileInfo->count;
                        }
                        buildb->permSoundFile = currentSound;
                        Com_AddSoundAlias(buildb, alias, currentNameb, currentSound, subtitle);
                        ++aliasInfo->count;
                        ++aliasList->count;
                    }
                }
            }
        }
    }
}

int __cdecl Com_LoadSoundAliasSounds(SoundFileInfo *soundFileInfo)
{
    unsigned __int8 v1; // al
    char filepath[256]; // [esp+0h] [ebp-110h] BYREF
    int numMissing; // [esp+104h] [ebp-Ch]
    int soundIndex; // [esp+108h] [ebp-8h]
    SoundFile *soundFile; // [esp+10Ch] [ebp-4h]

    numMissing = 0;
    for (soundIndex = 0; soundIndex < soundFileInfo->count; ++soundIndex)
    {
        soundFile = &soundFileInfo->files[soundIndex];
        if (soundFile->type == 1)
        {
            if (!soundFile->u.loadSnd)
                MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2067, 0, "%s", "soundFile->u.loadSnd");
            if (!soundFile->exists)
                ++numMissing;
        }
        else
        {
            if (soundFile->type != 2)
                MyAssertHandler(
                    ".\\universal\\com_sndalias_load_obj.cpp",
                    2073,
                    0,
                    "%s\n\t(soundFile->type) = %i",
                    "(soundFile->type == SAT_STREAMED)",
                    soundFile->type);
            if (!soundFile->exists)
                MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2074, 0, "%s", "soundFile->exists");
            if (snd_touchStreamFilesOnLoad->current.enabled || fs_copyfiles->current.enabled)
            {
                ProfLoad_Begin("Verify streamed sound");
                Com_sprintf(
                    filepath,
                    0x100u,
                    "sound/%s/%s",
                    soundFile->u.streamSnd.filename.info.raw.dir,
                    soundFile->u.streamSnd.filename.info.raw.name);
                v1 = FS_TouchFile(filepath);
                soundFile->exists = v1;
                ProfLoad_End();
                if (!soundFile->exists)
                    Com_PrintError(9, "ERROR: Streamed sound file '%s' not found\n", filepath);
            }
        }
    }
    return numMissing;
}

void __cdecl Com_ParseEntChannelFile(const char *buffer)
{
    char v1; // [esp+3h] [ebp-25h]
    char *v2; // [esp+8h] [ebp-20h]
    parseInfo_t *v3; // [esp+Ch] [ebp-1Ch]
    int i; // [esp+20h] [ebp-8h]
    parseInfo_t *value; // [esp+24h] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 960, 0, "%s", "buffer");
    saLoadObjGlob.entChannelCount = 0;
    Com_BeginParseSession("soundaliases/channels.def");
    Com_SetCSV(1);
    while (1)
    {
        value = Com_Parse(&buffer);
        if (!buffer)
            break;
        if (value->token[0] && value->token[0] != 35)
        {
            if (strlen(value->token) > 0x40)
            {
                Com_EndParseSession();
                Com_Error(
                    ERR_DROP,
                    "channel name too long (max chars %d): %s in file [%s].\n",
                    64,
                    value->token,
                    "soundaliases/channels.def");
            }
            for (i = 0; i < saLoadObjGlob.entChannelCount; ++i)
            {
                if (!I_stricmp(saLoadObjGlob.entChannels[i], value->token))
                {
                    Com_EndParseSession();
                    Com_Error(ERR_DROP, "duplicate channel name '%s' in file [%s].\n", value->token, "soundaliases/channels.def");
                }
            }
            v3 = value;
            v2 = saLoadObjGlob.entChannels[saLoadObjGlob.entChannelCount];
            do
            {
                v1 = v3->token[0];
                *v2 = v3->token[0];
                v3 = (v3 + 1);
                ++v2;
            } while (v1);
            if (++saLoadObjGlob.entChannelCount > 64)
            {
                Com_EndParseSession();
                Com_Error(ERR_DROP, "exceeded max number of channels (%d) in file [%s].\n", 64, "soundaliases/channels.def");
            }
        }
        Com_SkipRestOfLine(&buffer);
    }
    Com_EndParseSession();
}

void __cdecl Com_SetChannelMapEntry(
    MSSChannelMap *entry,
    unsigned int inputChannel,
    unsigned int outputChannel,
    float volume)
{
    MSSSpeakerLevels *speaker; // [esp+0h] [ebp-4h]

    if (!entry)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2531, 0, "%s", "entry");
    if (inputChannel > 2)
        MyAssertHandler(
            ".\\universal\\com_sndalias_load_obj.cpp",
            2532,
            0,
            "%s",
            "inputChannel >= 0 && inputChannel <= MSS_MAXSRCCHANNELS");
    if (outputChannel > 6)
        MyAssertHandler(
            ".\\universal\\com_sndalias_load_obj.cpp",
            2533,
            0,
            "%s",
            "outputChannel >= 0 && outputChannel <= MSS_MAXDSTCHANNELS");
    speaker = &entry->speakers[outputChannel];
    if (entry->speakers[outputChannel].numLevels <= inputChannel)
        entry->speakers[outputChannel].numLevels = inputChannel + 1;
    speaker->levels[inputChannel] = volume;
    speaker->speaker = outputChannel;
}

void Com_InitSoundDevGuiGraphs_LoadObj()
{
    char devguiPath[256]; // [esp+0h] [ebp-108h] BYREF
    int i; // [esp+104h] [ebp-4h]

    if (!g_sa.curvesInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 240, 0, "%s", "g_sa.curvesInitialized");
    for (i = 1; i < 16; ++i)
    {
        if (*&g_sa.volumeFalloffCurveNames[-18][72 * i])
        {
            sprintf(devguiPath, "Main/Snd:6/Volume Falloff Curves/%s:%d", *&g_sa.volumeFalloffCurveNames[-18][72 * i], i);
            g_sa.curveDevGraphs[i].knotCountMax = 8;
            g_sa.curveDevGraphs[i].knots = g_sa.volumeFalloffCurves[i].knots;
            g_sa.curveDevGraphs[i].knotCount = &g_sa.volumeFalloffCurves[i].knotCount;
            g_sa.curveDevGraphs[i].eventCallback = Com_VolumeFalloffCurveGraphEventCallback;
            g_sa.curveDevGraphs[i].data = (void*)i;
            g_sa.curveDevGraphs[i].disableEditingEndPoints = 1;
            DevGui_AddGraph(devguiPath, &g_sa.curveDevGraphs[i]);
        }
    }
}

void __cdecl Com_PreLoadSpkrMapFile(SpeakerMapInfo *info)
{
    info->speakerMap.channelMaps[0][0].speakerCount = 2;
    info->speakerMap.channelMaps[1][0].speakerCount = 2;
    info->speakerMap.channelMaps[0][1].speakerCount = 6;
    info->speakerMap.channelMaps[1][1].speakerCount = 6;
    info->speakerMap.name = info->name;
}

char __cdecl Com_LoadSpkrMapParseBuffer(char *fileName, char *buffer)
{
    signed int fileLength; // [esp+10h] [ebp-10h]
    int fileHandle; // [esp+18h] [ebp-8h] BYREF
    int identifierStrLength; // [esp+1Ch] [ebp-4h]

    identifierStrLength = strlen("SPKRMAP");
    fileLength = FS_FOpenFileByMode(fileName, &fileHandle, FS_READ);
    if (fileLength > 0)
    {
        if (identifierStrLength >= 0x2000)
            MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2620, 0, "%s", "identifierStrLength < BIG_INFO_STRING");
        FS_Read((unsigned char *)buffer, identifierStrLength, fileHandle);
        buffer[identifierStrLength] = 0;
        if (!strncmp(buffer, "SPKRMAP", identifierStrLength))
        {
            if (fileLength - identifierStrLength < 0x2000)
            {
                FS_Read((unsigned char*)buffer, fileLength - identifierStrLength, fileHandle);
                buffer[fileLength - identifierStrLength] = 0;
                FS_FCloseFile(fileHandle);
                return 1;
            }
            else
            {
                Com_PrintError(9, "ERROR: \"%s\" Is too long of a spkrmap file to parse\n", fileName);
                return 0;
            }
        }
        else
        {
            Com_PrintError(9, "ERROR: \"%s\" does not appear to be a spkrmap file\n", fileName);
            return 0;
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: Could not load spkrmap file '%s'\n", fileName);
        return 0;
    }
}

const char *g_spkrMapParseBuffer;
const char *sa_spkrMapIdentifierStrings[9] =
{
  "MONOSOURCE",
  "LEFTSOURCE",
  "RIGHTSOURCE",
  "LEFTSPEAKER",
  "RIGHTSPEAKER",
  "CENTERSPEAKER",
  "LFESPEAKER",
  "LEFTSURROUNDSPEAKER",
  "RIGHTSURROUNDSPEAKER"
};
int sa_spkrMapIdentifierValues[9] =
{ 0, 0, 1, 0, 1, 2, 3, 4, 5 };


char __cdecl Com_ParseChannelMapEntry(
    const char *fileName,
    MSSChannelMap *entry,
    SA_SPKRMAPIDENTIFIERS inputChannel,
    SA_SPKRMAPIDENTIFIERS outputChannel)
{
    float volume; // [esp+8h] [ebp-8h]
    parseInfo_t *token; // [esp+Ch] [ebp-4h]
    parseInfo_t *tokena; // [esp+Ch] [ebp-4h]
    parseInfo_t *tokenb; // [esp+Ch] [ebp-4h]

    if (!g_spkrMapParseBuffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2656, 0, "%s", "g_spkrMapParseBuffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2657, 0, "%s", "fileName");
    if (!entry)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2658, 0, "%s", "entry");
    token = Com_Parse(&g_spkrMapParseBuffer);
    if (I_stricmp(token->token, sa_spkrMapIdentifierStrings[inputChannel]))
    {
        Com_PrintError(
            9,
            "ERROR: spkrMap parse failure on file \"%s\": got token '%s', expected '%s'.\n",
            fileName,
            token->token,
            sa_spkrMapIdentifierStrings[inputChannel]);
        return 0;
    }
    else
    {
        tokena = Com_Parse(&g_spkrMapParseBuffer);
        if (I_stricmp(tokena->token, sa_spkrMapIdentifierStrings[outputChannel]))
        {
            Com_PrintError(
                9,
                "ERROR: spkrMap parse failure on file \"%s\": got token '%s', expected '%s'.\n",
                fileName,
                tokena->token,
                sa_spkrMapIdentifierStrings[outputChannel]);
            return 0;
        }
        else
        {
            tokenb = Com_Parse(&g_spkrMapParseBuffer);
            if (tokenb->token[0])
            {
                volume = atof(tokenb->token);
                if (volume >= 0.0 && volume <= 1.0)
                {
                    Com_SetChannelMapEntry(
                        entry,
                        sa_spkrMapIdentifierValues[inputChannel],
                        sa_spkrMapIdentifierValues[outputChannel],
                        volume);
                    return 1;
                }
                else
                {
                    Com_PrintError(
                        9,
                        "ERROR: spkrMap parse failure on file \"%s\": volume '%f' is not in the range 0-1.",
                        fileName,
                        volume);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(
                    9,
                    "ERROR: spkrMap parse failure on file \"%s\": token '%s' is not a float value in the range 0-1.\n",
                    fileName,
                    tokenb->token);
                return 0;
            }
        }
    }
}

bool __cdecl Com_ParseMonoSourceStereoOutput(const char *fileName, SpeakerMap *speakerMap)
{
    if (!g_spkrMapParseBuffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2702, 0, "%s", "g_spkrMapParseBuffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2703, 0, "%s", "fileName");
    if (!speakerMap)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2704, 0, "%s", "speakerMap");
    return Com_ParseChannelMapEntry(fileName, speakerMap->channelMaps[0], SA_MONOSOURCE, SA_LEFTSPEAKER)
        && Com_ParseChannelMapEntry(fileName, speakerMap->channelMaps[0], SA_MONOSOURCE, SA_RIGHTSPEAKER) != 0;
}

bool __cdecl Com_ParseStereoSourceStereoOutput(const char *fileName, SpeakerMap *speakerMap)
{
    MSSChannelMap *entries; // [esp+0h] [ebp-4h]

    if (!g_spkrMapParseBuffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2725, 0, "%s", "g_spkrMapParseBuffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2726, 0, "%s", "fileName");
    if (!speakerMap)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2727, 0, "%s", "speakerMap");
    entries = speakerMap->channelMaps[1];
    if (!Com_ParseChannelMapEntry(fileName, speakerMap->channelMaps[1], SA_LEFTSOURCE, SA_LEFTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_LEFTSPEAKER))
        return 0;
    if (Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_RIGHTSPEAKER))
        return Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_RIGHTSPEAKER) != 0;
    return 0;
}

bool __cdecl Com_ParseMonoSource51Output(const char *fileName, SpeakerMap *speakerMap)
{
    MSSChannelMap *entries; // [esp+0h] [ebp-4h]

    if (!g_spkrMapParseBuffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2752, 0, "%s", "g_spkrMapParseBuffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2753, 0, "%s", "fileName");
    if (!speakerMap)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2754, 0, "%s", "speakerMap");
    entries = &speakerMap->channelMaps[0][1];
    if (!Com_ParseChannelMapEntry(fileName, &speakerMap->channelMaps[0][1], SA_MONOSOURCE, SA_LEFTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_MONOSOURCE, SA_RIGHTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_MONOSOURCE, SA_CENTERSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_MONOSOURCE, SA_LFESPEAKER))
        return 0;
    if (Com_ParseChannelMapEntry(fileName, entries, SA_MONOSOURCE, SA_LEFTSURROUNDSPEAKER))
        return Com_ParseChannelMapEntry(fileName, entries, SA_MONOSOURCE, SA_RIGHTSURROUNDSPEAKER) != 0;
    return 0;
}

bool __cdecl Com_ParseStereoSource51Output(const char *fileName, SpeakerMap *speakerMap)
{
    MSSChannelMap *entries; // [esp+0h] [ebp-4h]

    if (!g_spkrMapParseBuffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2783, 0, "%s", "g_spkrMapParseBuffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2784, 0, "%s", "fileName");
    if (!speakerMap)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2785, 0, "%s", "speakerMap");
    entries = &speakerMap->channelMaps[1][1];
    if (!Com_ParseChannelMapEntry(fileName, &speakerMap->channelMaps[1][1], SA_LEFTSOURCE, SA_LEFTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_LEFTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_RIGHTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_RIGHTSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_CENTERSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_CENTERSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_LFESPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_LFESPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_LEFTSURROUNDSPEAKER))
        return 0;
    if (!Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_LEFTSURROUNDSPEAKER))
        return 0;
    if (Com_ParseChannelMapEntry(fileName, entries, SA_LEFTSOURCE, SA_RIGHTSURROUNDSPEAKER))
        return Com_ParseChannelMapEntry(fileName, entries, SA_RIGHTSOURCE, SA_RIGHTSURROUNDSPEAKER) != 0;
    return 0;
}

char __cdecl Com_ParseSpkrMapFile(const char *buffer, const char *fileName, SpeakerMap *speakerMap)
{
    if (!buffer)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2824, 0, "%s", "buffer");
    if (!fileName)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2825, 0, "%s", "fileName");
    if (!speakerMap)
        MyAssertHandler(".\\universal\\com_sndalias_load_obj.cpp", 2826, 0, "%s", "speakerMap");
    g_spkrMapParseBuffer = buffer;
    Com_BeginParseSession(fileName);
    if (Com_ParseMonoSourceStereoOutput(fileName, speakerMap)
        && Com_ParseStereoSourceStereoOutput(fileName, speakerMap)
        && Com_ParseMonoSource51Output(fileName, speakerMap)
        && Com_ParseStereoSource51Output(fileName, speakerMap))
    {
        Com_EndParseSession();
        g_spkrMapParseBuffer = 0;
        return 1;
    }
    else
    {
        Com_EndParseSession();
        g_spkrMapParseBuffer = 0;
        return 0;
    }
}

char __cdecl Com_LoadSpkrMapFile(char *name, SpeakerMapInfo *info)
{
    char dest[64]; // [esp+0h] [ebp-2048h] BYREF
    char buffer[8196]; // [esp+40h] [ebp-2008h] BYREF

    Com_PreLoadSpkrMapFile(info);
    Com_sprintf(dest, 0x40u, "soundaliases/%s.spkrmap", name);
    if (Com_LoadSpkrMapParseBuffer(dest, buffer) && Com_ParseSpkrMapFile(buffer, dest, &info->speakerMap))
    {
        Com_sprintf(info->name, 0x40u, name);
        return 1;
    }
    else
    {
        Com_InitDefaultSoundAliasSpeakerMap(info);
        return 0;
    }
}

void __cdecl Com_InitSoundAlias()
{
    saLoadObjGlob.tempAliases = 0;
    saLoadObjGlob.tempAliasCount = 0;
}