#include "com_sndalias.h"
#include "com_memory.h"
#include <devgui/devgui.h>
#include <sound/snd_local.h>

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

