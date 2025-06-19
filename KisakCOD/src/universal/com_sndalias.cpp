#include "com_sndalias.h"
#include "com_memory.h"
#include <devgui/devgui.h>
#include <sound/snd_local.h>
#include "com_files.h"
#include <qcommon/cmd.h>
#include <database/database.h>
#include "q_parse.h"

SoundAliasGlobals g_sa;

char __cdecl Com_LoadVolumeFalloffCurve(const char *name, SndCurve *curve)
{
    char dest[68]; // [esp+10h] [ebp-2058h] BYREF
    signed int v4; // [esp+54h] [ebp-2014h]
    unsigned char buffer[8192]; // [esp+58h] [ebp-2010h] BYREF
    const char *last; // [esp+205Ch] [ebp-Ch]
    int file; // [esp+2060h] [ebp-8h] BYREF
    int len; // [esp+2064h] [ebp-4h]

    last = "SNDCURVE";
    len = strlen("SNDCURVE");
    Com_sprintf(dest, 0x40u, "soundaliases/%s.vfcurve", name);
    v4 = FS_FOpenFileRead(dest, &file);
    if (v4 >= 0)
    {
        if (v4)
        {
            FS_Read(buffer, len, file);
            buffer[len] = 0;
            if (!strncmp((const char*)buffer, last, len))
            {
                if (v4 - len < 0x2000)
                {
                    memset(buffer, 0, sizeof(buffer));
                    FS_Read(buffer, v4 - len, file);
                    buffer[v4 - len] = 0;
                    FS_FCloseFile(file);
                    if (Com_ParseSndCurveFile((const char*)buffer, dest, curve))
                    {
                        curve->filename = name;
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    FS_FCloseFile(file);
                    Com_PrintError(9, "ERROR: \"%s\" Is too long of a sndcurve file to parse\n", dest);
                    return 0;
                }
            }
            else
            {
                FS_FCloseFile(file);
                Com_PrintError(9, "ERROR: \"%s\" does not appear to be a sndcurve file\n", dest);
                return 0;
            }
        }
        else
        {
            FS_FCloseFile(file);
            Com_PrintError(9, "ERROR: sndcurve file '%s' is empty\n", dest);
            return 0;
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: Could not load sndcurve file '%s'\n", dest);
        return 0;
    }
}

void Com_InitCurves()
{
    int fileIndex; // [esp+10h] [ebp-10h]
    const char **fileNames; // [esp+14h] [ebp-Ch]
    char *name; // [esp+18h] [ebp-8h]
    int fileCount; // [esp+1Ch] [ebp-4h] BYREF

    if (g_sa.curvesInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 867, 0, "%s", "!g_sa.curvesInitialized");
    memset(g_sa.volumeFalloffCurves, 0, sizeof(g_sa.volumeFalloffCurves));
    Com_InitDefaultSoundAliasVolumeFalloffCurve(g_sa.volumeFalloffCurves);
    fileNames = FS_ListFiles("soundaliases", "vfcurve", FS_LIST_PURE_ONLY, &fileCount);
    if (fileCount > 15)
        Com_Error(ERR_DROP, "Snd_Alias Curve initialization: .vfcurve file count (%d) exceeds maximum (%d)", fileCount, 15);
    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
    {
        name = g_sa.volumeFalloffCurveNames[fileIndex + 1];
        I_strncpyz(name, fileNames[fileIndex], strlen(fileNames[fileIndex]) - 7);
        if (!Com_LoadVolumeFalloffCurve(name, &g_sa.volumeFalloffCurves[fileIndex + 1]))
            Com_Error(ERR_FATAL, "Failed to load sndcurve file %s", fileNames[fileIndex]);
    }
    FS_FreeFileList(fileNames);
    g_sa.curvesInitialized = 1;
}

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

void __cdecl Com_VolumeFalloffCurveGraphEventCallback(const DevGraph *graph, DevEventType event, int i)
{
    char string[8196]; // [esp+14h] [ebp-2030h] BYREF
    int data; // [esp+2018h] [ebp-2Ch]
    char dest[32]; // [esp+201Ch] [ebp-28h] BYREF
    //int i; // [esp+2040h] [ebp-4h]

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
        if (header.xmodelPieces->name)
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

snd_alias_list_t *__cdecl Com_TryFindSoundAlias(const char *name)
{
    if (useFastFile->current.enabled)
        return Com_TryFindSoundAlias_FastFile(name);
    else
        return Com_TryFindSoundAlias_LoadObj(name);
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

// KISAK: Why is this in com_sndalias when every single other load function is in db_load.cpp? no clue!
extern char const ***varXStringPtr;
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

void __cdecl Com_StreamedSoundList(snd_alias_system_t system)
{
    int j; // [esp+4h] [ebp-94h]
    char filename[132]; // [esp+8h] [ebp-90h] BYREF
    int i; // [esp+90h] [ebp-8h]
    snd_alias_t *aliases; // [esp+94h] [ebp-4h]

    if (g_sa.initialized[system])
    {
        aliases = *(snd_alias_t **)(&g_sa.soundFileInfo[-4].count + 3 * system);
        for (i = 0; i < g_sa.aliasInfo[system].count; ++i)
        {
            if ((aliases[i].flags & 0xC0) >> 6 == 2)
            {
                for (j = 0; j < i; ++j)
                {
                    if ((aliases[j].flags & 0xC0) >> 6 == 2 && aliases[j].soundFile == aliases[i].soundFile)
                        goto LABEL_3;
                }
                Com_GetSoundFileName(&aliases[i], filename, 128);
                if (aliases[i].soundFile->exists)
                    Com_Printf(9, "%-64s\n", filename);
                else
                    Com_Printf(9, "%-64s FILE NOT FOUND\n", filename);
            }
        LABEL_3:
            ;
        }
    }
}

void __cdecl Com_LoadedSoundList(snd_alias_system_t system)
{
    int j; // [esp+Ch] [ebp-9Ch]
    char filename[132]; // [esp+10h] [ebp-98h] BYREF
    int i; // [esp+98h] [ebp-10h]
    snd_alias_t *aliases; // [esp+9Ch] [ebp-Ch]
    int totalMem; // [esp+A0h] [ebp-8h]
    int fileMem; // [esp+A4h] [ebp-4h]

    if (g_sa.initialized[system])
    {
        totalMem = 0;
        aliases = *(snd_alias_t **)(&g_sa.soundFileInfo[-4].count + 3 * system);
        for (i = 0; i < g_sa.aliasInfo[system].count; ++i)
        {
            if ((aliases[i].flags & 0xC0) >> 6 == 1)
            {
                for (j = 0; j < i; ++j)
                {
                    if ((aliases[j].flags & 0xC0) >> 6 == 1 && aliases[j].soundFile == aliases[i].soundFile)
                        goto LABEL_3;
                }
                Com_GetSoundFileName(&aliases[i], filename, 128);
                if (aliases[i].soundFile->exists)
                {
                    fileMem = SND_GetSoundFileSize((unsigned int*)&aliases[i].soundFile->u.loadSnd->sound.info.format);
                    totalMem += fileMem;
                    Com_Printf(9, "%-64s %7.1f KB\n", filename, fileMem * 0.0009765625);
                }
                else
                {
                    Com_Printf(9, "%-64s FAILED TO LOAD\n", filename);
                }
            }
        LABEL_3:
            ;
        }
        Com_Printf(9, "\ntotal usage %7.3f MB\n", totalMem * 0.00000095367431640625);
    }
}

char __cdecl Com_AddAliasList(const char *name, snd_alias_list_t *aliasList)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    for (hashIndex = Com_HashAliasName(name); g_sa.hash[hashIndex]; hashIndex = (hashIndex + 1) % g_sa.hashSize)
    {
        if (!I_stricmp(name, g_sa.hash[hashIndex]->aliasName))
            return 0;
    }
    if (++g_sa.hashUsed >= g_sa.hashSize)
        MyAssertHandler(
            ".\\universal\\com_sndalias.cpp",
            788,
            0,
            "g_sa.hashUsed < g_sa.hashSize\n\t%i, %i",
            g_sa.hashUsed,
            g_sa.hashSize);
    g_sa.hash[hashIndex] = aliasList;
    return 1;
}

void __cdecl Com_SoundList_f()
{
    Com_Printf(0, "\n________________________________________\ncurrently streamed menu sounds:\n");
    Com_StreamedSoundList(SASYS_UI);
    Com_Printf(0, "\n________________________________________\ncurrently streamed in-game sounds:\n");
    Com_StreamedSoundList(SASYS_CGAME);
    Com_Printf(0, "________________________________________\ncurrently loaded menu sounds:\n");
    Com_LoadedSoundList(SASYS_UI);
    Com_Printf(0, "\n________________________________________\ncurrently loaded in-game sounds:\n");
    Com_LoadedSoundList(SASYS_CGAME);
    Com_Printf(0, "\n");
}

cmd_function_s Com_SoundList_f_VAR;

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
        Com_InitEntChannels();
        for (i = 0; i < fileCount; ++i)
            Com_LoadSoundAliasFile(trimspec, loadspecCurGame, fileNames[i]);
        ProfLoad_End();
        ProfLoad_Begin("Finish sound aliases");
        Com_MakeSoundAliasesPermanent(&g_sa.aliasInfo[system], &g_sa.soundFileInfo[system]);
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
        numMissing = Com_LoadSoundAliasSounds(&g_sa.soundFileInfo[system]);
        if (numMissing)
        {
#ifndef DEDICATED
            if (snd_errorOnMissing->current.enabled)
            {
                v4 = va("%i sound file(s) are missing or in a bad format", numMissing);
                Com_Error((errorParm_t)(system != SASYS_UI), v4);
            }
#endif
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

void Com_InitEntChannels()
{
    void *file; // [esp+0h] [ebp-4h] BYREF

    //file = file_1;
    if (FS_ReadFile("soundaliases/channels.def", (void **)&file) < 0)
        Com_Error(ERR_DROP, "unable to load entity channel file [%s].\n", "soundaliases/channels.def");
    Com_ParseEntChannelFile((const char*)file);
}

void __cdecl Com_InitDefaultSoundAliasVolumeFalloffCurve(SndCurve *sndCurve)
{
    sndCurve->filename = "";
    sndCurve->knots[0][0] = 0.0;
    sndCurve->knots[0][1] = 1.0;
    sndCurve->knots[1][0] = 1.0;
    sndCurve->knots[1][1] = 0.0;
    sndCurve->knotCount = 2;
}

void __cdecl Com_InitDefaultSoundAliasSpeakerMap(SpeakerMapInfo *info)
{
    Com_PreLoadSpkrMapFile(info);
    info->speakerMap.isDefault = 1;
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[0], 0, 0, 0.5);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[0], 0, 1u, 0.5);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 0, 0, 1.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 1u, 0, 0.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 0, 1u, 0.0);
    Com_SetChannelMapEntry(info->speakerMap.channelMaps[1], 1u, 1u, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 0, 0.5);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 1u, 0.5);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[0][1], 0, 5u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 0, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 0, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 1u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 1u, 1.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 2u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 3u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 4u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 0, 5u, 0.0);
    Com_SetChannelMapEntry(&info->speakerMap.channelMaps[1][1], 1u, 5u, 0.0);
}

int __cdecl Com_StringEdReferenceExists(const char *pszReference)
{
    char *file; // [esp+14h] [ebp-10h] BYREF
    const char *ptr; // [esp+18h] [ebp-Ch] BYREF
    const char *token; // [esp+1Ch] [ebp-8h]
    int bReferenceFound; // [esp+20h] [ebp-4h]

    bReferenceFound = 0;
    if (I_strncmp(pszReference, "SUBTITLE_", 9))
        return 0;
    if (FS_ReadFile("soundaliases/subtitle.st", (void**)&file) >= 0)
    {
        Com_BeginParseSession("soundaliases/subtitle.st");
        for (ptr = file; ; Com_SkipRestOfLine(&ptr))
        {
            token = Com_Parse(&ptr)->token;
            if (!ptr)
                break;
            if (!strcmp(token, "REFERENCE"))
            {
                token = Com_ParseOnLine(&ptr)->token;
                if (!I_stricmp(pszReference + 9, token))
                {
                    bReferenceFound = 1;
                    break;
                }
            }
        }
        Com_EndParseSession();
        FS_FreeFile(file);
        return bReferenceFound;
    }
    else
    {
        Com_PrintWarning(9, "WARNING: Could not read local copy of StringEd file %s\n", "soundaliases/subtitle.st");
        return 0;
    }
}

char szReference[1024];
char *__cdecl Com_GetSubtitleStringEdReference(const char *subtitle)
{
    char v2; // [esp+17h] [ebp-2Dh]
    char *v3; // [esp+1Ch] [ebp-28h]
    const char *v4; // [esp+20h] [ebp-24h]
    char *file; // [esp+38h] [ebp-Ch] BYREF
    const char *ptr; // [esp+3Ch] [ebp-8h] BYREF
    const char *token; // [esp+40h] [ebp-4h]

    if (FS_ReadFile("soundaliases/subtitle.st", (void**)&file) >= 0)
    {
        Com_BeginParseSession("soundaliases/subtitle.st");
        for (ptr = file; ; Com_SkipRestOfLine(&ptr))
        {
            token = Com_Parse(&ptr)->token;
            if (!ptr)
                break;
            if (!strcmp(token, "REFERENCE"))
            {
                token = Com_ParseOnLine(&ptr)->token;
                v4 = token;
                v3 = szReference;
                do
                {
                    v2 = *v4;
                    *v3++ = *v4++;
                } while (v2);
                Com_SkipRestOfLine(&ptr);
                do
                {
                    token = Com_Parse(&ptr)->token;
                    if (!ptr)
                        Com_Error(ERR_DROP, "StringEd file %s has bad syntax", "soundaliases/subtitle.st");
                } while (strcmp(token, "LANG_ENGLISH"));
                token = Com_ParseOnLine(&ptr)->token;
                if (!I_stricmp(subtitle, token))
                {
                    Com_EndParseSession();
                    FS_FreeFile(file);
                    return szReference;
                }
            }
        }
        Com_EndParseSession();
        FS_FreeFile(file);
        return 0;
    }
    else
    {
        Com_PrintWarning(9, "WARNING: Could not read local copy of StringEd file %s\n", "soundaliases/subtitle.st");
        return 0;
    }
}

void __cdecl Com_WriteStringEdReferenceToFile(char *pszReference, char *subtitle, int hOutFile)
{
    FS_Write("REFERENCE           ", strlen("REFERENCE           "), hOutFile);
    FS_Write(pszReference, strlen(pszReference), hOutFile);
    FS_Write("\r\nLANG_ENGLISH        \"", strlen("\r\nLANG_ENGLISH        \""), hOutFile);
    FS_Write(subtitle, strlen(subtitle), hOutFile);
    FS_Write("\"\r\n\r\n", strlen("\"\r\n\r\n"), hOutFile);

}
void __cdecl Com_SetStringEdReference(const char *pszReference, char *subtitle)
{
    int hOutFile; // [esp+74h] [ebp-22Ch]
    int bReferenceAdded; // [esp+7Ch] [ebp-224h]
    char szFromFile[256]; // [esp+80h] [ebp-220h] BYREF
    int iLen; // [esp+180h] [ebp-120h]
    const char *pszInFileReference; // [esp+184h] [ebp-11Ch]
    char *file; // [esp+188h] [ebp-118h] BYREF
    const char *ptr; // [esp+18Ch] [ebp-114h] BYREF
    char szToFile[260]; // [esp+190h] [ebp-110h] BYREF
    const char *token; // [esp+298h] [ebp-8h]
    const char *startmarker; // [esp+29Ch] [ebp-4h]

    bReferenceAdded = 0;
    pszInFileReference = pszReference + 9;
    hOutFile = FS_FOpenFileWrite((char*)"soundaliases/temp.st");
    if (hOutFile)
    {
        if (FS_ReadFile("soundaliases/subtitle.st", (void**)&file) >= 0)
        {
            Com_BeginParseSession("soundaliases/subtitle.st");
            ptr = file;
            startmarker = file;
            while (1)
            {
                token = Com_Parse(&ptr)->token;
                if (!ptr)
                    break;
                if (!strcmp(token, "ENDMARKER"))
                {
                    if (startmarker < ptr)
                    {
                        iLen = ptr - startmarker - 11;
                        FS_Write((char*)startmarker, iLen, hOutFile);
                    }
                    break;
                }
                if (!strcmp(token, "REFERENCE"))
                {
                    token = Com_ParseOnLine(&ptr)->token;
                    if (!strcmp(token, pszInFileReference))
                    {
                        if (startmarker < ptr)
                        {
                            iLen = ptr - startmarker;
                            FS_Write((char*)startmarker, ptr - startmarker, hOutFile);
                        }
                        Com_WriteStringEdReferenceToFile((char*)pszInFileReference, subtitle, hOutFile);
                        bReferenceAdded = 1;
                        do
                        {
                            startmarker = ptr;
                            token = Com_Parse(&ptr)->token;
                            if (!ptr)
                            {
                                startmarker = 0;
                                goto LABEL_22;
                            }
                        } while (strcmp(token, "REFERENCE") && strcmp(token, "ENDMARKER"));
                        Com_UngetToken();
                    }
                }
            LABEL_22:
                Com_SkipRestOfLine(&ptr);
            }
            if (!bReferenceAdded)
                Com_WriteStringEdReferenceToFile((char*)pszInFileReference, subtitle, hOutFile);
            Com_EndParseSession();
            FS_FreeFile(file);
            token = "\r\nENDMARKER\r\n\r\n\r\n";
            FS_Write((char*)"\r\nENDMARKER\r\n\r\n\r\n", strlen("\r\nENDMARKER\r\n\r\n\r\n"), hOutFile);
            FS_FCloseFile(hOutFile);
            FS_BuildOSPath((char*)fs_basepath->current.integer, fs_gamedir, (char*)"soundaliases/temp.st", szFromFile);
            FS_BuildOSPath((char*)fs_basepath->current.integer, fs_gamedir, (char*)"soundaliases/subtitle.st", szToFile);
            FS_CopyFile(szFromFile, szToFile);
            FS_Remove(szFromFile);
        }
        else
        {
            Com_PrintWarning(9, "WARNING: Could not read local copy of StringEd file %s\n", "soundaliases/subtitle.st");
            FS_FCloseFile(hOutFile);
        }
    }
    else
    {
        Com_PrintWarning(9, "WARNING: Could not open output file %s for writing\n", "soundaliases/temp.st");
    }
}

void __cdecl Com_ProcessSoundAliasFileLocalization(char *sourceFile, char *loadspecCurGame)
{
    char *v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int v9; // eax
    char v10; // [esp+33h] [ebp-8239h]
    char *v11; // [esp+38h] [ebp-8234h]
    char *v12; // [esp+3Ch] [ebp-8230h]
    int v13; // [esp+60h] [ebp-820Ch]
    int v14; // [esp+64h] [ebp-8208h]
    int h; // [esp+68h] [ebp-8204h]
    char v16[1024]; // [esp+6Ch] [ebp-8200h] BYREF
    char v17[1024]; // [esp+46Ch] [ebp-7E00h] BYREF
    char v18[2048]; // [esp+86Ch] [ebp-7A00h] BYREF
    char pszReference; // [esp+106Ch] [ebp-7200h] BYREF
    _BYTE v20[3]; // [esp+106Dh] [ebp-71FFh] BYREF
    const char *filename; // [esp+746Ch] [ebp-E00h]
    int len; // [esp+7470h] [ebp-DFCh]
    char dest[256]; // [esp+7474h] [ebp-DF8h] BYREF
    char fromOSPath[260]; // [esp+7574h] [ebp-CF8h] BYREF
    int v25; // [esp+7678h] [ebp-BF4h]
    char s[1024]; // [esp+767Ch] [ebp-BF0h] BYREF
    void *buffer; // [esp+7A7Ch] [ebp-7F0h] BYREF
    char *v28; // [esp+7A80h] [ebp-7ECh]
    char *data_p; // [esp+7A84h] [ebp-7E8h] BYREF
    FILE *stream; // [esp+7A88h] [ebp-7E4h]
    snd_alias_members_t field[256]; // [esp+7A8Ch] [ebp-7E0h]
    snd_alias_build_s alias; // [esp+7E8Ch] [ebp-3E0h] BYREF
    snd_alias_members_t i; // [esp+8030h] [ebp-23Ch]
    char ospath[256]; // [esp+8034h] [ebp-238h] BYREF
    char toOSPath[260]; // [esp+8134h] [ebp-138h] BYREF
    char *s0; // [esp+8238h] [ebp-34h]
    int v37; // [esp+823Ch] [ebp-30h]
    char isFieldSet[4]; // [esp+8240h] [ebp-2Ch] BYREF
    int v39; // [esp+8244h] [ebp-28h]
    int v40; // [esp+8248h] [ebp-24h]
    int v41; // [esp+824Ch] [ebp-20h]
    int v42; // [esp+8250h] [ebp-1Ch]
    int v43; // [esp+8254h] [ebp-18h]
    int v44; // [esp+8258h] [ebp-14h]
    char v45; // [esp+825Ch] [ebp-10h]
    snd_alias_members_t v46; // [esp+8264h] [ebp-8h]
    char *v47; // [esp+8268h] [ebp-4h]

    filename = "soundaliases/temp.csv";
    Com_sprintf(dest, 0x100u, "soundaliases/%s", sourceFile);
    FS_BuildOSPath((char*)fs_basepath->current.integer, fs_gamedir, dest, ospath);
    Com_Printf(9, "Processing sound alias file %s..\n", ospath);
    stream = fopen(ospath, "r+");
    if (!stream)
    {
        Com_PrintWarning(9, "WARNING: Can not write to sound alias file %s\n", ospath);
        return;
    }
    fclose(stream);
    if (FS_ReadFile(dest, &buffer) < 0)
    {
        Com_PrintWarning(9, "WARNING: Could not read sound alias file %s\n", dest);
        return;
    }
    h = FS_FOpenFileWrite((char*)filename);
    if (!h)
    {
        Com_PrintWarning(9, "WARNING: Could not open output file %s for writing\n", filename);
        return;
    }
    Com_BeginParseSession(dest);
    Com_SetCSV(1);
    data_p = (char*)buffer;
    v46 = SA_INVALID;
    v37 = 0;
    while (data_p)
    {
        if (*data_p == 13)
        {
            while (*data_p == 13)
                ++data_p;
        }
        if (*data_p == 10)
        {
            ++data_p;
            FS_Write((char*)"\r\n", 2u, h);
        }
        v47 = data_p;
        s0 = Com_Parse(&data_p)->token;
        if (!data_p)
            break;
        if (!I_stricmp(s0, "#Chateau"))
            i = SA_INVALID;
        if (!*s0 || *s0 == 35)
        {
            Com_SkipRestOfLine((const char **)&data_p);
            if (*(char*)v47 == 10)
                FS_Write((char*)"\r", 1u, h);
            goto LABEL_21;
        }
        if (v46)
        {
            *isFieldSet = 0;
            v39 = 0;
            v40 = 0;
            v41 = 0;
            v42 = 0;
            v43 = 0;
            v44 = 0;
            v45 = 0;
            Com_LoadSoundAliasDefaults(&alias, sourceFile, "menu");
            i = SA_INVALID;
            while (1)
            {
                v12 = s0;
                v11 = &v16[1024 * field[i]];
                do
                {
                    v10 = *v12;
                    *v11++ = *v12++;
                } while (v10);
                if (*s0)
                    Com_LoadSoundAliasField("menu", loadspecCurGame, sourceFile, s0, field[i], isFieldSet, &alias);
                if (++i == v46)
                    break;
                s0 = Com_ParseOnLine(&data_p)->token;
            }
            if (!isFieldSet[1] || !isFieldSet[3])
                Com_Error(ERR_DROP, "Sound alias file %s: alias entry missing name and/or file", sourceFile);
            v25 = 0;
            if (v39)
            {
                len = &v20[strlen(&pszReference)] - v20;
                for (i = SA_INVALID;
                    i < len
                    && (v20[i - 1] >= 65 && v20[i - 1] <= 90 || v20[i - 1] >= 48 && v20[i - 1] <= 57 || v20[i - 1] == 95);
                    ++i)
                {
                    ;
                }
                if (i < len || I_strncmp(&pszReference, "SUBTITLE_", 9) || !Com_StringEdReferenceExists(&pszReference))
                    v25 = 1;
            }
            if (v25)
            {
                for (i = SA_INVALID; i < v46; ++i)
                {
                    if (field[i] && isFieldSet[field[i]])
                    {
                        if (field[i] == SA_SUBTITLE)
                        {
                            s0 = Com_GetSubtitleStringEdReference(&pszReference);
                            if (s0)
                            {
                                Com_sprintf(s, 0x400u, "%s%s", "SUBTITLE_", s0);
                                s0 = I_strupr(s);
                            }
                            else
                            {
                                if (isFieldSet[2])
                                    Com_sprintf(s, 0x400u, "%s%s_%s", "SUBTITLE_", v17, v18);
                                else
                                    Com_sprintf(s, 0x400u, "%s%s", "SUBTITLE_", v17);
                                s0 = I_strupr(s);
                                Com_SetStringEdReference(s, &pszReference);
                                ++v37;
                            }
                        }
                        else if (i == v46 - 1)
                        {
                            //v2 = strchr(&v16[1024 * field[i]], 0x2Cu);
                            //if (v2
                            //    || (strchr(&v16[1024 * field[i]], 0x20u), v3)
                            //    || (strchr(&v16[1024 * field[i]], 0xAu), v4)
                            //    || (strchr(&v16[1024 * field[i]], 0xDu), v5))
                            if ( strchr(&v16[1024 * field[i]], 0x2Cu) || 
                                strchr(&v16[1024 * field[i]], 0x20u) || 
                                strchr(&v16[1024 * field[i]], 0xAu) || 
                                strchr(&v16[1024 * field[i]], 0xDu))
                            {
                                s0 = va("\"%s\"", &v16[1024 * field[i]]);
                            }
                            else
                            {
                                s0 = va("%s", &v16[1024 * field[i]]);
                            }
                        }
                        else
                        {
                            //strchr(&v16[1024 * field[i]], 0x2Cu);
                            //if (v6
                            //    || (strchr(&v16[1024 * field[i]], 0x20u), v7)
                            //    || (strchr(&v16[1024 * field[i]], 0xAu), v8)
                            //    || (strchr(&v16[1024 * field[i]], 0xDu), v9))
                            if (strchr(&v16[1024 * field[i]], 0x2Cu) || strchr(&v16[1024 * field[i]], 0x20u) || strchr(&v16[1024 * field[i]], 0xAu) || strchr(&v16[1024 * field[i]], 0xDu))
                            {
                                s0 = va("\"%s\",", &v16[1024 * field[i]]);
                            }
                            else
                            {
                                s0 = va("%s,", &v16[1024 * field[i]]);
                            }
                        }
                        len = strlen(s0);
                        FS_Write(s0, len, h);
                    }
                    else if (i != v46 - 1)
                    {
                        FS_Write((char*)",", 1u, h);
                    }
                }
                FS_Write((char*)"\r\n", 2u, h);
                Com_SkipRestOfLine((const char**)&data_p);
            }
            else
            {
                Com_SkipRestOfLine((const char **)&data_p);
                v28 = data_p;
                FS_Write((char*)v47, data_p - v47, h);
            }
        }
        else
        {
            v13 = 0;
            v14 = 0;
            while (2)
            {
                field[v46] = SA_INVALID;
                for (i = SA_NAME; i < SA_NUMFIELDS; ++i)
                {
                    if (!I_stricmp(g_pszSndAliasKeyNames[i], s0))
                    {
                        field[v46] = i;
                        if (i == SA_NAME)
                        {
                            v13 = 1;
                        }
                        else if (i == SA_FILE)
                        {
                            v14 = 1;
                        }
                        break;
                    }
                }
                if (++v46 != 256 && data_p && *data_p != 10)
                {
                    s0 = Com_ParseOnLine(&data_p)->token;
                    continue;
                }
                break;
            }
            if (!v13 || !v14)
                Com_Error(ERR_DROP, "Sound alias file %s: missing name and/or file columns", sourceFile);
            Com_SkipRestOfLine((const char **)&data_p);
            if (*(char*)v47 == 10)
                FS_Write((char*)"\r", 1u, h);
        LABEL_21:
            v28 = data_p;
            FS_Write((char *)v47, data_p - v47, h);
        }
    }
    Com_EndParseSession();
    FS_FCloseFile(h);
    FS_BuildOSPath((char *)fs_basepath->current.integer, fs_gamedir, (char *)filename, fromOSPath);
    FS_BuildOSPath((char *)fs_basepath->current.integer, fs_gamedir, dest, toOSPath);
    if (v37)
        FS_CopyFile(fromOSPath, toOSPath);
    FS_Remove(fromOSPath);
    Com_Printf(9, "Localized %i sound alias subtitles\n", v37);
}

void __cdecl Com_InitSoundAliasHash(unsigned int aliasCount)
{
    g_sa.hashUsed = 0;
    g_sa.hashSize = (3 * aliasCount + 1) >> 1;
    g_sa.hash = (snd_alias_list_t**)CM_Hunk_Alloc(4 * ((3 * aliasCount + 1) >> 1), "Com_InitSoundAliasHash", 15);
    memset(g_sa.hash, 0, 4 * ((3 * aliasCount + 1) >> 1));
}

cmd_function_s Com_RefreshSpeakerMaps_f_VAR;
void __cdecl Com_RefreshSpeakerMaps_f()
{
    int speakerMapIndex; // [esp+0h] [ebp-4h]

    if (!g_sa.speakerMapsInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 334, 0, "%s", "g_sa.speakerMapsInitialized");
    for (speakerMapIndex = 1; *g_sa.speakerMaps[speakerMapIndex].speakerMap.name; ++speakerMapIndex)
    {
        if (!Com_LoadSpkrMapFile((char*)g_sa.speakerMaps[speakerMapIndex].speakerMap.name, &g_sa.speakerMaps[speakerMapIndex]))
            Com_Error(ERR_DROP, "Failed to load speaker map %s", g_sa.speakerMaps[speakerMapIndex].speakerMap.name);
    }
}

void Com_InitDefaultSpeakerMap()
{
    Com_InitDefaultSoundAliasSpeakerMap(g_sa.speakerMaps);
}

void Com_InitSpeakerMaps()
{
    int fileIndex; // [esp+10h] [ebp-58h]
    const char **fileNames; // [esp+14h] [ebp-54h]
    char name[68]; // [esp+18h] [ebp-50h] BYREF
    int len; // [esp+60h] [ebp-8h]
    int fileCount; // [esp+64h] [ebp-4h] BYREF

    if (g_sa.speakerMapsInitialized)
        MyAssertHandler(".\\universal\\com_sndalias.cpp", 360, 0, "%s", "!g_sa.speakerMapsInitialized");
    Com_InitDefaultSpeakerMap();
    fileNames = FS_ListFiles("soundaliases", "spkrmap", FS_LIST_PURE_ONLY, &fileCount);
    if (fileCount > 15)
        Com_Error(ERR_DROP, "Snd_Alias Curve initialization: .vfcurve file count (%d) exceeds maximum (%d) ", fileCount, 15);
    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
    {
        len = strlen(fileNames[fileIndex]) - 8;
        if (len >= 64)
            Com_Error(ERR_DROP, "Speaker map %s name too long", fileNames[fileIndex]);
        strncpy(name, fileNames[fileIndex], len);
        name[len] = 0;
        if (!Com_LoadSpkrMapFile(name, &g_sa.speakerMaps[fileIndex + 1]))
            Com_Error(ERR_DROP, "Failed to load speaker map %s", fileNames[fileIndex]);
    }
    FS_FreeFileList(fileNames);
    Cmd_AddCommandInternal("snd_refreshSpeakerMaps", Com_RefreshSpeakerMaps_f, &Com_RefreshSpeakerMaps_f_VAR);
    g_sa.speakerMapsInitialized = 1;
}