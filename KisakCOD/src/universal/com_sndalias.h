#pragma once
#include <qcommon/qcommon.h>
#include <qcommon/graph.h>
#include <sound/snd_public.h>

struct SoundFileInfo // sizeof=0x8
{                                       // ...
    int count;                          // ...
    SoundFile *files;                   // ...
};

struct SoundAliasGlobals // sizeof=0x2858
{                                       // ...
    unsigned __int8 initialized[3];     // ...
    // padding byte
    DevGraph curveDevGraphs[16];        // ...
    int randSeed;                       // ...
    unsigned int hashUsed;              // ...
    unsigned int hashSize;              // ...
    snd_alias_list_t **hash;            // ...
    snd_alias_list_t aliasInfo[3];      // ...
    SoundFileInfo soundFileInfo[3];     // ...
    bool curvesInitialized;             // ...
    // padding byte
    // padding byte
    // padding byte
    SndCurve volumeFalloffCurves[16];   // ...
    char volumeFalloffCurveNames[16][64]; // ...
    bool speakerMapsInitialized;        // ...
    // padding byte
    // padding byte
    // padding byte
    SpeakerMapInfo speakerMaps[16];     // ...
};

double __cdecl Com_GetVolumeFalloffCurveValue(SndCurve *volumeFalloffCurve, float fraction);
void __cdecl Com_InitSoundDevGuiGraphs();
void __cdecl Com_VolumeFalloffCurveGraphEventCallback(const DevGraph *graph, DevEventType event);
void Com_InitSoundDevGuiGraphs_FastFile();
void __cdecl Com_GetGraphList(XAssetHeader header, int *data);
MSSChannelMap *__cdecl Com_GetSpeakerMap(SpeakerMap *speakerMap, int sourceChannelCount);
snd_alias_list_t *__cdecl Com_TryFindSoundAlias(const char *name);
snd_alias_list_t *__cdecl Com_TryFindSoundAlias_FastFile(const char *name);
snd_alias_list_t *__cdecl Com_FindSoundAlias(const char *name);
snd_alias_list_t *__cdecl Com_FindSoundAlias_FastFile(const char *name);
snd_alias_list_t *__cdecl Com_FindSoundAliasNoErrors(const char *name);
snd_alias_list_t *__cdecl Com_FindSoundAliasNoErrors_FastFile(const char *name);
void __cdecl Load_SndAliasCustom(snd_alias_list_t **var);
int __cdecl SND_GetAliasOffset(const snd_alias_t *alias);
snd_alias_t *__cdecl SND_GetAliasWithOffset(const char *name, int offset);
void __cdecl Com_GetSoundFileName(const snd_alias_t *alias, char *filename, int size);
snd_alias_t *__cdecl Com_PickSoundAliasFromList(snd_alias_list_t *aliasList);
snd_alias_t *__cdecl Com_PickSoundAlias(const char *aliasname);
bool __cdecl Com_AliasNameRefersToSingleAlias(const char *aliasname);

void __cdecl Com_LoadSoundAliases(const char *loadspec, const char *loadspecCurGame, snd_alias_system_t system);

extern SoundAliasGlobals g_sa;