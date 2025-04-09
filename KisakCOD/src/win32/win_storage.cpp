#include "win_storage.h"
#include <qcommon/qcommon.h>
#include <client_mp/client_mp.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/com_playerprofile.h>
#include <universal/com_files.h>
#include <qcommon/cmd.h>
#include <qcommon/md4.h>
#include <qcommon/com_bsp.h>

const dvar_t *debugStats;
const dvar_t *stat_version;

playerStatNetworkData statData;

CaCItem cacItems[61] =
{
  { 0, 0, STAT_TYPE_SECONDARY },
  { 1, 15, STAT_TYPE_SECONDARY },
  { 2, 0, STAT_TYPE_SECONDARY },
  { 3, 42, STAT_TYPE_SECONDARY },
  { 4, 54, STAT_TYPE_SECONDARY },
  { 10, 0, STAT_TYPE_PRIMARY },
  { 11, 0, STAT_TYPE_PRIMARY },
  { 12, 12, STAT_TYPE_PRIMARY },
  { 13, 27, STAT_TYPE_PRIMARY },
  { 14, 39, STAT_TYPE_PRIMARY },
  { 20, 0, STAT_TYPE_PRIMARY },
  { 21, 45, STAT_TYPE_PRIMARY },
  { 22, 51, STAT_TYPE_PRIMARY },
  { 23, 24, STAT_TYPE_PRIMARY },
  { 24, 36, STAT_TYPE_PRIMARY },
  { 25, 0, STAT_TYPE_PRIMARY },
  { 26, 9, STAT_TYPE_PRIMARY },
  { 50, 0, STAT_TYPE_EQUIPMENT },
  { 55, 0, STAT_TYPE_EQUIPMENT },
  { 60, 21, STAT_TYPE_PRIMARY },
  { 61, 0, STAT_TYPE_PRIMARY },
  { 62, 48, STAT_TYPE_PRIMARY },
  { 64, 33, STAT_TYPE_PRIMARY },
  { 65, 6, STAT_TYPE_PRIMARY },
  { 70, 30, STAT_TYPE_PRIMARY },
  { 71, 0, STAT_TYPE_PRIMARY },
  { 80, 0, STAT_TYPE_PRIMARY },
  { 81, 0, STAT_TYPE_PRIMARY },
  { 82, 18, STAT_TYPE_PRIMARY },
  { 90, 0, STAT_TYPE_EQUIPMENT },
  { 91, 22, STAT_TYPE_EQUIPMENT },
  { 100, 0, STAT_TYPE_GRENADE },
  { 101, 0, STAT_TYPE_GRENADE },
  { 102, 0, STAT_TYPE_GRENADE },
  { 103, 0, STAT_TYPE_GRENADE },
  { 150, 34, STAT_TYPE_ABILITY },
  { 151, 10, STAT_TYPE_WEAPON },
  { 152, 25, STAT_TYPE_ABILITY },
  { 153, 43, STAT_TYPE_ABILITY },
  { 154, 0, STAT_TYPE_ABILITY },
  { 155, 13, STAT_TYPE_EQUIPMENT },
  { 156, 0, STAT_TYPE_WEAPON },
  { 157, 7, STAT_TYPE_ABILITY },
  { 158, 16, STAT_TYPE_ABILITY },
  { 160, 0, STAT_TYPE_WEAPON },
  { 161, 0, STAT_TYPE_ABILITY },
  { 162, 0, STAT_TYPE_ABILITY },
  { 163, 28, STAT_TYPE_WEAPON },
  { 164, 19, STAT_TYPE_WEAPON },
  { 165, 31, STAT_TYPE_EQUIPMENT },
  { 166, 37, STAT_TYPE_WEAPON },
  { 167, 0, STAT_TYPE_WEAPON },
  { 173, 40, STAT_TYPE_EQUIPMENT },
  { 176, 0, STAT_TYPE_EQUIPMENT },
  { 184, 0, STAT_TYPE_EQUIPMENT },
  { 185, 22, STAT_TYPE_EQUIPMENT },
  { 186, 0, STAT_TYPE_EQUIPMENT },
  { 190, 0, STAT_TYPE_EQUIPMENT },
  { 191, 0, STAT_TYPE_EQUIPMENT },
  { 192, 0, STAT_TYPE_EQUIPMENT },
  { 193, 0, STAT_TYPE_EQUIPMENT }
}; // idb

const CaCItem *__cdecl Script_FindCacItem(int itemIndex)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x3D; ++i)
    {
        if (cacItems[i].itemIndex == itemIndex)
            return &cacItems[i];
    }
    return 0;
}

void __cdecl Script_ValidateSlotItem(int itemIndex, int level, unsigned int typeMask)
{
    const CaCItem *cacItem; // [esp+0h] [ebp-4h]

    cacItem = Script_FindCacItem(itemIndex);
    if (!cacItem)
        Com_Error(ERR_DROP, "No create-a-class item found at index %d\n", itemIndex);
    if (level < cacItem->minLevel)
        Com_Error(
            ERR_DROP,
            "Create-a-class item %d is too high level %d < %d.\n",
            itemIndex,
            level + 1 < cacItem->minLevel + 1,
            cacItem);
    if ((cacItem->type & typeMask) == 0)
        Com_Error(
            ERR_DROP,
            "Create-a-class item %d is not the expected type for this slot (0x%x & 0x%x) == 0.\n\n",
            itemIndex,
            typeMask,
            cacItem->type);
}

bool __cdecl Script_CaCHasOverkill(int controllerIndex, int index)
{
    return LiveStorage_GetStat(controllerIndex, 10 * (index / 10) + 6) == 166;
}

void __cdecl LiveStorage_ValidateCaCStat(int controllerIndex, int index, int value)
{
    int rank; // [esp+4h] [ebp-8h]
    int xp; // [esp+8h] [ebp-4h]

    if (index < 200 || index >= 250)
        Com_Error(ERR_DROP, "trying to set invalid create-a-class stat.  You can't set stat %d.\n", index);
    xp = LiveStorage_GetStat(controllerIndex, 2301);
    rank = CL_GetRankForXp(xp);
    switch (index % 10)
    {
    case 1:
        Script_ValidateSlotItem(value, rank, 1u);
        break;
    case 3:
        if (Script_CaCHasOverkill(controllerIndex, index))
            Script_ValidateSlotItem(value, rank, 3u);
        else
            Script_ValidateSlotItem(value, rank, 2u);
        break;
    case 5:
        Script_ValidateSlotItem(value, rank, 4u);
        break;
    case 6:
        Script_ValidateSlotItem(value, rank, 8u);
        break;
    case 7:
        Script_ValidateSlotItem(value, rank, 0x10u);
        break;
    case 8:
        Script_ValidateSlotItem(value, rank, 0x20u);
        break;
    default:
        return;
    }
}

void __cdecl LiveStorage_ValidateSetStatCmd(int index, int value)
{
    int controller; // [esp+0h] [ebp-4h]

    controller = RETURN_ZERO32();
    if (index < 256 || index > 279)
    {
        if (index < 200 || index > 250)
        {
            if (value)
                Com_Error(ERR_DROP, "Trying to set a server-only stat %d.\n", index);
        }
        else
        {
            LiveStorage_ValidateCaCStat(controller, index, value);
        }
    }
}

void __cdecl LiveStorage_StatsInit(int controllerIndex)
{
    LiveStorage_WeaponPerkChallengeReset(controllerIndex);
    LiveStorage_TrySetStatRange(controllerIndex, 2300, 2353, 0);
    LiveStorage_UnlockClassAssault(controllerIndex);
    LiveStorage_UnlockClassDemolitions(controllerIndex);
    LiveStorage_UnlockClassHeavyGunner(controllerIndex);
    LiveStorage_UnlockClassSniper(controllerIndex);
    LiveStorage_UnlockClassSpecOps(controllerIndex);
    if (!*(_BYTE *)fs_gameDirVar->current.integer)
    {
        Dvar_SetStringByName("clanName", (char *)"");
        LiveStorage_SetFromLocString(controllerIndex, "customclass1", (char*)"CLASS_SLOT1");
        LiveStorage_SetFromLocString(controllerIndex, "customclass2", (char*)"CLASS_SLOT2");
        LiveStorage_SetFromLocString(controllerIndex, "customclass3", (char*)"CLASS_SLOT3");
        LiveStorage_SetFromLocString(controllerIndex, "customclass4", (char*)"CLASS_SLOT4");
        LiveStorage_SetFromLocString(controllerIndex, "customclass5", (char*)"CLASS_SLOT5");
    }
    LiveStorage_TrySetStat(controllerIndex, 200, 0);
    LiveStorage_TrySetStat(controllerIndex, 210, 0);
    LiveStorage_TrySetStat(controllerIndex, 220, 0);
    LiveStorage_TrySetStat(controllerIndex, 230, 0);
    LiveStorage_TrySetStat(controllerIndex, 240, 0);
    LiveStorage_TrySetStat(controllerIndex, 250, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 251, 269, 0);
    LiveStorage_TrySetStat(controllerIndex, 3000, 9u);
    LiveStorage_TrySetStat(controllerIndex, 3002, 9u);
}

void __cdecl LiveStorage_TrySetStat(int controllerIndex, int index, unsigned int value)
{
    if (LiveStorage_GetStat(controllerIndex, index) != value)
        LiveStorage_SetStat(controllerIndex, index, value);
}

void __cdecl LiveStorage_TrySetStatRange(int controllerIndex, int first, int last, unsigned int value)
{
    while (first <= last)
        LiveStorage_TrySetStat(controllerIndex, first++, value);
}

void __cdecl LiveStorage_WeaponPerkChallengeReset(int controllerIndex)
{
    LiveStorage_TrySetStatRange(controllerIndex, 290, 298, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 270, 289, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 0, 199, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 501, 840, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 2501, 2840, 0);
    LiveStorage_TrySetStatRange(controllerIndex, 3000, 3149, 0);
}

void __cdecl LiveStorage_UnlockClassAssault(int controllerIndex)
{
    LiveStorage_TrySetStat(controllerIndex, 201, 0x19u);
    LiveStorage_TrySetStat(controllerIndex, 202, 5u);
    LiveStorage_TrySetStat(controllerIndex, 203, 0);
    LiveStorage_TrySetStat(controllerIndex, 204, 0);
    LiveStorage_TrySetStat(controllerIndex, 205, 0xBFu);
    LiveStorage_TrySetStat(controllerIndex, 206, 0xA0u);
    LiveStorage_TrySetStat(controllerIndex, 207, 0x9Au);
    LiveStorage_TrySetStat(controllerIndex, 208, 0x67u);
    LiveStorage_TrySetStat(controllerIndex, 209, 0);
    LiveStorage_TrySetStat(controllerIndex, 3025, 0x321u);
    LiveStorage_TrySetStat(controllerIndex, 3020, 0x321u);
    LiveStorage_TrySetStat(controllerIndex, 190, 1u);
    LiveStorage_TrySetStat(controllerIndex, 160, 1u);
    LiveStorage_TrySetStat(controllerIndex, 154, 1u);
    LiveStorage_TrySetStat(controllerIndex, 103, 1u);
}

void __cdecl LiveStorage_UnlockClassDemolitions(int controllerIndex)
{
    LiveStorage_TrySetStat(controllerIndex, 231, 0x47u);
    LiveStorage_TrySetStat(controllerIndex, 232, 0);
    LiveStorage_TrySetStat(controllerIndex, 233, 0);
    LiveStorage_TrySetStat(controllerIndex, 234, 0);
    LiveStorage_TrySetStat(controllerIndex, 235, 0xBAu);
    LiveStorage_TrySetStat(controllerIndex, 236, 0x9Cu);
    LiveStorage_TrySetStat(controllerIndex, 237, 0x9Au);
    LiveStorage_TrySetStat(controllerIndex, 238, 0x66u);
    LiveStorage_TrySetStat(controllerIndex, 239, 0);
    LiveStorage_TrySetStat(controllerIndex, 3071, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 186, 1u);
    LiveStorage_TrySetStat(controllerIndex, 156, 1u);
    LiveStorage_TrySetStat(controllerIndex, 154, 1u);
    LiveStorage_TrySetStat(controllerIndex, 102, 1u);
}

void __cdecl LiveStorage_UnlockClassHeavyGunner(int controllerIndex)
{
    LiveStorage_TrySetStat(controllerIndex, 221, 0x51u);
    LiveStorage_TrySetStat(controllerIndex, 222, 0);
    LiveStorage_TrySetStat(controllerIndex, 223, 2u);
    LiveStorage_TrySetStat(controllerIndex, 224, 0);
    LiveStorage_TrySetStat(controllerIndex, 225, 0xB0u);
    LiveStorage_TrySetStat(controllerIndex, 226, 0xA7u);
    LiveStorage_TrySetStat(controllerIndex, 227, 0xA1u);
    LiveStorage_TrySetStat(controllerIndex, 228, 0x67u);
    LiveStorage_TrySetStat(controllerIndex, 229, 0);
    LiveStorage_TrySetStat(controllerIndex, 3081, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 3080, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 176, 1u);
    LiveStorage_TrySetStat(controllerIndex, 167, 1u);
    LiveStorage_TrySetStat(controllerIndex, 161, 1u);
    LiveStorage_TrySetStat(controllerIndex, 103, 1u);
}

void __cdecl LiveStorage_UnlockClassSniper(int controllerIndex)
{
    LiveStorage_TrySetStat(controllerIndex, 241, 0x3Du);
    LiveStorage_TrySetStat(controllerIndex, 242, 0);
    LiveStorage_TrySetStat(controllerIndex, 243, 0);
    LiveStorage_TrySetStat(controllerIndex, 244, 3u);
    LiveStorage_TrySetStat(controllerIndex, 245, 0xB0u);
    LiveStorage_TrySetStat(controllerIndex, 246, 0xA0u);
    LiveStorage_TrySetStat(controllerIndex, 247, 0xA1u);
    LiveStorage_TrySetStat(controllerIndex, 248, 0x65u);
    LiveStorage_TrySetStat(controllerIndex, 249, 0);
    LiveStorage_TrySetStat(controllerIndex, 3061, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 176, 1u);
    LiveStorage_TrySetStat(controllerIndex, 160, 1u);
    LiveStorage_TrySetStat(controllerIndex, 161, 1u);
    LiveStorage_TrySetStat(controllerIndex, 101, 1u);
}

void __cdecl LiveStorage_UnlockClassSpecOps(int controllerIndex)
{
    LiveStorage_TrySetStat(controllerIndex, 211, 0xAu);
    LiveStorage_TrySetStat(controllerIndex, 212, 0);
    LiveStorage_TrySetStat(controllerIndex, 213, 2u);
    LiveStorage_TrySetStat(controllerIndex, 214, 3u);
    LiveStorage_TrySetStat(controllerIndex, 215, 0xB8u);
    LiveStorage_TrySetStat(controllerIndex, 216, 0x9Cu);
    LiveStorage_TrySetStat(controllerIndex, 217, 0xA2u);
    LiveStorage_TrySetStat(controllerIndex, 218, 0x65u);
    LiveStorage_TrySetStat(controllerIndex, 219, 0);
    LiveStorage_TrySetStat(controllerIndex, 3010, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 3011, 0x301u);
    LiveStorage_TrySetStat(controllerIndex, 184, 1u);
    LiveStorage_TrySetStat(controllerIndex, 156, 1u);
    LiveStorage_TrySetStat(controllerIndex, 162, 1u);
    LiveStorage_TrySetStat(controllerIndex, 101, 1u);
}

void __cdecl LiveStorage_SetFromLocString(int controllerIndex, const char *dvarName, char *preLocalizedText)
{
    char *localizedText; // [esp+0h] [ebp-4h]

    if (!Dvar_IsValidName(dvarName))
        MyAssertHandler(".\\qcommon\\com_storage.cpp", 282, 0, "%s", "Dvar_IsValidName( dvarName )");
    localizedText = SEH_LocalizeTextMessage(preLocalizedText, "dvar string", LOCMSG_NOERR);
    if (localizedText && *localizedText)
        Dvar_SetCommand(dvarName, localizedText);
    else
        Dvar_SetCommand(dvarName, preLocalizedText);
}

void __cdecl LiveStorage_ReadStats()
{
    LiveStorage_ReadStatsFromDir((char *)fs_gameDirVar->current.integer);
}

void __cdecl LiveStorage_ReadStatsFromDir(char *directory)
{
    char path[268]; // [esp+0h] [ebp-2238h] BYREF
    int v2; // [esp+10Ch] [ebp-212Ch]
    StatsFile buffer; // [esp+110h] [ebp-2128h] BYREF

    statData.statsFetched = 0;
    if (Com_HasPlayerProfile())
    {
        if (directory && *directory)
            Com_BuildPlayerProfilePath(path, 260, "%s/%s", directory, "mpdata");
        else
            Com_BuildPlayerProfilePath(path, 260, "mpdata");
        if (LiveStorage_ReadStatsFile(path, buffer.magic, 0x211Cu))
        {
            if (!LiveStorage_DecryptAndCheck(&buffer, directory))
                LiveStorage_HandleCorruptStats(path);
            memcpy(statData.playerStats, buffer.body.statsData.stats, sizeof(statData.playerStats));
            v2 = LiveStorage_ChecksumGamerStats(&statData.playerStats[4], 8188);
            statData.statsFetched = 1;
            statData.statWriteNeeded = 0;
            if (*(unsigned int *)statData.playerStats != v2)
                LiveStorage_HandleCorruptStats(path);
            if (!stat_version)
                MyAssertHandler(".\\win32\\win_storage.cpp", 300, 0, "%s", "stat_version");
            if (LiveStorage_GetStat(0, 299) != stat_version->current.integer)
            {
                LiveStorage_NoStatsFound();
                Com_SetErrorMessage((char*)"MENU_RESETCUSTOMCLASSES");
            }
        }
        else
        {
            LiveStorage_NoStatsFound();
        }
    }
}

void __cdecl xxtea_enc(unsigned int *v, unsigned int n, const unsigned int *k)
{
    unsigned int e; // [esp+4h] [ebp-18h]
    unsigned int z; // [esp+8h] [ebp-14h]
    unsigned int sum; // [esp+Ch] [ebp-10h]
    unsigned int q; // [esp+10h] [ebp-Ch]
    unsigned int p; // [esp+18h] [ebp-4h]

    if (n <= 1)
        MyAssertHandler(".\\win32\\win_storage.cpp", 74, 0, "%s\n\t(n) = %i", "(n > 1)", n);
    z = v[n - 1];
    sum = 0;
    q = 0x34 / n + 6;
    while (q--)
    {
        sum -= 1640531527;
        e = (sum >> 2) & 3;
        for (p = 0; p < n - 1; ++p)
        {
            v[p] += ((z ^ k[e ^ p & 3]) + (v[p + 1] ^ sum)) ^ (((16 * z) ^ (v[p + 1] >> 3)) + ((4 * v[p + 1]) ^ (z >> 5)));
            z = v[p];
        }
        v[n - 1] += ((z ^ k[e ^ p & 3]) + (*v ^ sum)) ^ (((16 * z) ^ (*v >> 3)) + ((4 * *v) ^ (z >> 5)));
        z = v[n - 1];
    }
}

void __cdecl xxtea_dec(unsigned int *v, unsigned int n, const unsigned int *k)
{
    unsigned int e; // [esp+4h] [ebp-18h]
    unsigned int sum; // [esp+Ch] [ebp-10h]
    unsigned int y; // [esp+14h] [ebp-8h]
    unsigned int p; // [esp+18h] [ebp-4h]

    if (n <= 1)
        MyAssertHandler(".\\win32\\win_storage.cpp", 99, 0, "%s\n\t(n) = %i", "(n > 1)", n);
    y = *v;
    for (sum = -1640531527 * (0x34 / n + 6); sum; sum += 1640531527)
    {
        e = (sum >> 2) & 3;
        for (p = n - 1; p; --p)
        {
            v[p] -= ((v[p - 1] ^ k[e ^ p & 3]) + (y ^ sum)) ^ (((16 * v[p - 1]) ^ (y >> 3)) + ((4 * y) ^ (v[p - 1] >> 5)));
            y = v[p];
        }
        *v -= ((v[n - 1] ^ k[e]) + (y ^ sum)) ^ (((16 * v[n - 1]) ^ (y >> 3)) + ((4 * y) ^ (v[n - 1] >> 5)));
        y = *v;
    }
}

bool __cdecl LiveStorage_DecryptAndCheck(StatsFile *statsFile, const char *statsDir)
{
    unsigned int hash[4]; // [esp+64h] [ebp-20h] BYREF
    unsigned int key[4]; // [esp+74h] [ebp-10h] BYREF

    if (*(unsigned int *)statsFile->magic != *(unsigned int *)"iwm0")
        return 0;
    LiveStorage_GetCryptKey(statsFile->nonce, (unsigned __int8 *)key);
    xxtea_dec(statsFile->body.hash, 0x845u, key);
    Com_BlockChecksum128(
        (unsigned __int8 *)&statsFile->body.statsData,
        0x2104u,
        statsFile->nonce ^ (key[2] - 1836222900),
        (unsigned __int8 *)hash);
    if (memcmp(hash, &statsFile->body, 0x10u))
        return 0;
    if (statsDir)
        return I_stricmp(statsDir, statsFile->body.statsData.path) == 0;
    return I_stricmp("", statsFile->body.statsData.path) == 0;
}

void __cdecl LiveStorage_GetCryptKey(unsigned int nonce, unsigned __int8 *outKey)
{
    unsigned int hashR[4]; // [esp+0h] [ebp-44h] BYREF
    unsigned int keyHash[4]; // [esp+10h] [ebp-34h] BYREF
    unsigned int ipad[4]; // [esp+20h] [ebp-24h] BYREF
    unsigned int opad[4]; // [esp+30h] [ebp-14h] BYREF
    int wordIndex; // [esp+40h] [ebp-4h]

    Com_BlockChecksum128((unsigned __int8 *)cl_cdkey, 0x22u, 529771271, (unsigned __int8 *)keyHash);
    for (wordIndex = 0; wordIndex != 4; ++wordIndex)
    {
        ipad[wordIndex] = keyHash[wordIndex] ^ 0x36363636;
        opad[wordIndex] = keyHash[wordIndex] ^ 0x5C5C5C5C;
    }
    Com_BlockChecksum128Cat((unsigned __int8 *)ipad, 0x10u, (unsigned __int8 *)&nonce, 4u, (unsigned __int8 *)hashR);
    Com_BlockChecksum128Cat((unsigned __int8 *)opad, 0x10u, (unsigned __int8 *)hashR, 0x10u, outKey);
}

int __cdecl LiveStorage_ChecksumGamerStats(unsigned __int8 *buffer, int len)
{
    return Com_BlockChecksumKey32(buffer, len, 0);
}

void LiveStorage_NoStatsFound()
{
    int v0; // eax
    unsigned int unsignedInt; // [esp-4h] [ebp-4h]

    memset(statData.playerStats, 0, sizeof(statData.playerStats));
    LiveStorage_WriteChecksumToBuffer(statData.playerStats, 0x2000);
    statData.statsFetched = 1;
    Com_Printf(16, "No stats found, zeroing out stats buffer\n");
    LiveStorage_StatsInit(0);
    if (!stat_version)
        MyAssertHandler(".\\win32\\win_storage.cpp", 208, 0, "%s", "stat_version");
    unsignedInt = stat_version->current.unsignedInt;
    v0 = CL_ControllerIndexFromClientNum(0);
    LiveStorage_SetStat(v0, 299, unsignedInt);
}

void __cdecl LiveStorage_WriteChecksumToBuffer(unsigned __int8 *buffer, int len)
{
    *(unsigned int *)buffer = LiveStorage_ChecksumGamerStats(buffer + 4, len - 4);
}

bool __cdecl LiveStorage_ReadStatsFile(const char *qpath, unsigned __int8 *buffer, unsigned int lenToRead)
{
    unsigned int len; // [esp+0h] [ebp-8h]
    unsigned int lena; // [esp+0h] [ebp-8h]
    int h; // [esp+4h] [ebp-4h] BYREF

    FS_CheckFileSystemStarted();
    if (!qpath && !qpath[0])
        MyAssertHandler(".\\win32\\win_storage.cpp", 220, 0, "%s", "qpath || qpath[0]");
    len = FS_FOpenFileRead(qpath, &h);
    if (h && len == lenToRead)
    {
        lena = FS_Read(buffer, lenToRead, h);
        FS_FCloseFile(h);
        return lena == lenToRead;
    }
    else
    {
        FS_FCloseFile(h);
        return 0;
    }
}

void __cdecl LiveStorage_HandleCorruptStats(char *filename)
{
    char corruptName[276]; // [esp+0h] [ebp-118h] BYREF

    if (!filename)
        MyAssertHandler(".\\win32\\win_storage.cpp", 242, 0, "%s", "filename");
    Com_sprintf(corruptName, 0x10Eu, "%s.%s", filename, "corrupt");
    FS_DeleteInDir(corruptName, "players");
    FS_Rename(filename, "players", corruptName, "players");
    FS_DeleteInDir(filename, "players");
    LiveStorage_NoStatsFound();
    Com_Error(ERR_DROP, "PLATFORM_STATSREADERROR");
}

playerStatNetworkData *__cdecl LiveStorage_GetStatBuffer()
{
    if (!statData.statsFetched)
        MyAssertHandler(".\\win32\\win_storage.cpp", 311, 0, "%s", "statData.statsFetched");
    return &statData;
}

bool __cdecl LiveStorage_DoWeHaveStats()
{
    return statData.statsFetched;
}

void __cdecl LiveStorage_StatsWriteNeeded()
{
    statData.statWriteNeeded = 1;
}

void __cdecl LiveStorage_UploadStats()
{
    char path[264]; // [esp+0h] [ebp-2230h] BYREF
    StatsFile statsFile; // [esp+108h] [ebp-2128h] BYREF
    int v2; // [esp+222Ch] [ebp-4h]

    if (statData.statsFetched && statData.statWriteNeeded)
    {
        LiveStorage_WriteChecksumToBuffer(statData.playerStats, 0x2000);
        if (Com_HasPlayerProfile())
        {
            if (*(_BYTE *)fs_gameDirVar->current.integer)
                Com_BuildPlayerProfilePath(path, 260, "%s/%s", fs_gameDirVar->current.string, "mpdata");
            else
                Com_BuildPlayerProfilePath(path, 260, "mpdata");
            memcpy(statsFile.body.statsData.stats, statData.playerStats, sizeof(statsFile.body.statsData.stats));
            I_strncpyz(statsFile.body.statsData.path, (char *)fs_gameDirVar->current.integer, 260);
            LiveStorage_Encrypt(&statsFile);
            v2 = FS_WriteFileToDir(path, "players", (char *)&statsFile, 0x211Cu);
            if (!LiveStorage_DecryptAndCheck(&statsFile, fs_gameDirVar->current.string))
                MyAssertHandler(
                    ".\\win32\\win_storage.cpp",
                    359,
                    0,
                    "%s",
                    "LiveStorage_DecryptAndCheck( &statsFile, fs_gameDirVar->current.string )");
            if (v2)
            {
                statData.statWriteNeeded = 0;
                Com_Printf(16, "Successfully wrote stats data\n");
            }
            else
            {
                Com_Printf(16, "Unable to write stats: %s.\n", path);
            }
        }
    }
}

void __cdecl LiveStorage_Encrypt(StatsFile *statsFile)
{
    unsigned int key[4]; // [esp+5Ch] [ebp-10h] BYREF

    *(unsigned int *)statsFile->magic = *(unsigned int *)"iwm0";
    statsFile->nonce = timeGetTime();
    LiveStorage_GetCryptKey(statsFile->nonce, (unsigned __int8 *)key);
    Com_BlockChecksum128(
        (unsigned __int8 *)&statsFile->body.statsData,
        0x2104u,
        statsFile->nonce ^ (key[2] - 1836222900),
        (unsigned __int8 *)&statsFile->body);
    xxtea_enc(statsFile->body.hash, 0x845u, key);
}

int __cdecl LiveStorage_GetStat(int __formal, int index)
{
    const char *v3; // eax

    if ((unsigned int)index > 0xDAA)
        MyAssertHandler(".\\win32\\win_storage.cpp", 375, 0, "%s\n\t(index) = %i", "(index >= 0 && index < 3499)", index);
    if (!statData.statsFetched)
        return 0;
    if (index < 2000)
        return *(unsigned __int8 *)(index + 231835788);
    if (index < 3498)
        return *(unsigned int *)&statData.playerStats[4 * index - 5996];
    if (!alwaysfails)
    {
        v3 = va("Unhandled stat index %i", index);
        MyAssertHandler(".\\win32\\win_storage.cpp", 393, 0, v3);
    }
    return 0;
}

void __cdecl LiveStorage_SetStat(int __formal, int index, unsigned int value)
{
    const char *v3; // eax

    if ((unsigned int)index > 0xDAA)
        MyAssertHandler(".\\win32\\win_storage.cpp", 403, 0, "%s\n\t(index) = %i", "(index >= 0 && index < 3499)", index);
    if (!statData.statsFetched)
    {
        Com_Printf(14, "Tried to set stat index %i before we have obtained player stats\n", index);
        return;
    }
    if (index >= 2000)
    {
        if (index >= 3498)
        {
            if (!alwaysfails)
            {
                v3 = va("Unhandled stat index %i", index);
                MyAssertHandler(".\\win32\\win_storage.cpp", 445, 0, v3);
            }
        }
        else
        {
            if (!debugStats)
                MyAssertHandler(".\\win32\\win_storage.cpp", 434, 0, "%s", "debugStats");
            if (debugStats->current.enabled)
                Com_Printf(14, "Setting stat %i from %i to %i\n", index, *(unsigned int *)&g_debugPacket[0][4 * index + 6884], value);
            if (*(unsigned int *)&g_debugPacket[0][4 * index + 6884] != value)
            {
                *(unsigned int *)&g_debugPacket[0][4 * index + 6884] = value;
                goto LABEL_24;
            }
        }
    }
    else
    {
        if (value >= 0x100)
        {
            CL_DumpReliableCommands(0);
            Com_Error(
                ERR_SERVERDISCONNECT,
                "Trying to set index %i (which is a byte value) to invalid value %i",
                index,
                value);
        }
        if (!debugStats)
            MyAssertHandler(".\\win32\\win_storage.cpp", 420, 0, "%s", "debugStats");
        if (debugStats->current.enabled)
            Com_Printf(14, "Setting stat %i from %i to %i\n", index, *(unsigned __int8 *)(index + 231835788), value);
        if (*(unsigned __int8 *)(index + 231835788) != value)
        {
            *(_BYTE *)(index + 231835788) = value;
        LABEL_24:
            LiveStorage_StatsWriteNeeded();
        }
    }
}

void __cdecl LiveStorage_TrySetStatForCmd(int index, unsigned int value)
{
    if (LiveStorage_GetStat(0, index) != value)
    {
        LiveStorage_SetStat(0, index, value);
        LiveStorage_StatsWriteNeeded();
    }
}

void __cdecl LiveStorage_NewUser()
{
    memset(statData.playerStats, 0, sizeof(statData));
}

cmd_function_s LiveStorage_StatSetCmd_VAR;
cmd_function_s LiveStorage_StatGetInDvarCmd_VAR;
cmd_function_s LiveStorage_UploadStatsCmd_VAR;
cmd_function_s LiveStorage_ReadStatsCmd_VAR;
cmd_function_s LiveStorage_StatGetCmd_VAR;

void __cdecl LiveStorage_Init()
{
    memset(statData.playerStats, 0, sizeof(statData));
    Cmd_AddCommandInternal("statSet", LiveStorage_StatSetCmd, &LiveStorage_StatSetCmd_VAR);
    Cmd_AddCommandInternal("statGetInDvar", LiveStorage_StatGetInDvarCmd, &LiveStorage_StatGetInDvarCmd_VAR);
    Cmd_AddCommandInternal("uploadStats", LiveStorage_UploadStatsCmd, &LiveStorage_UploadStatsCmd_VAR);
    Cmd_AddCommandInternal("readStats", LiveStorage_ReadStatsCmd, &LiveStorage_ReadStatsCmd_VAR);
    Cmd_AddCommandInternal("statGet", LiveStorage_StatGetCmd, &LiveStorage_StatGetCmd_VAR);
    debugStats = Dvar_RegisterBool("debugStats", 0, 0, "Print messages showing when persistent stats are set");
    stat_version = Dvar_RegisterInt("stat_version", 10, (DvarLimits)0xFF00000000LL, 0, "Stats version number");
}

void __cdecl LiveStorage_StatSetCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int index; // [esp+0h] [ebp-8h]
    int value; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        v0 = Cmd_Argv(1);
        index = atoi(v0);
        v1 = Cmd_Argv(2);
        value = atoi(v1);
        LiveStorage_ValidateSetStatCmd(index, value);
        LiveStorage_TrySetStatForCmd(index, value);
    }
    else
    {
        Com_PrintError(15, "statset usage: statset <index> <value>\n");
    }
}

void __cdecl LiveStorage_StatGetCmd()
{
    const char *v0; // eax
    int Stat; // eax
    int index; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        v0 = Cmd_Argv(1);
        index = atoi(v0);
        Stat = LiveStorage_GetStat(0, index);
        Com_Printf(16, "Stat %i: %i\n", index, Stat);
    }
    else
    {
        Com_PrintError(15, "statget usage: statget <index>\n");
    }
}

void __cdecl LiveStorage_StatGetInDvarCmd()
{
    const char *v0; // eax
    char *v1; // eax
    int Stat; // eax
    char dvarName[1024]; // [esp+0h] [ebp-408h] BYREF
    int index; // [esp+404h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        v0 = Cmd_Argv(1);
        index = atoi(v0);
        v1 = (char *)Cmd_Argv(2);
        I_strncpyz(dvarName, v1, 1024);
        Stat = LiveStorage_GetStat(0, index);
        Dvar_SetIntByName(dvarName, Stat);
    }
    else
    {
        Com_PrintError(15, "statgetindvar usage: statgetindvar <index> <dvar>\n");
    }
}

void __cdecl LiveStorage_UploadStatsCmd()
{
    LiveStorage_UploadStats();
}

void __cdecl LiveStorage_ReadStatsCmd()
{
    LiveStorage_ReadStats();
}

