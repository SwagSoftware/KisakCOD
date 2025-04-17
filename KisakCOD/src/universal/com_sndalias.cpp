#include "com_sndalias.h"
#include "com_memory.h"
#include <devgui/devgui.h>
#include <sound/snd_local.h>
#include "com_files.h"
#include <qcommon/cmd.h>
#include <database/database.h>

SoundAliasGlobals g_sa;
const char ***varXStringPtr;


double __cdecl Com_GetVolumeFalloffCurveValue(SndCurve *volumeFalloffCurve, float fraction)
{
    return GraphGetValueFromFraction(volumeFalloffCurve->knotCount, volumeFalloffCurve->knots, fraction);
}

void __cdecl Com_InitSoundDevGuiGraphs()
{
    if (useFastFile->current.enabled)
        ((void(__cdecl *)(void (*)()))Com_InitSoundDevGuiGraphs_FastFile)(Com_InitSoundDevGuiGraphs_FastFile);
    else
        ((void(__cdecl *)(void (*)()))Com_InitSoundDevGuiGraphs_LoadObj)(Com_InitSoundDevGuiGraphs_LoadObj);
}

void __cdecl Com_VolumeFalloffCurveGraphEventCallback(const DevGraph *graph, DevEventType event)
{
    char string[8196]; // [esp+14h] [ebp-2030h] BYREF
    int data; // [esp+2018h] [ebp-2Ch]
    char dest[32]; // [esp+201Ch] [ebp-28h] BYREF
    int i; // [esp+2040h] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 213, 0, "%s", "graph");
    data = (int)graph->data;
    if (data <= 0 || data >= 16)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            215,
            0,
            "%s\n\t(curveIndex) = %i",
            "(curveIndex > 0 && curveIndex < 16)",
            data);
    if (event == EVENT_ACCEPT)
    {
        sprintf(string, "Volume Falloff Curve #%02d\nKnot Count: %d\n", data, *graph->knotCount);
        for (i = 0; i < *graph->knotCount; ++i)
        {
            Com_sprintf(dest, 0x20u, "%.4f %.4f\n", graph->knots[i][0], graph->knots[i][1]);
            I_strncat(string, 0x2000, dest);
        }
        Com_Printf(9, "^6%s", string);
    }
}

void Com_InitSoundDevGuiGraphs_FastFile()
{
    int counter; // [esp+0h] [ebp-4h] BYREF

    counter = 0;
    DB_EnumXAssets(ASSET_TYPE_SOUND_CURVE, (void(__cdecl *)(XAssetHeader, void *))Com_GetGraphList, &counter, 0);
}

void __cdecl Com_VolumeFalloffCurveGraphEventCallback(const DevGraph *graph, DevEventType event, int i)
{
    char string[8196]; // [esp+14h] [ebp-2030h] BYREF
    int data; // [esp+2018h] [ebp-2Ch]
    char dest[32]; // [esp+201Ch] [ebp-28h] BYREF
    int i; // [esp+2040h] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 213, 0, "%s", "graph");
    data = (int)graph->data;
    if (data <= 0 || data >= 16)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            215,
            0,
            "%s\n\t(curveIndex) = %i",
            "(curveIndex > 0 && curveIndex < 16)",
            data);
    if (event == EVENT_ACCEPT)
    {
        sprintf(string, "Volume Falloff Curve #%02d\nKnot Count: %d\n", data, *graph->knotCount);
        for (i = 0; i < *graph->knotCount; ++i)
        {
            Com_sprintf(dest, 0x20u, "%.4f %.4f\n", graph->knots[i][0], graph->knots[i][1]);
            I_strncat(string, 0x2000, dest);
        }
        Com_Printf(9, "^6%s", string);
    }
}

void __cdecl Com_GetGraphList(XAssetHeader header, int *data)
{
    char devguiPath[256]; // [esp+0h] [ebp-110h] BYREF
    DevGraph *graph; // [esp+104h] [ebp-Ch]
    int index; // [esp+108h] [ebp-8h]
    int *count; // [esp+10Ch] [ebp-4h]

    count = data;
    index = *data;
    if (index < 16)
    {
        graph = &g_sa.curveDevGraphs[index];
        if (*header.xmodelPieces->name)
        {
            sprintf(devguiPath, "Main:1/Snd:6/Volume Falloff Curves/%s:%d", header.xmodelPieces->name, index);
            graph->knotCountMax = 8;
            graph->knots = (float (*)[2]) & header.xmodelPieces->pieces;
            graph->knotCount = &header.xmodelPieces->numpieces;
            graph->eventCallback = Com_VolumeFalloffCurveGraphEventCallback;
            graph->data = (void *)index;
            graph->disableEditingEndPoints = 1;
            DevGui_AddGraph(devguiPath, graph);
            ++*count;
        }
    }
}

MSSChannelMap *__cdecl Com_GetSpeakerMap(SpeakerMap *speakerMap, int sourceChannelCount)
{
    if (sourceChannelCount != 1 && sourceChannelCount != 2)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            410,
            0,
            "%s\n\t(sourceChannelCount) = %i",
            "(sourceChannelCount == 1 || sourceChannelCount == 2)",
            sourceChannelCount);
    return &speakerMap->channelMaps[sourceChannelCount == 2][SND_IsMultiChannel()];
}

snd_alias_list_t *__cdecl Com_TryFindSoundAlias(const char *name)
{
    if (useFastFile->current.enabled)
        return Com_TryFindSoundAlias_FastFile(name);
    else
        return Com_TryFindSoundAlias_LoadObj(name);
}

unsigned int __cdecl Com_HashAliasName(const char *name)
{
    unsigned int hash; // [esp+4h] [ebp-4h]

    hash = 0;
    while (*name)
        hash = tolower(*name++) ^ (33 * hash);
    return hash % g_sa.hashSize;
}

snd_alias_list_t *__cdecl Com_TryFindSoundAlias_LoadObj(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 568, 0, "%s", "name");
    for (hashIndex = Com_HashAliasName(name); g_sa.hash[hashIndex]; hashIndex = (hashIndex + 1) % g_sa.hashSize)
    {
        if (!I_stricmp(name, g_sa.hash[hashIndex]->aliasName))
            return g_sa.hash[hashIndex];
    }
    return 0;
}

snd_alias_list_t *__cdecl Com_FindSoundAlias_LoadObj(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 586, 0, "%s", "name");
    for (hashIndex = Com_HashAliasName(name); g_sa.hash[hashIndex]; hashIndex = (hashIndex + 1) % g_sa.hashSize)
    {
        if (!I_stricmp(name, g_sa.hash[hashIndex]->aliasName))
            return g_sa.hash[hashIndex];
    }
    Com_PrintError(10, "Missing soundalias \"%s\".\n", name);
    return 0;
}

snd_alias_list_t *__cdecl Com_TryFindSoundAlias_FastFile(const char *name)
{
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 609, 0, "%s", "name");
    aliasList = DB_FindXAssetHeader(ASSET_TYPE_SOUND, name).sound;
    if (DB_IsXAssetDefault(ASSET_TYPE_SOUND, name))
        return 0;
    else
        return aliasList;
}

snd_alias_list_t *__cdecl Com_FindSoundAlias(const char *name)
{
    if (useFastFile->current.enabled)
        return Com_FindSoundAlias_FastFile(name);
    else
        return Com_FindSoundAlias_LoadObj(name);
}

snd_alias_list_t *__cdecl Com_FindSoundAliasNoErrors_LoadObj(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 655, 0, "%s", "name");
    for (hashIndex = Com_HashAliasName(name); g_sa.hash[hashIndex]; hashIndex = (hashIndex + 1) % g_sa.hashSize)
    {
        if (!I_stricmp(name, g_sa.hash[hashIndex]->aliasName))
            return g_sa.hash[hashIndex];
    }
    return 0;
}

snd_alias_list_t *__cdecl Com_FindSoundAlias_FastFile(const char *name)
{
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 623, 0, "%s", "name");
    aliasList = DB_FindXAssetHeader(ASSET_TYPE_SOUND, name).sound;
    if (!DB_IsXAssetDefault(ASSET_TYPE_SOUND, name))
        return aliasList;
    Com_PrintError(10, "Missing soundalias \"%s\".\n", name);
    return 0;
}

snd_alias_list_t *__cdecl Com_FindSoundAliasNoErrors(const char *name)
{
    if (useFastFile->current.enabled)
        return Com_FindSoundAliasNoErrors_FastFile(name);
    else
        return Com_FindSoundAliasNoErrors_LoadObj(name);
}

snd_alias_list_t *__cdecl Com_FindSoundAliasNoErrors_FastFile(const char *name)
{
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 675, 0, "%s", "name");
    aliasList = DB_FindXAssetHeader(ASSET_TYPE_SOUND, name).sound;
    return !DB_IsXAssetDefault(ASSET_TYPE_SOUND, name) ? aliasList : 0;
}


void __cdecl Load_SndAliasCustom(snd_alias_list_t **var)
{
    if (*var)
    {
        varXStringPtr = (const char ***)var;
        Load_XStringPtr(0);
        if (!*varXStringPtr)
            MyAssertHandler(".\\universal\\com_sndalias.cpp", 696, 0, "%s", "*varXStringPtr");
        *(XAssetHeader *)var = DB_FindXAssetHeader(ASSET_TYPE_SOUND, **varXStringPtr);
    }
}

int __cdecl SND_GetAliasOffset(const snd_alias_t *alias)
{
    snd_alias_t *checkAlias; // [esp+0h] [ebp-Ch]
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-8h]
    int index; // [esp+8h] [ebp-4h]

    if (!alias->aliasName)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 708, 0, "%s", "alias->aliasName");
    aliasList = Com_FindSoundAlias(alias->aliasName);
    checkAlias = aliasList->head;
    for (index = 0; index < aliasList->count; ++index)
    {
        if (checkAlias == alias)
            return index;
        ++checkAlias;
    }
    return 0;
}

snd_alias_t *__cdecl SND_GetAliasWithOffset(const char *name, int offset)
{
    snd_alias_t *checkAlias; // [esp+0h] [ebp-Ch]
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-8h]
    int index; // [esp+8h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 733, 0, "%s", "name");
    aliasList = Com_FindSoundAlias(name);
    if (!aliasList)
        goto LABEL_11;
    checkAlias = aliasList->head;
    for (index = 0; index < aliasList->count; ++index)
    {
        if (index == offset)
            return checkAlias;
        ++checkAlias;
    }
    if (aliasList->count)
        return aliasList->head;
LABEL_11:
    Com_Error(ERR_DROP, "SND_GetAliasWithOffset: could not find sound alias '%s' with offset %d", name, offset);
    return 0;
}

void __cdecl StreamFileNameGetName(const StreamFileName *streamFileName, char *filename, unsigned int size)
{
    Com_sprintf(filename, size, "%s\\%s", streamFileName->info.raw.dir, streamFileName->info.raw.name);
}

void __cdecl Com_GetSoundFileName(const snd_alias_t *alias, char *filename, int size)
{
    SoundFile *soundFile; // [esp+0h] [ebp-4h]

    if (!alias)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 801, 0, "%s", "alias");
    if (!alias->soundFile)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 802, 0, "%s", "alias->soundFile");
    soundFile = alias->soundFile;
    if (soundFile->type == 1)
        I_strncpyz(filename, (char *)soundFile->u.loadSnd->name, size);
    else
        StreamFileNameGetName((const StreamFileName *)&soundFile->u, filename, size);
}

snd_alias_t *__cdecl Com_PickSoundAliasFromList(snd_alias_list_t *aliasList)
{
    int maxSequence; // [esp+8h] [ebp-18h]
    snd_alias_t *bestAlias; // [esp+Ch] [ebp-14h]
    float cumulativeProbability; // [esp+10h] [ebp-10h]
    float cumulativeProbabilitya; // [esp+10h] [ebp-10h]
    int index; // [esp+14h] [ebp-Ch]
    int indexa; // [esp+14h] [ebp-Ch]
    snd_alias_t *alias; // [esp+18h] [ebp-8h]
    snd_alias_t *aliasa; // [esp+18h] [ebp-8h]
    snd_alias_t *firstAlias; // [esp+1Ch] [ebp-4h]

    if (!aliasList)
        return 0;
    if (aliasList->count)
    {
        firstAlias = aliasList->head;
        bestAlias = firstAlias;
        cumulativeProbability = firstAlias->probability;
        maxSequence = firstAlias->sequence;
        alias = firstAlias;
        index = 0;
        while (++index != aliasList->count)
        {
            ++alias;
            cumulativeProbability = cumulativeProbability + alias->probability;
            g_sa.randSeed = 214013 * g_sa.randSeed + 2531011;
            if (alias->probability * 32768.0 > (double)((g_sa.randSeed >> 16) & 0x7FFF) * cumulativeProbability)
                bestAlias = alias;
            if (maxSequence < alias->sequence)
                maxSequence = alias->sequence;
        }
        if (aliasList->count > 2 && maxSequence == bestAlias->sequence)
        {
            cumulativeProbabilitya = 0.0;
            aliasa = firstAlias;
            for (indexa = 0; indexa < aliasList->count; ++indexa)
            {
                if (maxSequence != aliasa->sequence)
                {
                    cumulativeProbabilitya = cumulativeProbabilitya + aliasa->probability;
                    g_sa.randSeed = 214013 * g_sa.randSeed + 2531011;
                    if (aliasa->probability * 32768.0 > (double)((g_sa.randSeed >> 16) & 0x7FFF) * cumulativeProbabilitya)
                        bestAlias = aliasa;
                }
                ++aliasa;
            }
        }
        bestAlias->sequence = maxSequence + 1;
        return bestAlias;
    }
    else
    {
        Com_PrintWarning(9, "Sound not loaded: \"%s\"\n", aliasList->aliasName);
        return 0;
    }
}

snd_alias_t *__cdecl Com_PickSoundAlias(const char *aliasname)
{
    snd_alias_list_t *aliasList; // [esp+0h] [ebp-4h]

    aliasList = Com_FindSoundAlias(aliasname);
    return Com_PickSoundAliasFromList(aliasList);
}

bool __cdecl Com_AliasNameRefersToSingleAlias(const char *aliasname)
{
    snd_alias_list_t *aliasList; // [esp+4h] [ebp-4h]

    aliasList = Com_FindSoundAlias(aliasname);
    return aliasList && aliasList->count == 1;
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
                                (snd_alias_members_t)ptr[i + 1],
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
                                &String);
                        break;
                    }
                    if (!isFieldSet[16])
                        alias.keep = Com_SoundAliasLoadSpec(loadspec, loadspecCurGame, &String, sourceFile);
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

void __cdecl Com_LoadSoundAliases(const char *loadspec, const char *loadspecCurGame, snd_alias_system_t system)
{
    char *v3; // ecx
    const char *v4; // eax
    char v5; // [esp+3h] [ebp-69h]
    char *v6; // [esp+8h] [ebp-64h]
    const char *v7; // [esp+Ch] [ebp-60h]
    int mark; // [esp+10h] [ebp-5Ch]
    char trimspec[68]; // [esp+14h] [ebp-58h] BYREF
    int numMissing; // [esp+5Ch] [ebp-10h]
    const char **fileNames; // [esp+60h] [ebp-Ch]
    int i; // [esp+64h] [ebp-8h]
    int fileCount; // [esp+68h] [ebp-4h] BYREF

    if (!loadspec)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 907, 0, "%s", "loadspec");
    if (!*loadspec)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 908, 0, "%s", "loadspec[0]");
    if ((unsigned int)system > SASYS_GAME)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            913,
            0,
            "%s\n\t(system) = %i",
            "(system >= 0 && system < SASYS_COUNT)",
            system);
    if (system == SASYS_GAME && g_sa.initialized[1])
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            914,
            0,
            "%s",
            "system != SASYS_GAME || !g_sa.initialized[SASYS_CGAME]");
    if (g_sa.initialized[system])
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            916,
            0,
            "%s\n\t(system) = %i",
            "(!g_sa.initialized[system])",
            system);
    if (g_sa.aliasInfo[system].count)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            917,
            0,
            "%s\n\t(g_sa.aliasInfo[system].count) = %i",
            "(g_sa.aliasInfo[system].count == 0)",
            g_sa.aliasInfo[system].count);
    if (*(&g_sa.soundFileInfo[-4].count + 3 * system))
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 918, 0, "%s", "g_sa.aliasInfo[system].head == NULL");
    if (!g_sa.curvesInitialized)
        Com_InitCurves();
    if (!g_sa.speakerMapsInitialized)
        Com_InitSpeakerMaps();
    if ((unsigned int)system > SASYS_GAME)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            930,
            0,
            "%s\n\t(system) = %i",
            "(system >= 0 && system < SASYS_COUNT)",
            system);
    if (I_strnicmp(loadspec, "maps/mp/", 8))
    {
        if (I_strnicmp(loadspec, "maps/", 5))
        {
            v7 = loadspec;
            v6 = trimspec;
            do
            {
                v5 = *v7;
                *v6++ = *v7++;
            } while (v5);
        }
        else
        {
            Com_StripExtension((char *)loadspec + 5, trimspec);
        }
    }
    else
    {
        Com_StripExtension((char *)loadspec + 8, trimspec);
    }
    I_strlwr(trimspec);
    if (system == SASYS_CGAME && com_sv_running->current.enabled)
    {
        g_sa.aliasInfo[1] = g_sa.aliasInfo[2];
        g_sa.soundFileInfo[1] = g_sa.soundFileInfo[2];
    }
    else
    {
        ProfLoad_Begin("Find sound alias files");
        fileNames = FS_ListFiles("soundaliases", "csv", FS_LIST_PURE_ONLY, &fileCount);
        ProfLoad_End();
        if (!fileCount)
        {
            Com_PrintWarning(9, "WARNING: can't find any sound alias files (soundaliases/*.csv)\n");
            return;
        }
        ProfLoad_Begin("Load sound alias files");
        mark = Hunk_HideTempMemory();
        Com_InitSoundAlias();
        Com_InitEntChannels(v3);
        for (i = 0; i < fileCount; ++i)
            Com_LoadSoundAliasFile(trimspec, loadspecCurGame, fileNames[i]);
        ProfLoad_End();
        ProfLoad_Begin("Finish sound aliases");
        Com_MakeSoundAliasesPermanent(&g_sa.aliasInfo[system], (SoundFileInfo *)(8 * system + 230749208));
        ProfLoad_End();
        Hunk_ClearTempMemory();
        Hunk_ShowTempMemory(mark);
        FS_FreeFileList(fileNames);
    }
    if ((unsigned int)system <= SASYS_CGAME && !g_sa.initialized[1] && !g_sa.initialized[0])
        Cmd_AddCommandInternal("snd_list", Com_SoundList_f, &Com_SoundList_f_VAR);
    g_sa.initialized[system] = 1;
    if ((unsigned int)system <= SASYS_CGAME)
    {
        numMissing = Com_LoadSoundAliasSounds((SoundFileInfo *)(8 * system + 230749208));
        if (numMissing)
        {
            if (snd_errorOnMissing->current.enabled)
            {
                v4 = va(&byte_8C0EE8, numMissing);
                Com_Error((errorParm_t)(system != SASYS_UI), v4);
            }
        }
    }
}

void __cdecl Com_UnloadSoundAliasSounds(snd_alias_system_t system)
{
    int j; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    snd_alias_t *aliases; // [esp+Ch] [ebp-4h]

    if ((unsigned int)system > SASYS_CGAME)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            828,
            0,
            "%s\n\t(system) = %i",
            "(system == SASYS_UI || system == SASYS_CGAME)",
            system);
    SND_StopSounds(SND_STOP_ALL);
    aliases = (snd_alias_t *)*(&g_sa.soundFileInfo[-4].count + 3 * system);
    for (i = 0; i < g_sa.aliasInfo[system].count; ++i)
    {
        if ((aliases[i].flags & 0xC0) >> 6 == 1)
        {
            for (j = 0;
                j < i
                && (aliases[j].soundFile != aliases[i].soundFile
                    || (aliases[j].flags & 0xC0) >> 6 != (aliases[i].flags & 0xC0) >> 6);
                    ++j)
            {
                ;
            }
            aliases[i].soundFile->exists = 0;
        }
    }
}

void __cdecl Com_UnloadSoundAliases(snd_alias_system_t system)
{
    if ((unsigned int)system > SASYS_GAME)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            1019,
            0,
            "%s\n\t(system) = %i",
            "(system >= 0 && system < SASYS_COUNT)",
            system);
    if (g_sa.initialized[system])
    {
        if (system != SASYS_GAME)
            Com_UnloadSoundAliasSounds(system);
        if (system == SASYS_UI && g_sa.initialized[1])
            MyAssertHandler(
                ".\\universal\\com_sndalias.cpp",
                1032,
                0,
                "%s",
                "system != SASYS_UI || !g_sa.initialized[SASYS_CGAME]");
        if (system == SASYS_GAME && g_sa.initialized[1])
            MyAssertHandler(
                ".\\universal\\com_sndalias.cpp",
                1033,
                0,
                "%s",
                "system != SASYS_GAME || !g_sa.initialized[SASYS_CGAME]");
        if (*(&g_sa.soundFileInfo[-4].count + 3 * system))
        {
            *(&g_sa.soundFileInfo[-4].count + 3 * system) = 0;
            g_sa.aliasInfo[system].count = 0;
            memset((unsigned __int8 *)g_sa.hash, 0, 4 * g_sa.hashSize);
            g_sa.hashUsed = 0;
        }
        else if (g_sa.aliasInfo[system].count)
        {
            MyAssertHandler(
                ".\\universal\\com_sndalias.cpp",
                1049,
                0,
                "%s\n\t(g_sa.aliasInfo[system].count) = %i",
                "(g_sa.aliasInfo[system].count == 0)",
                g_sa.aliasInfo[system].count);
        }
        g_sa.initialized[system] = 0;
        if ((unsigned int)system <= SASYS_CGAME && !g_sa.initialized[1] && !g_sa.initialized[0])
            Cmd_RemoveCommand("snd_list");
    }
}