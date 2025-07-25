#pragma once
#include <qcommon/qcommon.h>
#include <qcommon/graph.h>
#include <sound/snd_public.h>

static const char *g_pszSndAliasKeyNames[29] =
{
  NULL,
  "name",
  "sequence",
  "file",
  "subtitle",
  "vol_min",
  "vol_max",
  "vol_mod",
  "pitch_min",
  "pitch_max",
  "dist_min",
  "dist_max",
  "channel",
  "type",
  "loop",
  "probability",
  "loadspec",
  "masterslave",
  "secondaryaliasname",
  "chainaliasname",
  "volumefalloffcurve",
  "startdelay",
  "speakermap",
  "reverb",
  "lfe percentage",
  "center percentage",
  "envelop_min",
  "envelop_max",
  "envelop percentage"
}; // idb

enum snd_alias_type_t : __int32
{                                       // ...
    SAT_UNKNOWN = 0x0,
    SAT_LOADED = 0x1,
    SAT_STREAMED = 0x2,
    SAT_COUNT = 0x3,
};

enum snd_alias_members_t : __int32
{                                       // ...
    SA_INVALID = 0x0,
    SA_NAME = 0x1,
    SA_SEQUENCE = 0x2,
    SA_FILE = 0x3,
    SA_SUBTITLE = 0x4,
    SA_VOL_MIN = 0x5,
    SA_VOL_MAX = 0x6,
    SA_VOL_MOD = 0x7,
    SA_PITCH_MIN = 0x8,
    SA_PITCH_MAX = 0x9,
    SA_DIST_MIN = 0xA,
    SA_DIST_MAX = 0xB,
    SA_CHANNEL = 0xC,
    SA_TYPE = 0xD,
    SA_LOOP = 0xE,
    SA_PROBABILITY = 0xF,
    SA_LOADSPEC = 0x10,
    SA_MASTERSLAVE = 0x11,
    SA_SECONDARYALIASNAME = 0x12,
    SA_CHAINALIASNAME = 0x13,
    SA_VOLUMEFALLOFFCURVE = 0x14,
    SA_STARTDELAY = 0x15,
    SA_SPEAKERMAP = 0x16,
    SA_REVERB = 0x17,
    SA_LFEPERCENTAGE = 0x18,
    SA_CENTERPERCENTAGE = 0x19,
    SA_ENVELOPMIN = 0x1A,
    SA_ENVELOPMAX = 0x1B,
    SA_ENVELOPPERCENTAGE = 0x1C,
    SA_NUMFIELDS = 0x1D,
};
enum SA_SPKRMAPIDENTIFIERS : __int32
{                                       // ...
    SA_MONOSOURCE = 0x0,
    SA_LEFTSOURCE = 0x1,
    SA_RIGHTSOURCE = 0x2,
    SA_LEFTSPEAKER = 0x3,
    SA_RIGHTSPEAKER = 0x4,
    SA_CENTERSPEAKER = 0x5,
    SA_LFESPEAKER = 0x6,
    SA_LEFTSURROUNDSPEAKER = 0x7,
    SA_RIGHTSURROUNDSPEAKER = 0x8,
    SA_SPKRMAPIDENTIFIERCOUNT = 0x9,
};
inline snd_alias_members_t &operator++(snd_alias_members_t &e) {
    e = static_cast<snd_alias_members_t>(static_cast<int>(e) + 1);
    return e;
}
inline snd_alias_members_t &operator++(snd_alias_members_t &e, int i)
{
    ++e;
    return e;
}

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

struct snd_alias_build_s // sizeof=0x19C
{                                       // ...
    char szSourceFile[64];
    char aliasName[64];                 // ...
    char secondaryAliasName[64];
    char chainAliasName[64];
    char *subtitleText;
    int sequence;
    char soundFile[64];                 // ...
    SoundFile *permSoundFile;
    float volMin;
    float volMax;
    float fVolMod;
    float pitchMin;
    float pitchMax;
    float distMin;
    float distMax;
    int iChannel;
    snd_alias_type_t eType;
    SndCurve *volumeFalloffCurve;
    SpeakerMap *speakerMap;
    float slavePercentage;
    float probability;
    float lfePercentage;
    float centerPercentage;
    int startDelay;
    unsigned __int8 bLooping;
    unsigned __int8 bMaster;
    unsigned __int8 bSlave;
    unsigned __int8 bFullDryLevel;
    unsigned __int8 bNoWetLevel;
    unsigned __int8 bRandomLooping;
    bool error;                         // ...
    bool keep;                          // ...
    snd_alias_build_s *pSameSoundFile;
    snd_alias_build_s *pNext;
};

struct VolumeModGroup // sizeof=0x44
{                                       // ...
    char name[64];
    float value;
};

struct SoundAliasLoadGlobals // sizeof=0x1894
{                                       // ...
    snd_alias_build_s *tempAliases;     // ...
    int tempAliasCount;                 // ...
    VolumeModGroup volumeModGroups[32]; // ...
    bool volumeModGroupsInitialized;    // ...
    char entChannels[64][64];           // ...
    // padding byte
    // padding byte
    // padding byte
    int entChannelCount;                // ...
    bool refreshVolumeModGroupsCommandInitialized; // ...
    // padding byte
    // padding byte
    // padding byte
};

double __cdecl Com_GetVolumeFalloffCurveValue(SndCurve *volumeFalloffCurve, float fraction);
void __cdecl Com_InitSoundDevGuiGraphs();
void __cdecl Com_VolumeFalloffCurveGraphEventCallback(const DevGraph *graph, DevEventType event, int i);
void Com_InitSoundDevGuiGraphs_FastFile();
void __cdecl Com_GetGraphList(XAssetHeader header, int *data);
MSSChannelMap *__cdecl Com_GetSpeakerMap(SpeakerMap *speakerMap, int sourceChannelCount);
snd_alias_list_t *__cdecl Com_TryFindSoundAlias(const char *name);
snd_alias_list_t *__cdecl Com_TryFindSoundAlias_FastFile(const char *name);
snd_alias_list_t *__cdecl Com_FindSoundAlias(const char *name);
inline snd_alias_list_t *__cdecl Com_FindSoundAlias_DEDICATED(const char *name)
{
    return NULL;
}
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
void __cdecl Com_UnloadSoundAliases(snd_alias_system_t system);

char __cdecl Com_AddAliasList(const char *name, snd_alias_list_t *aliasList);

void __cdecl Com_InitSoundDevGuiGraphs();
void Com_InitSoundDevGuiGraphs_LoadObj();
void Com_InitSoundDevGuiGraphs_FastFile();
void __cdecl Com_InitSoundAliasHash(unsigned int aliasCount);
void Com_InitSpeakerMaps();
void Com_InitDefaultSpeakerMap();
void Com_InitCurves();
void __cdecl Com_InitSoundAlias();
void Com_InitEntChannels();

bool __cdecl Com_ParseSndCurveFile(const char *buffer, const char *fileName, SndCurve *curve);

void __cdecl Com_LoadSoundAliasFile(const char *loadspec, const char *loadspecCurGame, const char *sourceFile);

void __cdecl Com_MakeSoundAliasesPermanent(snd_alias_list_t *aliasInfo, SoundFileInfo *soundFileInfo);

int __cdecl Com_LoadSoundAliasSounds(SoundFileInfo *soundFileInfo);

void __cdecl Com_ParseEntChannelFile(const char *buffer);
void __cdecl Com_LoadSoundAliasField(
    const char *loadspec,
    const char *loadspecCurGame,
    const char *sourceFile,
    char *token,
    snd_alias_members_t field,
    char *isFieldSet,
    snd_alias_build_s *alias);
void __cdecl Com_LoadSoundAliasDefaults(snd_alias_build_s *alias, const char *sourceFile, const char *loadspec);
void __cdecl Com_SetChannelMapEntry(
    MSSChannelMap *entry,
    unsigned int inputChannel,
    unsigned int outputChannel,
    float volume);


void __cdecl Com_PreLoadSpkrMapFile(SpeakerMapInfo *info);
char __cdecl Com_LoadSpkrMapFile(char *name, SpeakerMapInfo *info);


extern SoundAliasGlobals g_sa;