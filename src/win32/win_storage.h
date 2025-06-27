#pragma once

enum StatType : __int32
{                                       // ...
    STAT_TYPE_PRIMARY = 0x1,          // ...
    STAT_TYPE_SECONDARY = 0x2,          // ...
    STAT_TYPE_EQUIPMENT = 0x4,          // ...
    STAT_TYPE_WEAPON = 0x8,          // ...
    STAT_TYPE_ABILITY = 0x10,         // ...
    STAT_TYPE_GRENADE = 0x20,         // ...
};

struct StatsData // sizeof=0x2104
{                                       // ...
    char path[260];
    unsigned __int8 stats[8192];        // ...
};
struct StatsFile_s// sizeof=0x2114
{                                       // ...
    unsigned int hash[4];
    StatsData statsData;                // ...
};
struct StatsFile // sizeof=0x211C
{                                       // ...
    unsigned __int8 magic[4];
    unsigned int nonce;
    StatsFile_s body; // ...
};

struct playerStatNetworkData // sizeof=0x2002
{                                       // ...
    unsigned __int8 playerStats[8192];  // ...
    bool statsFetched;                  // ...
    bool statWriteNeeded;               // ...
};

struct CaCItem // sizeof=0xC
{                                       // ...
    int itemIndex;                      // ...
    int minLevel;
    StatType type;
};

void __cdecl LiveStorage_ValidateCaCStat(int controllerIndex, int index, int value);
void __cdecl LiveStorage_ValidateSetStatCmd(int index, int value);
void __cdecl LiveStorage_StatsInit(int controllerIndex);
void __cdecl LiveStorage_TrySetStat(int controllerIndex, int index, unsigned int value);
void __cdecl LiveStorage_TrySetStatRange(int controllerIndex, int first, int last, unsigned int value);
void __cdecl LiveStorage_WeaponPerkChallengeReset(int controllerIndex);
void __cdecl LiveStorage_UnlockClassAssault(int controllerIndex);
void __cdecl LiveStorage_UnlockClassDemolitions(int controllerIndex);
void __cdecl LiveStorage_UnlockClassHeavyGunner(int controllerIndex);
void __cdecl LiveStorage_UnlockClassSniper(int controllerIndex);
void __cdecl LiveStorage_UnlockClassSpecOps(int controllerIndex);
void __cdecl LiveStorage_SetFromLocString(int controllerIndex, const char *dvarName, char *preLocalizedText);
void __cdecl LiveStorage_ReadStats();
void __cdecl LiveStorage_ReadStatsFromDir(char *directory);
bool __cdecl LiveStorage_DecryptAndCheck(StatsFile *statsFile, const char *statsDir);
void __cdecl LiveStorage_GetCryptKey(unsigned int nonce, unsigned __int8 *outKey);
int __cdecl LiveStorage_ChecksumGamerStats(unsigned __int8 *buffer, int len);
void LiveStorage_NoStatsFound();
void __cdecl LiveStorage_WriteChecksumToBuffer(unsigned __int8 *buffer, int len);
bool __cdecl LiveStorage_ReadStatsFile(const char *qpath, unsigned __int8 *buffer, unsigned int lenToRead);
void __cdecl LiveStorage_HandleCorruptStats(char *filename);
playerStatNetworkData *__cdecl LiveStorage_GetStatBuffer();
bool __cdecl LiveStorage_DoWeHaveStats();
void __cdecl LiveStorage_StatsWriteNeeded();
void __cdecl LiveStorage_UploadStats();
void __cdecl LiveStorage_Encrypt(StatsFile *statsFile);
int __cdecl LiveStorage_GetStat(int __formal, int index);
void __cdecl LiveStorage_SetStat(int __formal, int index, unsigned int value);
void __cdecl LiveStorage_TrySetStatForCmd(int index, unsigned int value);
void __cdecl LiveStorage_NewUser();
void __cdecl LiveStorage_Init();
void __cdecl LiveStorage_StatSetCmd();
void __cdecl LiveStorage_StatGetCmd();
void __cdecl LiveStorage_StatGetInDvarCmd();
void __cdecl LiveStorage_UploadStatsCmd();
void __cdecl LiveStorage_ReadStatsCmd();
