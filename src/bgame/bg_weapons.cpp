#include "bg_public.h"
#include "bg_local.h"
#include <database/database.h>
#include <qcommon/mem_track.h>
#include <universal/surfaceflags.h>
#include <aim_assist/aim_assist.h>
#include <xanim/xanim.h>
#include <universal/com_files.h>

#ifdef KISAK_MP
#include <game_mp/g_main_mp.h>
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <game/g_local.h>
#endif

//struct WeaponDef **bg_weaponDefs 82800908     bg_weapons.obj
//float (*)[29] penetrationDepthTable 82800f10     bg_weapons.obj
//unsigned int bg_lastParsedWeaponIndex 828010e4     bg_weapons.obj

int surfaceTypeSoundListCount;
WeaponDef *bg_weaponDefs[128];

const float MY_RELOADSTART_INTERUPT_IGNORE_FRAC = 0.4f;

WeaponDef *bg_weapAmmoTypes[128];
WeaponDef *bg_sharedAmmoCaps[128];
WeaponDef *bg_weapClips[128];
unsigned int bg_numAmmoTypes;
unsigned int bg_numSharedAmmoCaps;
unsigned int bg_numWeapClips;

bool penetrationDepthTableLoaded;
float penetrationDepthTable[4][29];

unsigned int bg_lastParsedWeaponIndex;

void __cdecl TRACK_bg_weapons()
{
    track_static_alloc_internal(bg_weapAmmoTypes, 512, "bg_weapAmmoTypes", 9);
    track_static_alloc_internal(bg_sharedAmmoCaps, 512, "bg_sharedAmmoCaps", 9);
    track_static_alloc_internal(bg_weapClips, 512, "bg_weapClips", 9);
}

void __cdecl BG_LoadPenetrationDepthTable()
{
    char *buffer; // [esp+0h] [ebp-200Ch]
    char loadBuffer[8196]; // [esp+4h] [ebp-2008h] BYREF

    if (!penetrationDepthTableLoaded)
    {
        buffer = Com_LoadInfoString(
            (char*)"info/bullet_penetration_mp",
            "bullet penetration table",
            "BULLET_PEN_TABLE",
            loadBuffer);
        Com_Memset((unsigned int *)penetrationDepthTable, 0, 464);
        BG_ParsePenetrationDepthTable("small", penetrationDepthTable[1], buffer);
        BG_ParsePenetrationDepthTable("medium", penetrationDepthTable[2], buffer);
        BG_ParsePenetrationDepthTable("large", penetrationDepthTable[3], buffer);
        penetrationDepthTableLoaded = 1;
    }
}

void __cdecl BG_ParsePenetrationDepthTable(const char *penetrateType, float *depthTable, char *buffer)
{
    const char *v3; // eax
    const char *v4; // eax
    int iTypeIndex; // [esp+0h] [ebp-1E6Ch]
    char dest[7428]; // [esp+4h] [ebp-1E68h] BYREF
    cspField_t pFieldList[29]; // [esp+1D0Ch] [ebp-160h] BYREF

    if (!penetrateType)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 90, 0, "%s", "penetrateType");
    if (!depthTable)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 91, 0, "%s", "depthTable");
    if (!buffer)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 92, 0, "%s", "buffer");
    for (iTypeIndex = 0; iTypeIndex < 29; ++iTypeIndex)
    {
        v3 = Com_SurfaceTypeToName(iTypeIndex);
        if (Com_sprintf(&dest[256 * iTypeIndex], 0x100u, "%s_%s", penetrateType, v3) < 0)
        {
            v4 = Com_SurfaceTypeToName(iTypeIndex);
            Com_Error(ERR_DROP, "Bullet penetration table param name [%s_%s] is to long.", penetrateType, v4);
        }
        pFieldList[iTypeIndex].szName = &dest[256 * iTypeIndex];
        pFieldList[iTypeIndex].iOffset = 4 * iTypeIndex;
        pFieldList[iTypeIndex].iFieldType = 6;
    }
    if (!ParseConfigStringToStruct((unsigned __int8 *)depthTable, pFieldList, 29, buffer, 0, 0, BG_StringCopy))
        Com_Error(ERR_DROP, "Error parsing bullet penetration table [%s].", penetrateType);
}

char __cdecl BG_AdvanceTrace(BulletFireParams *bp, BulletTraceResults *br, float dist)
{
    float offset; // [esp+14h] [ebp-8h]
    float offseta; // [esp+14h] [ebp-8h]
    float dot; // [esp+18h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 134, 0, "%s", "bp");
    if (!br)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 135, 0, "%s", "br");
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 136, 0, "%s", "br->trace.hitType != TRACE_HITTYPE_NONE");
    bp->ignoreEntIndex = Trace_GetEntityHitId(&br->trace);
    if (bp->ignoreEntIndex == ENTITYNUM_WORLD && dist > 0.0)
    {
        dot = -Vec3Dot(br->trace.normal, bp->dir);
        if (dot < 0.125)
        {
            offset = dist / 0.125;
            Vec3Mad(br->hitPos, offset, bp->dir, bp->start);
            return 0;
        }
        offseta = dist / dot;
        Vec3Mad(br->hitPos, offseta, bp->dir, bp->start);
    }
    else
    {
        bp->start[0] = br->hitPos[0];
        bp->start[1] = br->hitPos[1];
        bp->start[2] = br->hitPos[2];
    }
    return 1;
}

double __cdecl BG_GetSurfacePenetrationDepth(const WeaponDef *weapDef, uint32_t surfaceType)
{
    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 166, 0, "%s", "weapDef");
    if (weapDef->penetrateType == PENETRATE_TYPE_NONE)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 167, 0, "%s", "weapDef->penetrateType != PENETRATE_TYPE_NONE");
    if (weapDef->penetrateType >= (uint32_t)PENETRATE_TYPE_COUNT)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            168,
            0,
            "weapDef->penetrateType doesn't index PENETRATE_TYPE_COUNT\n\t%i not in [0, %i)",
            weapDef->penetrateType,
            4);
    if (surfaceType >= 0x1D)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            169,
            0,
            "surfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            surfaceType,
            29);
    if (surfaceType)
        return penetrationDepthTable[weapDef->penetrateType][surfaceType];
    else
        return 0.0;
}

void __cdecl BG_ClearSurfaceTypeSounds()
{
    surfaceTypeSoundListCount = 0;
}

void __cdecl BG_FreeWeaponDefStrings()
{
    unsigned int j; // [esp+0h] [ebp-Ch]
    unsigned int ja; // [esp+0h] [ebp-Ch]
    unsigned int i; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    for (i = 1; i <= bg_lastParsedWeaponIndex; ++i)
    {
        weapDef = bg_weaponDefs[i];
        if (!weapDef)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 188, 0, "%s", "weapDef");
        for (j = 0; j < 8; ++j)
        {
            if (weapDef->hideTags[j])
                SL_RemoveRefToString(weapDef->hideTags[j]);
        }
        for (ja = 0; ja < 0x10; ++ja)
        {
            if (weapDef->notetrackSoundMapKeys[ja])
                SL_RemoveRefToString(weapDef->notetrackSoundMapKeys[ja]);
            if (weapDef->notetrackSoundMapValues[ja])
                SL_RemoveRefToString(weapDef->notetrackSoundMapValues[ja]);
        }
    }
}

void __cdecl BG_ShutdownWeaponDefFiles()
{
    if (*(_BYTE *)fs_gameDirVar->current.integer)
    {
        BG_ClearSurfaceTypeSounds();
        BG_FreeWeaponDefStrings();
    }
    bg_lastParsedWeaponIndex = 0;
}

void __cdecl BG_ClearWeaponDef()
{
    int itemIdx; // [esp+0h] [ebp-4h]

    iassert(bg_lastParsedWeaponIndex == 0);

    bg_weaponDefs[0] = BG_LoadDefaultWeaponDef();
    bg_weapAmmoTypes[0] = bg_weaponDefs[0];
    bg_numAmmoTypes = 1;
    bg_sharedAmmoCaps[0] = bg_weaponDefs[0];
    bg_numSharedAmmoCaps = 1;
    bg_weapClips[0] = bg_weaponDefs[0];
    bg_numWeapClips = 1;
    for (itemIdx = 1; itemIdx < 2048; ++itemIdx)
        bg_itemlist[itemIdx].giType = IT_BAD;
    BG_LoadPlayerAnimTypes();
#ifdef KISAK_MP
    BG_InitWeaponStrings();
#endif
}

void __cdecl BG_FillInAllWeaponItems()
{
    uint32_t weaponIndex; // [esp+0h] [ebp-8h]
    uint32_t weaponCount; // [esp+4h] [ebp-4h]

    weaponCount = BG_GetNumWeapons();
    for (weaponIndex = 1; weaponIndex < weaponCount; ++weaponIndex)
        BG_SetupWeaponIndex(weaponIndex);
}

void __cdecl BG_SetupWeaponIndex(uint32_t weapIndex)
{
    BG_SetupAmmoIndexes(weapIndex);
    BG_SetupSharedAmmoIndexes(weapIndex);
    BG_SetupClipIndexes(weapIndex);
    BG_FillInWeaponItems(weapIndex);
}

void __cdecl BG_FillInWeaponItems(uint32_t weapIndex)
{
    int32_t model; // [esp+0h] [ebp-8h]

    for (model = 0; model < 16; ++model)
        bg_itemlist[128 * model + weapIndex].giType = IT_WEAPON;
}

void __cdecl BG_SetupAmmoIndexes(uint32_t weapIndex)
{
    uint32_t index; // [esp+14h] [ebp-8h]
    WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    for (index = 0; index < bg_numAmmoTypes; ++index)
    {
        if (!strcmp(bg_weapAmmoTypes[index]->szAmmoName, weapDef->szAmmoName))
        {
            weapDef->iAmmoIndex = index;
            return;
        }
    }
    bg_weapAmmoTypes[index] = weapDef;
    weapDef->iAmmoIndex = index;
    ++bg_numAmmoTypes;
}

void __cdecl BG_SetupSharedAmmoIndexes(uint32_t weapIndex)
{
    uint32_t otherWeapIndex; // [esp+0h] [ebp-10h]
    WeaponDef *otherWeapDef; // [esp+4h] [ebp-Ch]
    uint32_t index; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    weapDef->iSharedAmmoCapIndex = -1;
    if (*weapDef->szSharedAmmoCapName)
    {
        Com_DPrintf(17, "%s: %s\n", weapDef->szInternalName, weapDef->szSharedAmmoCapName);
        for (index = 0; ; ++index)
        {
            if (index >= bg_numSharedAmmoCaps)
            {
                bg_sharedAmmoCaps[index] = weapDef;
                weapDef->iSharedAmmoCapIndex = index;
                ++bg_numSharedAmmoCaps;
                return;
            }
            if (!I_stricmp(bg_sharedAmmoCaps[index]->szSharedAmmoCapName, weapDef->szSharedAmmoCapName))
                break;
        }
        weapDef->iSharedAmmoCapIndex = index;
        if (bg_sharedAmmoCaps[index]->iSharedAmmoCap != weapDef->iSharedAmmoCap && index)
        {
            for (otherWeapIndex = 1; otherWeapIndex < weapIndex; ++otherWeapIndex)
            {
                otherWeapDef = bg_weaponDefs[otherWeapIndex];
                if (!I_stricmp(bg_sharedAmmoCaps[index]->szSharedAmmoCapName, otherWeapDef->szSharedAmmoCapName)
                    && otherWeapDef->iSharedAmmoCap == bg_sharedAmmoCaps[index]->iSharedAmmoCap)
                {
                    Com_Error(
                        ERR_DROP,
                        "Shared ammo cap mismatch for %s shared ammo cap: %s set it to %i, but '%s' already set it to %i.",
                        weapDef->szSharedAmmoCapName,
                        weapDef->szInternalName,
                        weapDef->iSharedAmmoCap,
                        otherWeapDef->szInternalName,
                        otherWeapDef->iSharedAmmoCap);
                }
            }
            if (!alwaysfails)
                MyAssertHandler(".\\bgame\\bg_weapons.cpp", 317, 0, "unreachable");
        }
    }
}

void __cdecl BG_SetupClipIndexes(uint32_t weapIndex)
{
    uint32_t index; // [esp+14h] [ebp-8h]
    WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    for (index = 0; index < bg_numWeapClips; ++index)
    {
        if (!strcmp(bg_weapClips[index]->szClipName, weapDef->szClipName))
        {
            weapDef->iClipIndex = index;
            return;
        }
    }
    bg_weapClips[index] = weapDef;
    weapDef->iClipIndex = index;
    ++bg_numWeapClips;
}

void __cdecl PM_StartWeaponAnim(playerState_s *ps, int32_t anim)
{
    if (ps->pm_type < PM_DEAD)
        ps->weapAnim = anim | ps->weapAnim & 0x200 ^ 0x200;
}

WeaponDef *__cdecl BG_GetWeaponDef(uint32_t weaponIndex)
{
    if (weaponIndex > bg_lastParsedWeaponIndex)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            479,
            0,
            "weaponIndex not in [0, bg_lastParsedWeaponIndex]\n\t%i not in [%i, %i]",
            weaponIndex,
            0,
            bg_lastParsedWeaponIndex);
    return bg_weaponDefs[weaponIndex];
}

unsigned int __cdecl BG_GetWeaponIndex(const WeaponDef *weapDef)
{
    unsigned int weapIndex; // [esp+0h] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 490, 0, "%s", "weapDef");
    for (weapIndex = 0; weapIndex <= bg_lastParsedWeaponIndex; ++weapIndex)
    {
        if (weapDef == bg_weaponDefs[weapIndex])
            return weapIndex;
    }
    return 0;
}

unsigned int __cdecl BG_GetNumWeapons()
{
    return bg_lastParsedWeaponIndex + 1;
}

int32_t __cdecl BG_GetSharedAmmoCapSize(uint32_t capIndex)
{
    if (capIndex >= bg_numSharedAmmoCaps)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            523,
            0,
            "capIndex doesn't index bg_numSharedAmmoCaps\n\t%i not in [0, %i)",
            capIndex,
            bg_numSharedAmmoCaps);
    return bg_sharedAmmoCaps[capIndex]->iSharedAmmoCap;
}

unsigned int __cdecl BG_FindWeaponIndexForName(const char *name)
{
    unsigned int weapIndex; // [esp+0h] [ebp-4h]

    if (!name)
        return 0;
    for (weapIndex = 1; weapIndex <= bg_lastParsedWeaponIndex; ++weapIndex)
    {
        if (!I_stricmp(name, bg_weaponDefs[weapIndex]->szInternalName))
            return weapIndex;
    }
    return 0;
}

uint32_t __cdecl BG_GetWeaponIndexForName(const char *name, void(__cdecl *regWeap)(uint32_t))
{
    uint32_t weapIndex; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (!*name || !I_stricmp(name, "none"))
        return 0;
    weapIndex = BG_FindWeaponIndexForName(name);
    if (weapIndex)
        return weapIndex;
    weapDef = BG_LoadWeaponDef(name);
    if (weapDef)
    {
        if (useFastFile->current.enabled
            && (DB_FindXAssetHeader(ASSET_TYPE_WEAPON, name), DB_IsXAssetDefault(ASSET_TYPE_WEAPON, name)))
        {
            return 0;
        }
        else
        {
            return BG_SetupWeaponDef(weapDef, regWeap);
        }
    }
    else
    {
        Com_DPrintf(17, "Couldn't find weapon \"%s\"\n", name);
        return 0;
    }
}

uint32_t __cdecl BG_SetupWeaponDef(WeaponDef *weapDef, void(__cdecl *regWeap)(uint32_t))
{
    uint32_t weapIndex; // [esp+0h] [ebp-4h]

    if (++bg_lastParsedWeaponIndex >= 0x80)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            419,
            0,
            "bg_lastParsedWeaponIndex doesn't index ARRAY_COUNT( bg_weaponDefs )\n\t%i not in [0, %i)",
            bg_lastParsedWeaponIndex,
            128);
    weapIndex = bg_lastParsedWeaponIndex;
    bg_weaponDefs[bg_lastParsedWeaponIndex] = weapDef;
    BG_SetupWeaponIndex(weapIndex);
    BG_SetupWeaponAlts(weapIndex, regWeap);
    if (regWeap)
        regWeap(weapIndex);
    return weapIndex;
}

void __cdecl BG_SetupWeaponAlts(uint32_t weapIndex, void(__cdecl *regWeap)(uint32_t))
{
    int32_t altWeaponIndex; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    weapDef->altWeaponIndex = 0;
    if (*weapDef->szAltWeaponName)
    {
        altWeaponIndex = BG_GetWeaponIndexForName(weapDef->szAltWeaponName, regWeap);
        if (!altWeaponIndex)
            Com_Error(ERR_DROP, "could not find altWeapon %s for weapon %s", weapDef->szAltWeaponName, weapDef->szInternalName);
        weapDef->altWeaponIndex = altWeaponIndex;
    }
}

unsigned int __cdecl BG_GetViewmodelWeaponIndex(const playerState_s *ps)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    if ((ps->weapFlags & 2) == 0)
        return ps->weapon;
    weapIndex = ps->offHandIndex;
    if (!weapIndex)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 652, 0, "%s", "weapIndex != WP_NONE");
    return weapIndex;
}

int32_t __cdecl BG_GetFirstAvailableOffhand(const playerState_s *ps, int32_t offhandClass)
{
    int32_t weapCount; // [esp+0h] [ebp-Ch]
    int32_t weapIndex; // [esp+4h] [ebp-8h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 670, 0, "%s", "ps");
    weapCount = BG_GetNumWeapons();
    for (weapIndex = 1; weapIndex < weapCount; ++weapIndex)
    {
        if (BG_GetWeaponDef(weapIndex)->offhandClass == offhandClass)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, weapIndex, 16)
                && (ps->throwBackGrenadeTimeLeft > 0 || BG_WeaponAmmo(ps, weapIndex) > 0))
            {
                BG_AssertOffhandIndexOrNone(weapIndex);
                return weapIndex;
            }
        }
    }
    return 0;
}

int32_t __cdecl BG_GetFirstEquippedOffhand(const playerState_s *ps, int32_t offhandClass)
{
    int32_t weapCount; // [esp+0h] [ebp-Ch]
    int32_t weapIndex; // [esp+4h] [ebp-8h]

    weapCount = BG_GetNumWeapons();
    for (weapIndex = 1; weapIndex < weapCount; ++weapIndex)
    {
        if (BG_GetWeaponDef(weapIndex)->offhandClass == offhandClass)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, weapIndex, 16))
                return weapIndex;
        }
    }
    return 0;
}

int32_t __cdecl BG_IsAimDownSightWeapon(uint32_t weaponIndex)
{
    return BG_GetWeaponDef(weaponIndex)->aimDownSight;
}

bool __cdecl BG_CanPlayerHaveWeapon(uint32_t weaponIndex)
{
    return BG_GetWeaponDef(weaponIndex)->gunXModel != 0;
}

bool __cdecl BG_ValidateWeaponNumber(uint32_t weaponIndex)
{
    return weaponIndex < BG_GetNumWeapons();
}

bool __cdecl BG_IsWeaponValid(const playerState_s *ps, uint32_t weaponIndex)
{
    if (!BG_ValidateWeaponNumber(weaponIndex))
        return 0;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    return Com_BitCheckAssert(ps->weapons, weaponIndex, 16);
}

bool __cdecl BG_WeaponBlocksProne(uint32_t weapIndex)
{
    return BG_GetWeaponDef(weapIndex)->blocksProne != 0;
}

int32_t __cdecl BG_TakePlayerWeapon(playerState_s *ps, uint32_t weaponIndex, int32_t takeAwayAmmo)
{
    int32_t v4; // esi
    uint32_t curWeaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->weapons, weaponIndex, 16))
        return 0;
    weapDef = BG_GetWeaponDef(weaponIndex);
    Com_BitClearAssert(ps->weapons, weaponIndex, 16);
    if (takeAwayAmmo)
    {
        v4 = AmmoAfterWeaponRemoved(ps, weaponIndex);
        ps->ammo[BG_AmmoForWeapon(weaponIndex)] = v4;
        ps->ammoclip[BG_ClipForWeapon(weaponIndex)] = 0;
    }
    for (curWeaponIndex = weapDef->altWeaponIndex;
        curWeaponIndex;
        curWeaponIndex = BG_GetWeaponDef(curWeaponIndex)->altWeaponIndex)
    {
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (!Com_BitCheckAssert(ps->weapons, curWeaponIndex, 16))
            break;
        if (takeAwayAmmo)
        {
            ps->ammo[BG_AmmoForWeapon(curWeaponIndex)] = 0;
            ps->ammoclip[BG_ClipForWeapon(curWeaponIndex)] = 0;
        }
        Com_BitClearAssert(ps->weapons, curWeaponIndex, 16);
    }
    if (weaponIndex == ps->weapon)
        ps->weapon = 0;
    return 1;
}

int32_t __cdecl AmmoAfterWeaponRemoved(const playerState_s *ps, uint32_t weaponIndex)
{
    int32_t result; // [esp+4h] [ebp-8h]
    int32_t maxAfterRemoval; // [esp+8h] [ebp-4h]

    maxAfterRemoval = BG_GetAmmoPlayerMax(ps, weaponIndex, weaponIndex);
    if (!maxAfterRemoval)
        return 0;
    result = ps->ammo[BG_AmmoForWeapon(weaponIndex)];
    if (result > maxAfterRemoval)
        return maxAfterRemoval;
    return result;
}

int32_t __cdecl BG_GetAmmoPlayerMax(const playerState_s *ps, uint32_t weaponIndex, uint32_t weaponIndexToSkip)
{
    WeaponDef *thisWeapDef; // [esp+0h] [ebp-10h]
    int32_t total; // [esp+4h] [ebp-Ch]
    uint32_t thisWeapIdx; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->iSharedAmmoCapIndex >= 0)
        return BG_GetSharedAmmoCapSize(weapDef->iSharedAmmoCapIndex);
    if (BG_WeaponIsClipOnly(weaponIndex))
        return weapDef->iClipSize;
    total = 0;
    for (thisWeapIdx = 0; thisWeapIdx <= bg_lastParsedWeaponIndex; ++thisWeapIdx)
    {
        if (thisWeapIdx != weaponIndexToSkip)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, thisWeapIdx, 16))
            {
                thisWeapDef = BG_GetWeaponDef(thisWeapIdx);
                if (thisWeapDef->iAmmoIndex == weapDef->iAmmoIndex)
                {
                    if (thisWeapDef->iSharedAmmoCapIndex >= 0)
                        return BG_GetSharedAmmoCapSize(thisWeapDef->iSharedAmmoCapIndex);
                    total += thisWeapDef->iMaxAmmo;
                }
            }
        }
    }
    return total;
}

int32_t __cdecl BG_GetMaxPickupableAmmo(const playerState_s *ps, uint32_t weaponIndex)
{
    int32_t ammo; // [esp+4h] [ebp-418h]
    int32_t ammoIndex; // [esp+8h] [ebp-414h]
    int32_t clipCounted[128]; // [esp+Ch] [ebp-410h] BYREF
    int32_t clipIndex; // [esp+20Ch] [ebp-210h]
    WeaponDef *curWeapDef; // [esp+210h] [ebp-20Ch]
    uint32_t currWeap; // [esp+214h] [ebp-208h]
    WeaponDef *weapDef; // [esp+218h] [ebp-204h]
    int32_t ammoCounted[128]; // [esp+21Ch] [ebp-200h] BYREF

    memset((unsigned __int8 *)ammoCounted, 0, sizeof(ammoCounted));
    memset((unsigned __int8 *)clipCounted, 0, sizeof(clipCounted));
    weapDef = BG_GetWeaponDef(weaponIndex);
    ammoIndex = BG_AmmoForWeapon(weaponIndex);
    clipIndex = BG_ClipForWeapon(weaponIndex);
    if (weapDef->iSharedAmmoCapIndex >= 0)
    {
        ammo = BG_GetSharedAmmoCapSize(weapDef->iSharedAmmoCapIndex);
        for (currWeap = 1; currWeap <= bg_lastParsedWeaponIndex; ++currWeap)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, currWeap, 16))
            {
                curWeapDef = BG_GetWeaponDef(currWeap);
                if (curWeapDef->iSharedAmmoCapIndex == weapDef->iSharedAmmoCapIndex)
                {
                    if (BG_WeaponIsClipOnly(currWeap))
                    {
                        if (!clipCounted[BG_ClipForWeapon(currWeap)])
                        {
                            clipCounted[BG_ClipForWeapon(currWeap)] = 1;
                            ammo -= ps->ammoclip[BG_ClipForWeapon(currWeap)];
                        }
                    }
                    else if (!ammoCounted[BG_AmmoForWeapon(currWeap)])
                    {
                        ammoCounted[BG_AmmoForWeapon(currWeap)] = 1;
                        ammo -= ps->ammo[BG_AmmoForWeapon(currWeap)];
                    }
                }
            }
        }
        return ammo;
    }
    else if (BG_WeaponIsClipOnly(weaponIndex))
    {
        return weapDef->iClipSize - ps->ammoclip[clipIndex];
    }
    else
    {
        return BG_GetAmmoPlayerMax(ps, weaponIndex, 0) - ps->ammo[ammoIndex];
    }
}

int32_t __cdecl BG_GetTotalAmmoReserve(const playerState_s *ps, uint32_t weaponIndex)
{
    int32_t ammo; // [esp+0h] [ebp-418h]
    int32_t ammoIndex; // [esp+4h] [ebp-414h]
    int32_t clipCounted[128]; // [esp+8h] [ebp-410h] BYREF
    int32_t clipIndex; // [esp+208h] [ebp-210h]
    WeaponDef *curWeapDef; // [esp+20Ch] [ebp-20Ch]
    uint32_t currWeap; // [esp+210h] [ebp-208h]
    WeaponDef *weapDef; // [esp+214h] [ebp-204h]
    int32_t ammoCounted[128]; // [esp+218h] [ebp-200h] BYREF

    ammo = 0;
    ammoIndex = BG_AmmoForWeapon(weaponIndex);
    clipIndex = BG_ClipForWeapon(weaponIndex);
    memset((unsigned __int8 *)ammoCounted, 0, sizeof(ammoCounted));
    memset((unsigned __int8 *)clipCounted, 0, sizeof(clipCounted));
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->iSharedAmmoCapIndex < 0)
    {
        if (BG_WeaponIsClipOnly(weaponIndex))
            return ps->ammoclip[clipIndex];
        else
            return ps->ammo[ammoIndex];
    }
    else
    {
        for (currWeap = 1; currWeap <= bg_lastParsedWeaponIndex; ++currWeap)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, currWeap, 16))
            {
                curWeapDef = BG_GetWeaponDef(currWeap);
                if (curWeapDef->iSharedAmmoCapIndex == weapDef->iSharedAmmoCapIndex)
                {
                    if (BG_WeaponIsClipOnly(currWeap))
                    {
                        if (!clipCounted[BG_ClipForWeapon(currWeap)])
                        {
                            clipCounted[BG_ClipForWeapon(currWeap)] = 1;
                            ammo += ps->ammoclip[BG_ClipForWeapon(currWeap)];
                        }
                    }
                    else if (!ammoCounted[BG_AmmoForWeapon(currWeap)])
                    {
                        ammoCounted[BG_AmmoForWeapon(currWeap)] = 1;
                        ammo += ps->ammo[BG_AmmoForWeapon(currWeap)];
                    }
                }
            }
        }
    }
    return ammo;
}

void __cdecl BG_GetSpreadForWeapon(
    const playerState_s *ps,
    const WeaponDef *weapDef,
    float *minSpread,
    float *maxSpread)
{
    double v4; // st7
    float frac; // [esp+4h] [ebp-4h]
    float fraca; // [esp+4h] [ebp-4h]

    if (ps->spreadOverrideState == 2)
    {
        *minSpread = (float)ps->spreadOverride;
        *maxSpread = (float)ps->spreadOverride;
    }
    else
    {
        if (ps->viewHeightCurrent <= 40.0)
        {
            fraca = (ps->viewHeightCurrent - 11.0) / 29.0;
            *minSpread = (weapDef->fHipSpreadDuckedMin - weapDef->fHipSpreadProneMin) * fraca + weapDef->fHipSpreadProneMin;
            v4 = (weapDef->hipSpreadDuckedMax - weapDef->hipSpreadProneMax) * fraca + weapDef->hipSpreadProneMax;
        }
        else
        {
            frac = (ps->viewHeightCurrent - 40.0) / 20.0;
            *minSpread = (weapDef->fHipSpreadStandMin - weapDef->fHipSpreadDuckedMin) * frac + weapDef->fHipSpreadDuckedMin;
            v4 = (weapDef->hipSpreadStandMax - weapDef->hipSpreadDuckedMax) * frac + weapDef->hipSpreadDuckedMax;
        }
        *maxSpread = v4;
    }
    if (ps->spreadOverrideState == 1)
        *maxSpread = (float)ps->spreadOverride;

#ifdef KISAK_MP
    if ((ps->perks & 2) != 0)
    {
        *minSpread = *minSpread * perk_weapSpreadMultiplier->current.value;
        *maxSpread = *maxSpread * perk_weapSpreadMultiplier->current.value;
    }
#endif
}

void __cdecl PM_UpdateAimDownSightFlag(pmove_t *pm, pml_t *pml)
{
    bool adsRequested; // [esp+2h] [ebp-Eh]
    bool adsAllowed; // [esp+3h] [ebp-Dh]
    int weapIndex; // [esp+4h] [ebp-Ch]
    playerState_s *ps; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1126, 0, "%s", "ps");
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    ps->pm_flags &= ~0x10u;
    adsAllowed = PM_IsAdsAllowed(ps, pml);
    adsRequested = (pm->cmd.buttons & 0x800) != 0;
    if ((pm->cmd.buttons & 2) != 0
        && (weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE || (pm->cmd.buttons & 0x2000) == 0))
    {
        PM_ExitAimDownSight(ps);
        adsAllowed = 0;
    }
    if (adsRequested && adsAllowed)
    {
        if ((ps->pm_flags & 1) == 0 || BG_UsingSniperScope(ps))
        {
            ps->pm_flags |= 0x10u;
            if ((ps->otherFlags & 4) == 0)
                MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1161, 0, "%s", "ps->otherFlags & POF_PLAYER");
        }
        else if ((pm->oldcmd.buttons & 0x800) == 0 || !pm->cmd.forwardmove && !pm->cmd.rightmove)
        {
            ps->pm_flags |= 0x10u;
            ps->pm_flags |= 0x200u;
            if ((ps->otherFlags & 4) == 0)
                MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1153, 0, "%s", "ps->otherFlags & POF_PLAYER");
        }
    }
#ifdef KISAK_MP
    if ((ps->pm_flags & 0x10) != 0)
        BG_SetConditionValue(ps->clientNum, 7u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 7u, 0);
#endif
}

bool __cdecl PM_IsAdsAllowed(playerState_s *ps, pml_t *pml)
{
    bool result; // al
    int weapIndex; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1046, 0, "%s", "ps");
    if (!pml)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1047, 0, "%s", "pml");
    switch (ps->pm_type)
    {
    case 1:
        if (!pml->almostGroundPlane)
            goto LABEL_10;
        result = 0;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
    case 8:
        result = 0;
        break;
    default:
    LABEL_10:
        if ((ps->otherFlags & 4) != 0)
        {
            weapIndex = BG_GetViewmodelWeaponIndex(ps);
            weapDef = BG_GetWeaponDef(weapIndex);
            if (weapDef->aimDownSight)
            {
                if (ps->weaponstate < 15 || ps->weaponstate > 20)
                {
                    switch (ps->weaponstate)
                    {
                    case 0xC:
                    case 0xD:
                    case 0xE:
                        result = 0;
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        result = 0;
                        break;
                    case 0x19:
                    case 0x1A:
                        result = 0;
                        break;
                    default:
                        result = (ps->eFlags & 0x300) == 0
                            && (ps->weapFlags & 0x20) == 0
                            && (!weapDef->noAdsWhenMagEmpty || ps->ammoclip[BG_ClipForWeapon(ps->weapon)]);
                        break;
                    }
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }
        }
        else
        {
            result = 0;
        }
        break;
    }
    return result;
}

void __cdecl PM_ExitAimDownSight(playerState_s *ps)
{
    PM_AddEvent(ps, 0xEu);
    ps->pm_flags &= ~0x10u;
}

void __cdecl PM_UpdateAimDownSightLerp(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float fWeaponPosFrac; // [esp+10h] [ebp-1Ch]
    float v7; // [esp+14h] [ebp-18h]
    bool adsRequested; // [esp+1Bh] [ebp-11h]
    int weapIndex; // [esp+1Ch] [ebp-10h]
    WeaponDef *weapDef; // [esp+20h] [ebp-Ch]
    playerState_s *ps; // [esp+24h] [ebp-8h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1191, 0, "%s", "ps");
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (player_scopeExitOnDamage->current.enabled && ps->damageCount && weapDef->overlayReticle)
    {
        PM_ExitAimDownSight(ps);
        ps->fWeaponPosFrac = 0.0;
        ps->adsDelayTime = 0;
    }
    else if (weapDef->aimDownSight && (ps->eFlags & 0x300) == 0)
    {
        adsRequested = 0;
        if (!weapDef->bSegmentedReload && ps->weaponstate == 7 && ps->weaponTime - weapDef->iPositionReloadTransTime > 0
            || weapDef->bSegmentedReload
            && (ps->weaponstate == 7
                || ps->weaponstate == 8
                || ps->weaponstate == 9
                || ps->weaponstate == 10
                || ps->weaponstate == 11 && ps->weaponTime - weapDef->iPositionReloadTransTime > 0)
            || !weapDef->bRechamberWhileAds && ps->weaponstate == 6)
        {
            adsRequested = 0;
        }
        else if ((ps->pm_flags & 0x10) != 0)
        {
            adsRequested = 1;
        }
        if (weapDef->adsFireOnly && ps->weaponDelay && ps->weaponstate == 5)
            adsRequested = 1;
        if (ps->fWeaponPosFrac != 1.0 || adsRequested || player_adsExitDelay->current.integer <= 0)
        {
            ps->adsDelayTime = 0;
        }
        else
        {
            if (!ps->adsDelayTime)
                ps->adsDelayTime = player_adsExitDelay->current.integer + pm->cmd.serverTime;
            if (ps->adsDelayTime <= pm->cmd.serverTime)
                ps->adsDelayTime = 0;
            else
                adsRequested = 1;
        }
        if (adsRequested && ps->fWeaponPosFrac != 1.0 || !adsRequested && ps->fWeaponPosFrac != 0.0)
        {
            if (adsRequested)
                v2 = (double)pml->msec * weapDef->fOOPosAnimLength[0] + ps->fWeaponPosFrac;
            else
                v2 = ps->fWeaponPosFrac - (double)pml->msec * weapDef->fOOPosAnimLength[1];
            ps->fWeaponPosFrac = v2;
            fWeaponPosFrac = ps->fWeaponPosFrac;
            v5 = fWeaponPosFrac - 1.0;
            if (v5 < 0.0)
                v7 = fWeaponPosFrac;
            else
                v7 = 1.0;
            v4 = 0.0 - fWeaponPosFrac;
            if (v4 < 0.0)
                v3 = v7;
            else
                v3 = 0.0;
            ps->fWeaponPosFrac = v3;
        }
    }
    else
    {
        ps->fWeaponPosFrac = 0.0;
        ps->adsDelayTime = 0;
    }
}

bool __cdecl BG_UsingSniperScope(playerState_s *ps)
{
    return BG_GetWeaponDef(ps->weapon)->overlayReticle && ps->fWeaponPosFrac > 0.0;
}

int __cdecl PM_InteruptWeaponWithProneMove(playerState_s *ps)
{
    if (ps->weaponstate <= 4u
        || ps->weaponstate == 7
        || ps->weaponstate == 9
        || ps->weaponstate == 11
        || ps->weaponstate == 10
        || ps->weaponstate == 8
        || ps->weaponstate == 6)
    {
        return 1;
    }
    if (ps->weaponstate == 5
        || ps->weaponstate == 12
        || ps->weaponstate == 13
        || ps->weaponstate == 14
        || ps->weaponstate >= 15 && ps->weaponstate <= 20
        || ps->weaponstate == 25
        || ps->weaponstate == 26)
    {
        return 0;
    }
    PM_Weapon_Idle(ps);
    return 1;
}

int32_t __cdecl BG_ClipForWeapon(uint32_t weapon)
{
    return BG_GetWeaponDef(weapon)->iClipIndex;
}

int32_t __cdecl BG_AmmoForWeapon(uint32_t weapon)
{
    return BG_GetWeaponDef(weapon)->iAmmoIndex;
}

int32_t __cdecl BG_WeaponIsClipOnly(uint32_t weapon)
{
    return BG_GetWeaponDef(weapon)->bClipOnly;
}

int32_t __cdecl BG_WeaponAmmo(const playerState_s *ps, uint32_t weapon)
{
    int32_t ammoIndex; // [esp+0h] [ebp-8h]

    ammoIndex = BG_AmmoForWeapon(weapon);
    return ps->ammoclip[BG_ClipForWeapon(weapon)] + ps->ammo[ammoIndex];
}

int __cdecl PM_WeaponAmmoAvailable(playerState_s *ps)
{
    return ps->ammoclip[BG_ClipForWeapon(ps->weapon)];
}

void __cdecl PM_AdjustAimSpreadScale(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    float v3; // [esp+8h] [ebp-58h]
    float v4; // [esp+Ch] [ebp-54h]
    float v5; // [esp+10h] [ebp-50h]
    float a1; // [esp+14h] [ebp-4Ch]
    float a2; // [esp+18h] [ebp-48h]
    float v8; // [esp+2Ch] [ebp-34h]
    float spreadOverrideScale; // [esp+34h] [ebp-2Ch]
    float wpnScale; // [esp+38h] [ebp-28h]
    int i; // [esp+44h] [ebp-1Ch]
    int ia; // [esp+44h] [ebp-1Ch]
    WeaponDef *weapDef; // [esp+48h] [ebp-18h]
    playerState_s *ps; // [esp+4Ch] [ebp-14h]
    float increase; // [esp+50h] [ebp-10h]
    float speedSquared; // [esp+54h] [ebp-Ch]
    float decrease; // [esp+58h] [ebp-8h]
    float viewchange; // [esp+5Ch] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2363, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    spreadOverrideScale = 1.0;
    wpnScale = weapDef->fHipSpreadDecayRate;
    if (wpnScale == 0.0)
    {
        increase = 0.0;
        decrease = 1.0;
    }
    else
    {
        spreadOverrideScale = ((double)ps->spreadOverride - weapDef->fHipSpreadStandMin)
            / (weapDef->hipSpreadStandMax - weapDef->fHipSpreadStandMin);
        if (ps->groundEntityNum != ENTITYNUM_NONE || ps->pm_type == PM_NORMAL_LINKED)
        {
            if ((ps->eFlags & 8) != 0)
            {
                wpnScale = wpnScale * weapDef->fHipSpreadProneDecay;
                spreadOverrideScale = ((double)ps->spreadOverride - weapDef->fHipSpreadProneMin)
                    / (weapDef->hipSpreadProneMax - weapDef->fHipSpreadProneMin);
            }
            else if ((ps->eFlags & 4) != 0)
            {
                wpnScale = wpnScale * weapDef->fHipSpreadDuckedDecay;
                spreadOverrideScale = ((double)ps->spreadOverride - weapDef->fHipSpreadDuckedMin)
                    / (weapDef->hipSpreadDuckedMax - weapDef->fHipSpreadDuckedMin);
            }
        }
        else
        {
            wpnScale = wpnScale * 0.5;
        }
        if (ps->spreadOverrideState == 1)
        {
            decrease = wpnScale * pml->frametime / spreadOverrideScale;
            increase = 0.0;
        }
        else
        {
            decrease = wpnScale * pml->frametime;
            if (ps->fWeaponPosFrac == 1.0)
            {
                increase = 0.0;
            }
            else
            {
                viewchange = 0.0;
                if (weapDef->fHipSpreadTurnAdd != 0.0)
                {
                    for (i = 0; i < 2; ++i)
                    {
                        a2 = (double)pm->oldcmd.angles[i] * 0.0054931640625;
                        a1 = (double)pm->cmd.angles[i] * 0.0054931640625;
                        v8 = AngleDelta(a1, a2);
                        v5 = I_fabs(v8);
                        viewchange = v5 * (float)0.0099999998 * weapDef->fHipSpreadTurnAdd / pml->frametime + viewchange;
                    }
                }
                if (weapDef->fHipSpreadMoveAdd != 0.0 && (pm->cmd.forwardmove || pm->cmd.rightmove))
                {
                    speedSquared = ps->velocity[1] * ps->velocity[1] + ps->velocity[0] * ps->velocity[0];
                    v4 = bg_aimSpreadMoveSpeedThreshold->current.value * bg_aimSpreadMoveSpeedThreshold->current.value;
                    if (v4 < (double)speedSquared)
                    {
                        v3 = sqrt(speedSquared);
                        viewchange = weapDef->fHipSpreadMoveAdd * v3 / (double)ps->speed + viewchange;
                    }
                }
                if (ps->groundEntityNum == ENTITYNUM_NONE && ps->pm_type != PM_NORMAL_LINKED)
                {
                    for (ia = 0; ia < 2; ++ia)
                        viewchange = (float)0.0099999998 * 128.0 + viewchange;
                }
                increase = viewchange * pml->frametime;
            }
        }
    }
    if (increase <= 0.0)
        v2 = ps->aimSpreadScale - decrease * 255.0;
    else
        v2 = increase * 255.0 + ps->aimSpreadScale;
    ps->aimSpreadScale = v2;
    if (ps->spreadOverrideState == 1 && ps->aimSpreadScale * spreadOverrideScale < 255.0)
    {
        ps->spreadOverrideState = 0;
        ps->aimSpreadScale = ps->aimSpreadScale * spreadOverrideScale;
    }
    if (ps->aimSpreadScale >= 0.0)
    {
        if (ps->aimSpreadScale > 255.0)
            ps->aimSpreadScale = 255.0;
    }
    else
    {
        ps->aimSpreadScale = 0.0;
    }
}

bool __cdecl ShotLimitReached(playerState_s *ps, WeaponDef *weapDef)
{
    bool result; // al

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2468, 0, "%s", "ps");
    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2469, 0, "%s", "weapDef");
    switch (weapDef->fireType)
    {
    case WEAPON_FIRETYPE_SINGLESHOT:
        if (!ps->weaponShotCount)
            goto LABEL_17;
        result = 1;
        break;
    case WEAPON_FIRETYPE_BURSTFIRE2:
        if (ps->weaponShotCount < 2)
            goto LABEL_17;
        result = 1;
        break;
    case WEAPON_FIRETYPE_BURSTFIRE3:
        if (ps->weaponShotCount < 3)
            goto LABEL_17;
        result = 1;
        break;
    case WEAPON_FIRETYPE_BURSTFIRE4:
        if (ps->weaponShotCount < 4)
            goto LABEL_17;
        result = 1;
        break;
    default:
    LABEL_17:
        result = 0;
        break;
    }
    return result;
}

int32_t __cdecl PM_GetWeaponFireButton(uint32_t weapon)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapon);
    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2752, 0, "%s", "weapDef");
    if (weapDef->weapType == WEAPTYPE_GRENADE && weapDef->hasDetonator)
        return 0x80000;
    else
        return 1;
}

void __cdecl PM_Weapon_Idle(playerState_s *ps)
{
#ifdef KISAK_MP
    ps->weapFlags &= ~2u;
    ps->pm_flags &= ~0x200u;
    if (G_IsServerGameSystem(ps->clientNum))
        Com_Printf(19, "end weapon (idle)\n");
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    ps->weaponstate = 0;
    PM_StartWeaponAnim(ps, 0);
#elif KISAK_SP
    unsigned int v1; // r10
    int pm_type; // r8
    unsigned int v3; // r9

    v1 = ps->weapFlags & 0xFFFFFFFD;
    pm_type = ps->pm_type;
    v3 = ps->pm_flags & 0xFFFFFDFF;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    ps->weapFlags = v1;
    ps->pm_flags = v3;
    ps->weaponstate = 0;
    if (pm_type < 5)
        ps->weapAnim = ~(unsigned __int16)ps->weapAnim & 0x200;
#endif
}

void __cdecl PM_Weapon(pmove_t *pm, pml_t *pml)
{
    const char *v2; // eax
    int delayedAction; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3954, 0, "%s", "ps");
    if (ps->weaponstate != 16
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 17
        && (ps->weapFlags & 2) != 0
        && !alwaysfails)
    {
        v2 = va("PWF_USING_OFFHAND set during non-offhand weapon state [%d]\n", ps->weaponstate);
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3961, 0, v2);
    }
    if (ps->pm_type < PM_DEAD)
    {
        if (!G_ExitAfterConnectPaths() && (ps->pm_flags & 0x400) == 0 && (ps->eFlags & 0x300) == 0)
        {
            PM_UpdateAimDownSightLerp(pm, pml);
            PM_UpdateHoldBreath(pm, pml);
            if (!PM_UpdateGrenadeThrow(ps, pml))
            {
                UpdatePendingTriggerPull(pm);
                delayedAction = PM_Weapon_WeaponTimeAdjust(pm, pml);
                if (!BurstFirePending(ps))
                {
                    PM_Weapon_CheckForNightVision(pm);
                    PM_Weapon_CheckForSprint(pm);
                    PM_Weapon_CheckForOffHand(pm);
                    PM_Weapon_CheckForChangeWeapon(pm);
                    PM_Weapon_CheckForReload(pm);
                    PM_Weapon_CheckForMelee(pm, delayedAction);
                    PM_Weapon_CheckForDetonation(pm);
                    PM_Weapon_CheckForGrenadeThrowCancel(pm);
                }
                if (!PM_Weapon_CheckForRechamber(ps, delayedAction))
                {
                    if ((ps->pm_flags & 1) != 0 && (pm->cmd.forwardmove || pm->cmd.rightmove) && ps->fWeaponPosFrac != 1.0
                        || ps->weaponstate == 12
                        || ps->weaponstate == 13
                        || ps->weaponstate == 14)
                    {
                        ps->aimSpreadScale = 255.0;
                    }
                    if (ps->weaponTime < 0 || ps->weaponDelay < 0)
                        MyAssertHandler(
                            ".\\bgame\\bg_weapons.cpp",
                            4018,
                            0,
                            "%s",
                            "(ps->weaponTime >= 0) && (ps->weaponDelay >= 0)");
                    if (delayedAction || !ps->weaponTime && !ps->weaponDelay)
                    {
                        switch (ps->weaponstate)
                        {
                        case 1:
                        case 2:
                            PM_Weapon_FinishWeaponRaise(ps);
                            break;
                        case 3:
                        case 4:
                            PM_Weapon_FinishWeaponChange(pm, ps->weaponstate == 4);
                            break;
                        case 7:
                        case 8:
                            PM_Weapon_FinishReload(pm, delayedAction);
                            break;
                        case 9:
                        case 0xA:
                            PM_Weapon_FinishReloadStart(pm, delayedAction);
                            break;
                        case 0xB:
                            PM_Weapon_FinishReloadEnd(ps);
                            break;
                        case 0xC:
                            PM_Weapon_MeleeFire(ps);
                            break;
                        case 0xD:
                            PM_Weapon_MeleeEnd(ps);
                            break;
                        case 0xE:
                        case 0x14:
                        case 0x18:
                            PM_Weapon_Idle(ps);
                            break;
                        case 0xF:
                            PM_Weapon_OffHandPrepare(ps);
                            break;
                        case 0x10:
                            PM_Weapon_OffHandHold(ps);
                            break;
                        case 0x11:
                            PM_Weapon_OffHand(pm);
                            break;
                        case 0x12:
                            PM_Weapon_OffHandStart(pm);
                            break;
                        case 0x13:
                            PM_Weapon_OffHandEnd(ps);
                            break;
                        case 0x15:
                            PM_Detonate(ps, delayedAction);
                            break;
                        case 0x16:
                            Sprint_State_Loop(ps);
                            break;
                        case 0x17:
                            return;
                        case 0x19:
                            PM_Weapon_FinishNightVisionWear(ps);
                            break;
                        case 0x1A:
                            PM_Weapon_FinishNightVisionRemove(ps);
                            break;
                        default:
                            if (ps->weapon)
                            {
                                if (PM_Weapon_ShouldBeFiring(pm, delayedAction))
                                {
                                    if (!PM_Weapon_CheckGrenadeHold(pm, delayedAction) && (ps->pm_flags & 0x800) == 0)
                                    {
                                        PM_Weapon_FireWeapon(ps, delayedAction);
                                        if (ps->weaponTime < 0 || ps->weaponDelay < 0)
                                            MyAssertHandler(
                                                ".\\bgame\\bg_weapons.cpp",
                                                4131,
                                                0,
                                                "%s",
                                                "(ps->weaponTime >= 0) && (ps->weaponDelay >= 0)");
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        ps->weapon = 0;
    }
}

void __cdecl PM_UpdateHoldBreath(pmove_t *pm, pml_t *pml)
{
    float lerp; // [esp+18h] [ebp-1Ch]
    float lerpa; // [esp+18h] [ebp-1Ch]
    int breathGaspTime; // [esp+1Ch] [ebp-18h]
    int weapIndex; // [esp+20h] [ebp-14h]
    float targetScale; // [esp+24h] [ebp-10h]
    float targetScalea; // [esp+24h] [ebp-10h]
    int breathHoldTime; // [esp+28h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+2Ch] [ebp-8h]
    playerState_s *ps; // [esp+30h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1313, 0, "%s", "ps");
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    breathHoldTime = (int)(player_breath_hold_time->current.value * 1000.0);
    breathGaspTime = (int)(player_breath_gasp_time->current.value * 1000.0);
#ifdef KISAK_MP
    if ((ps->perks & 0x10) != 0)
        breathHoldTime += (int)(perk_extraBreath->current.value * 1000.0);
#endif
    if (breathHoldTime > 0)
    {
        if (ps->fWeaponPosFrac == 1.0
            && weapDef->overlayReticle
            && weapDef->weapClass != WEAPCLASS_ITEM
            && (pm->cmd.buttons & 0x2000) != 0)
        {
            if (!ps->holdBreathTimer)
                PM_StartHoldBreath(ps);
        }
        else
        {
            PM_EndHoldBreath(ps);
        }
        if ((ps->weapFlags & 4) != 0)
            ps->holdBreathTimer += pml->msec;
        else
            ps->holdBreathTimer -= pml->msec;
        if (ps->holdBreathTimer < 0)
            ps->holdBreathTimer = 0;
        if ((ps->weapFlags & 4) != 0 && ps->holdBreathTimer > breathHoldTime)
        {
            ps->holdBreathTimer = breathGaspTime + breathHoldTime;
            PM_EndHoldBreath(ps);
        }
        if ((ps->weapFlags & 4) != 0)
        {
            targetScale = 0.0;
            lerp = player_breath_hold_lerp->current.value;
        }
        else
        {
            lerpa = (double)ps->holdBreathTimer / (double)(breathGaspTime + breathHoldTime);
            targetScale = (player_breath_gasp_scale->current.value - 1.0) * lerpa + 1.0;
            lerp = player_breath_gasp_lerp->current.value;
        }
        targetScalea = (targetScale - 1.0) * ps->fWeaponPosFrac + 1.0;
        ps->holdBreathScale = DiffTrack(targetScalea, ps->holdBreathScale, lerp, pml->frametime);
    }
    else
    {
        PM_EndHoldBreath(ps);
        ps->holdBreathScale = 1.0;
        ps->holdBreathTimer = 0;
    }
}

void __cdecl PM_StartHoldBreath(playerState_s *ps)
{
    ps->weapFlags |= 4u;
}

void __cdecl PM_EndHoldBreath(playerState_s *ps)
{
    ps->weapFlags &= ~4u;
}

int32_t __cdecl PM_Weapon_CheckForRechamber(playerState_s *ps, int32_t delayedAction)
{
    uint32_t bitNum; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponstate >= 15 && ps->weaponstate <= 20)
        return 0;
    if (weapDef->bBoltAction)
    {
        bitNum = ps->weapon;
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 243, 0, "%s", "ps");
        if (Com_BitCheckAssert(ps->weaponrechamber, bitNum, 16))
        {
            if (ps->weaponstate == 6)
            {
                if (delayedAction)
                {
                    Com_BitClearAssert(ps->weaponrechamber, ps->weapon, 16);
                    PM_AddEvent(ps, 0x1Du);
                    if (ps->weaponTime)
                        return 1;
                }
            }
            if (!ps->weaponTime
                || ps->weaponstate != 5
                && ps->weaponstate != 6
                && ps->weaponstate != 12
                && ps->weaponstate != 13
                && ps->weaponstate != 14
                && !ps->weaponDelay)
            {
                if (ps->weaponstate == 6)
                {
                    PM_Weapon_FinishRechamber(ps);
                }
                else if (!ps->weaponstate)
                {
                    if (ps->fWeaponPosFrac <= 0.75)
                        PM_StartWeaponAnim(ps, 4);
                    else
                        PM_StartWeaponAnim(ps, 7);
                    ps->weaponstate = 6;
                    ps->weaponTime = weapDef->iRechamberTime;
                    if (weapDef->iRechamberBoltTime && weapDef->iRechamberBoltTime < weapDef->iRechamberTime)
                        ps->weaponDelay = weapDef->iRechamberBoltTime;
                    else
                        ps->weaponDelay = 1;
                    PM_AddEvent(ps, 0x1Cu);
                }
            }
        }
    }
    return 0;
}

void __cdecl PM_Weapon_FinishRechamber(playerState_s *ps)
{
    PM_ContinueWeaponAnim(ps, 0);
    ps->weaponstate = 0;
}

void __cdecl PM_ContinueWeaponAnim(playerState_s *ps, int32_t anim)
{
    if ((ps->weapAnim & 0xFFFFFDFF) != anim)
        PM_StartWeaponAnim(ps, anim);
}

void __cdecl PM_Weapon_FinishWeaponChange(pmove_t *pm, bool quick)
{
    bool v2; // [esp+8h] [ebp-34h]
    int weapon; // [esp+Ch] [ebp-30h]
    int bitNum; // [esp+10h] [ebp-2Ch]
    int altswitch; // [esp+18h] [ebp-24h]
    float aimspread; // [esp+1Ch] [ebp-20h]
    unsigned int oldweapon; // [esp+20h] [ebp-1Ch]
    unsigned int anim; // [esp+24h] [ebp-18h]
    unsigned int weapontime; // [esp+28h] [ebp-14h]
    int *weapDef; // [esp+2Ch] [ebp-10h]
    playerState_s *ps; // [esp+30h] [ebp-Ch]
    bool firstequip; // [esp+34h] [ebp-8h]
    unsigned int newweapon; // [esp+38h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1928, 0, "%s", "ps");
    if (ps->weaponstate != 3 && ps->weaponstate != 4)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1930, 0, "%s", "WEAPONSTATE_DROPPING( ps->weaponstate )");
    BG_GetWeaponDef(ps->weapon);
    if (Mantle_IsWeaponInactive(ps))
    {
        newweapon = 0;
    }
    else
    {
        if ((ps->pm_flags & 8) != 0)
            goto LABEL_13;
        bitNum = pm->cmd.weapon;
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(ps->weapons, bitNum, 16))
        {
            if ((ps->weapFlags & 0x80) != 0)
            {
                newweapon = 0;
            }
            else
            {
                newweapon = pm->cmd.weapon;
                if (newweapon >= BG_GetNumWeapons())
                    newweapon = 0;
            }
        }
        else
        {
        LABEL_13:
            newweapon = 0;
        }
    }
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->weapons, newweapon, 16))
        newweapon = 0;
    oldweapon = ps->weapon;
    ps->weapon = (unsigned __int8)newweapon;
    if (ps->weapon != newweapon)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1957, 0, "%s", "ps->weapon == newweapon");
    weapDef = (int *)BG_GetWeaponDef(ps->weapon);
    if (oldweapon == newweapon)
    {
        ps->weaponstate = 0;
        PM_StartWeaponAnim(ps, 0);
    }
    else
    {
        weapon = ps->weapon;
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 236, 0, "%s", "ps");
        firstequip = !Com_BitCheckAssert(ps->weaponold, weapon, 16);
        Com_BitSetAssert(ps->weaponold, ps->weapon, 16);
        if ((ps->pm_flags & 0x8000) == 0 && oldweapon)
        {
            v2 = newweapon && newweapon == BG_GetWeaponDef(oldweapon)->altWeaponIndex;
            altswitch = v2;
        }
        else
        {
            altswitch = 0;
        }
        if (altswitch)
        {
            weapontime = weapDef[236];
            if (ps->aimSpreadScale >= 128.0)
                aimspread = ps->aimSpreadScale;
            else
                aimspread = 128.0;
            anim = 18;
        }
        else
        {
            aimspread = 255.0;
            if (PM_WeaponClipEmpty(ps))
            {
                anim = 22;
                weapontime = weapDef[240];
            }
            else if (firstequip)
            {
                anim = 12;
                weapontime = weapDef[239];
            }
            else if (quick)
            {
                anim = 20;
                weapontime = weapDef[238];
            }
            else
            {
                anim = 11;
                weapontime = weapDef[234];
            }
            if (oldweapon)
            {
                if (firstequip)
                    PM_AddEvent(ps, 0x16u);
                else
                    PM_AddEvent(ps, 0x15u);
#ifdef KISAK_MP
                BG_AnimScriptEvent(ps, ANIM_ET_RAISEWEAPON, 0, 0);
#endif
            }
        }
        PM_Weapon_BeginWeaponRaise(ps, anim, weapontime, aimspread, altswitch);
#ifdef KISAK_MP
        iassert(weapDef);
        BG_SetConditionBit(ps->clientNum, 0, weapDef[74]);
        BG_SetConditionBit(ps->clientNum, 1, weapDef[76]);
#endif
        BG_TakeClipOnlyWeaponIfEmpty(ps, oldweapon);
    }
}

bool __cdecl PM_WeaponClipEmpty(playerState_s *ps)
{
    return ps->ammoclip[BG_ClipForWeapon(ps->weapon)] == 0;
}

void __cdecl PM_Weapon_BeginWeaponRaise(
    playerState_s *ps,
    uint32_t anim,
    uint32_t time,
    float aim,
    int32_t altSwitch)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1869, 0, "%s", "ps");
    if (aim < 0.0)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 1870, 0, "%s", "aim >= 0");
    ps->weaponstate = (altSwitch != 0) + 1;
    ps->weaponTime = time;
    ps->aimSpreadScale = aim;
    PM_SetProneMovementOverride(ps);
    PM_StartWeaponAnim(ps, anim);
}

void __cdecl BG_TakeClipOnlyWeaponIfEmpty(playerState_s *ps, int32_t weaponIndex)
{
    if (weaponIndex)
    {
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(ps->weapons, weaponIndex, 16)
            && BG_WeaponIsClipOnly(weaponIndex)
            && !ps->ammoclip[BG_ClipForWeapon(weaponIndex)]
            && !ps->ammo[BG_AmmoForWeapon(weaponIndex)]
                && !BG_GetWeaponDef(weaponIndex)->hasDetonator)
        {
            BG_TakePlayerWeapon(ps, weaponIndex, 0);
        }
    }
}

void __cdecl PM_Weapon_FinishWeaponRaise(playerState_s *ps)
{
    if (ps->weaponstate != 1 && ps->weaponstate != 2)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2041, 0, "%s", "WEAPONSTATE_RAISING( ps->weaponstate )");
    ps->weaponstate = 0;
    PM_StartWeaponAnim(ps, 0);
}

void __cdecl PM_Weapon_FinishReloadStart(pmove_t *pm, int32_t delayedAction)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2158, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponstate != 9 && ps->weaponstate != 10)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            2161,
            0,
            "%s",
            "(ps->weaponstate == WEAPON_RELOAD_START) || (ps->weaponstate == WEAPON_RELOAD_START_INTERUPT)");
    if (delayedAction)
        PM_Weapon_ReloadDelayedAction(ps);
    if (!ps->weaponTime)
    {
        if (weapDef->bSegmentedReload && (pm->cmd.buttons & 1) != 0)
            ps->weaponstate = 10;
        if (ps->weaponstate == 10 && ps->ammoclip[BG_ClipForWeapon(ps->weapon)] || !PM_Weapon_AllowReload(ps))
        {
            Com_BitClearAssert(ps->weaponrechamber, ps->weapon, 16);
            if (weapDef->iReloadEndTime)
            {
                ps->weaponstate = 11;
                PM_StartWeaponAnim(ps, 16);
                ps->weaponTime = weapDef->iReloadEndTime;
                PM_AddEvent(ps, 0x12u);
            }
            else
            {
                ps->weaponstate = 0;
                PM_StartWeaponAnim(ps, 0);
            }
        }
        else
        {
            PM_SetReloadingState(ps);
        }
    }
}

void __cdecl PM_SetReloadingState(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->ammoclip[BG_ClipForWeapon(ps->weapon)] || weapDef->weapType)
    {
        PM_StartWeaponAnim(ps, 13);
        ps->weaponTime = weapDef->iReloadTime;
        PM_AddEvent(ps, 0xFu);
    }
    else
    {
        PM_StartWeaponAnim(ps, 14);
        ps->weaponTime = weapDef->iReloadEmptyTime;
        PM_AddEvent(ps, 0x10u);
    }
    if (ps->weaponstate == 10)
        ps->weaponstate = 8;
    else
        ps->weaponstate = 7;
    PM_SetWeaponReloadAddAmmoDelay(ps);
}

void __cdecl PM_SetWeaponReloadAddAmmoDelay(playerState_s *ps)
{
    unsigned int bitNum; // [esp+0h] [ebp-10h]
    int reloadTime; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponstate == 9 || ps->weaponstate == 10)
    {
        if (weapDef->iReloadStartAddTime)
        {
            if (weapDef->iReloadStartAddTime >= weapDef->iReloadStartTime)
                reloadTime = weapDef->iReloadStartTime;
            else
                reloadTime = weapDef->iReloadStartAddTime;
        }
        else
        {
            reloadTime = 0;
        }
    }
    else
    {
        if (ps->ammoclip[BG_ClipForWeapon(ps->weapon)] || weapDef->weapType)
            reloadTime = weapDef->iReloadTime;
        else
            reloadTime = weapDef->iReloadEmptyTime;
        if (weapDef->iReloadAddTime && weapDef->iReloadAddTime < reloadTime)
            reloadTime = weapDef->iReloadAddTime;
    }
    if (!weapDef->bBoltAction)
        goto LABEL_26;
    bitNum = ps->weapon;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 243, 0, "%s", "ps");
    if (Com_BitCheckAssert(ps->weaponrechamber, bitNum, 16))
    {
        if (!reloadTime)
            reloadTime = ps->weaponTime;
        if (weapDef->iRechamberBoltTime < reloadTime)
            reloadTime = weapDef->iRechamberBoltTime;
        if (!reloadTime)
            reloadTime = 1;
        ps->weaponDelay = reloadTime;
    }
    else
    {
    LABEL_26:
        if (reloadTime)
            ps->weaponDelay = reloadTime;
    }
}

int __cdecl PM_Weapon_AllowReload(playerState_s *ps)
{
    int clipWeap; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    clipWeap = BG_ClipForWeapon(ps->weapon);
    if (ps->ammo[BG_AmmoForWeapon(ps->weapon)] && ps->ammoclip[clipWeap] < weapDef->iClipSize)
    {
        if (!weapDef->bNoPartialReload)
            return 1;
        if (weapDef->iReloadAmmoAdd && weapDef->iReloadAmmoAdd < weapDef->iClipSize)
        {
            if (weapDef->iClipSize - ps->ammoclip[clipWeap] >= weapDef->iReloadAmmoAdd)
                return 1;
        }
        else if (!ps->ammoclip[clipWeap])
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl PM_Weapon_ReloadDelayedAction(playerState_s *ps)
{
    unsigned int bitNum; // [esp+0h] [ebp-14h]
    int reloadTime; // [esp+8h] [ebp-Ch]
    int reloadTimea; // [esp+8h] [ebp-Ch]
    int rechamberTime; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (!weapDef->bBoltAction)
        goto LABEL_28;
    bitNum = ps->weapon;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 243, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->weaponrechamber, bitNum, 16))
    {
    LABEL_28:
        PM_ReloadClip(ps);
        return;
    }
    Com_BitClearAssert(ps->weaponrechamber, ps->weapon, 16);
    PM_AddEvent(ps, 0x1Du);
    if (ps->weaponstate != 9 && ps->weaponstate != 10 || weapDef->iReloadStartAddTime)
    {
        if (ps->weaponTime)
        {
            if (ps->weaponstate == 9 || ps->weaponstate == 10)
            {
                if (!weapDef->iReloadStartAddTime)
                    MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2107, 0, "%s", "weapDef->iReloadStartAddTime");
                reloadTime = weapDef->iReloadStartAddTime >= weapDef->iReloadStartTime
                    ? weapDef->iReloadStartTime
                    : weapDef->iReloadStartAddTime;
            }
            else
            {
                if (ps->ammoclip[BG_ClipForWeapon(ps->weapon)] || weapDef->weapType)
                    reloadTime = weapDef->iReloadTime;
                else
                    reloadTime = weapDef->iReloadEmptyTime;
                if (weapDef->iReloadAddTime && weapDef->iReloadAddTime < reloadTime)
                    reloadTime = weapDef->iReloadAddTime;
            }
            rechamberTime = weapDef->iRechamberBoltTime >= reloadTime ? 1 : weapDef->iRechamberBoltTime;
            reloadTimea = reloadTime - rechamberTime;
            if (reloadTimea >= 1)
            {
                ps->weaponDelay = reloadTimea;
                return;
            }
        }
        goto LABEL_28;
    }
}

void __cdecl PM_ReloadClip(playerState_s *ps)
{
    int clip; // [esp+0h] [ebp-18h]
    int ammo; // [esp+4h] [ebp-14h]
    int ammoAdd; // [esp+Ch] [ebp-Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponstate != 9 && ps->weaponstate != 10 || weapDef->iReloadStartAdd)
    {
        ammo = BG_AmmoForWeapon(ps->weapon);
        clip = BG_ClipForWeapon(ps->weapon);
        ammoAdd = weapDef->iClipSize - ps->ammoclip[clip];
        if (ammoAdd > ps->ammo[ammo])
            ammoAdd = ps->ammo[ammo];
        if (ps->weaponstate == 9 || ps->weaponstate == 10)
        {
            if (weapDef->iReloadStartAdd < weapDef->iClipSize && ammoAdd > weapDef->iReloadStartAdd)
                ammoAdd = weapDef->iReloadStartAdd;
        }
        else if (weapDef->iReloadAmmoAdd
            && weapDef->iReloadAmmoAdd < weapDef->iClipSize
            && ammoAdd > weapDef->iReloadAmmoAdd)
        {
            ammoAdd = weapDef->iReloadAmmoAdd;
        }
        if (ammoAdd)
        {
            ps->ammo[ammo] -= ammoAdd;
            ps->ammoclip[clip] += ammoAdd;
            PM_AddEvent(ps, 0x14u);
        }
    }
}

void __cdecl PM_Weapon_FinishReload(pmove_t *pm, int32_t delayedAction)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2204, 0, "%s", "ps");
    if (ps->weaponstate != 7 && ps->weaponstate != 8)
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            2206,
            0,
            "%s",
            "(ps->weaponstate == WEAPON_RELOADING) || (ps->weaponstate == WEAPON_RELOADING_INTERUPT)");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (!delayedAction || (PM_Weapon_ReloadDelayedAction(ps), !ps->weaponTime))
    {
        if (!ps->weaponTime)
        {
            if (weapDef->bSegmentedReload && (pm->cmd.buttons & 1) != 0)
                ps->weaponstate = 8;
            Com_BitClearAssert(ps->weaponrechamber, ps->weapon, 16);
            if (!weapDef->bSegmentedReload)
                goto LABEL_19;
            if (ps->weaponstate != 8 && PM_Weapon_AllowReload(ps))
            {
                PM_SetReloadingState(ps);
                return;
            }
            if (weapDef->iReloadEndTime)
            {
                ps->weaponstate = 11;
                PM_StartWeaponAnim(ps, 16);
                ps->weaponTime = weapDef->iReloadEndTime;
                PM_AddEvent(ps, 0x12u);
            }
            else
            {
            LABEL_19:
                ps->weaponstate = 0;
                PM_StartWeaponAnim(ps, 0);
            }
        }
    }
}

void __cdecl PM_Weapon_FinishReloadEnd(playerState_s *ps)
{
    if (ps->weaponstate != 11)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2257, 0, "%s", "ps->weaponstate == WEAPON_RELOAD_END");
    ps->weaponstate = 0;
    PM_StartWeaponAnim(ps, 0);
}

void __cdecl PM_Weapon_CheckForReload(pmove_t *pm)
{
    float frac; // [esp+8h] [ebp-1Ch]
    int doReload; // [esp+Ch] [ebp-18h]
    int clipWeap; // [esp+10h] [ebp-14h]
    int ammoWeap; // [esp+14h] [ebp-10h]
    WeaponDef *weapDef; // [esp+18h] [ebp-Ch]
    playerState_s *ps; // [esp+1Ch] [ebp-8h]
    bool reloadRequested; // [esp+20h] [ebp-4h]

    doReload = 0;
    ps = pm->ps;
    iassert(ps);
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ((ps->weaponstate < 15 || ps->weaponstate > 20)
        && ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14)
    {
        reloadRequested = (pm->cmd.buttons & 0x10) != 0;
        if ((ps->weapFlags & 1) != 0)
        {
            ps->weapFlags &= ~1u;
            reloadRequested = 1;
        }
        if (weapDef->bSegmentedReload
            && (ps->weaponstate == 9 || ps->weaponstate == 7)
            && (pm->cmd.buttons & 1) != 0
            && (pm->oldcmd.buttons & 1) == 0)
        {
            if (ps->weaponstate == 9 && weapDef->iReloadStartTime)
            {
                frac = (double)(weapDef->iReloadStartTime - ps->weaponTime) / (double)weapDef->iReloadStartTime;
                if (MY_RELOADSTART_INTERUPT_IGNORE_FRAC < (double)frac)
                    ps->weaponstate = 10;
            }
            else if (ps->weaponstate == 7)
            {
                ps->weaponstate = 8;
            }
        }
        switch (ps->weaponstate)
        {
        case 1:
        case 2:
        case 3:
        case 4:
            return;
        case 7:
        case 8:
        case 9:
        case 0xA:
        case 0xB:
#ifdef KISAK_MP
            if (pm->proneChange)
            {
                if (!BG_WeaponIsClipOnly(ps->weapon))
                    BG_AnimScriptEvent(ps, ANIM_ET_RELOAD, 0, 1);
            }
#endif
            break;
        default:
            clipWeap = BG_ClipForWeapon(ps->weapon);
            ammoWeap = BG_AmmoForWeapon(ps->weapon);
            if (reloadRequested && PM_Weapon_AllowReload(ps))
                doReload = 1;
            if (!ps->ammoclip[clipWeap]
                && ps->ammo[ammoWeap]
                    && ps->weaponstate != 5
                        && (ps->weaponstate < 22 || ps->weaponstate > 24))
            {
                doReload = 1;
            }
            if (doReload)
                PM_BeginWeaponReload(ps);
            break;
        }
    }
}

void __cdecl PM_BeginWeaponReload(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ((!ps->weaponstate
        || ps->weaponstate == 5
        || ps->weaponstate == 6
        || ps->weaponstate >= 22 && ps->weaponstate <= 24)
        && ps->weapon
        && ps->weapon < BG_GetNumWeapons())
    {
#ifdef KISAK_MP
        if (!BG_WeaponIsClipOnly(ps->weapon))
            BG_AnimScriptEvent(ps, ANIM_ET_RELOAD, 0, 1);
#endif
        ps->weaponShotCount = 0;
        PM_AddEvent(ps, 0xEu);
        PM_AddEvent(ps, 0x13u);
        if (weapDef->bSegmentedReload && weapDef->iReloadStartTime)
        {
            PM_StartWeaponAnim(ps, 15);
            ps->weaponTime = weapDef->iReloadStartTime;
            ps->weaponstate = 9;
            PM_AddEvent(ps, 0x11u);
            PM_SetWeaponReloadAddAmmoDelay(ps);
        }
        else
        {
            PM_SetReloadingState(ps);
        }
    }
}

bool __cdecl BurstFirePending(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2501, 0, "%s", "ps");
    if (!ps->weapon)
        return 0;
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (weapDef->fireType == WEAPON_FIRETYPE_FULLAUTO)
        return 0;
    if (ps->weaponShotCount)
        return !ShotLimitReached(ps, weapDef);
    return 0;
}

void __cdecl UpdatePendingTriggerPull(pmove_t *pm)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2546, 0, "%s", "ps");
    if (BG_GetWeaponDef(ps->weapon)->fireType >= (unsigned int)WEAPON_FIRETYPE_BURSTFIRE2
        && (pm->cmd.buttons & 1) != 0
        && (pm->oldcmd.buttons & 1) == 0)
    {
        ps->weapFlags |= 0x100u;
    }
}

int __cdecl PM_Weapon_WeaponTimeAdjust(pmove_t *pm, pml_t *pml)
{
    bool v3; // [esp+0h] [ebp-74h]
    bool v4; // [esp+4h] [ebp-70h]
    int weaponTime; // [esp+8h] [ebp-6Ch]
    int weaponDelay; // [esp+Ch] [ebp-68h]
    int weaponRestrictKickTime; // [esp+10h] [ebp-64h]
    float v8; // [esp+18h] [ebp-5Ch]
    float v9; // [esp+28h] [ebp-4Ch]
    float v10; // [esp+44h] [ebp-30h]
    int msec; // [esp+68h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+6Ch] [ebp-8h]
    playerState_s *ps; // [esp+70h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2575, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponRestrictKickTime > 0)
    {
        ps->weaponRestrictKickTime -= pml->msec;
        if (ps->weaponRestrictKickTime < 0)
            weaponRestrictKickTime = 0;
        else
            weaponRestrictKickTime = ps->weaponRestrictKickTime;
        ps->weaponRestrictKickTime = weaponRestrictKickTime;
    }
    if ((ps->weaponstate == 7
        || ps->weaponstate == 9
        || ps->weaponstate == 11
        || ps->weaponstate == 10
        || ps->weaponstate == 8)
#ifdef KISAK_MP
        && (ps->perks & 4) != 0)
#elif KISAK_SP
        )
#endif 
    {
        if (perk_weapReloadMultiplier->current.value == 0.0)
        {
            if (ps->weaponTime < ps->weaponDelay)
                weaponDelay = ps->weaponDelay;
            else
                weaponDelay = ps->weaponTime;
            msec = weaponDelay;
        }
        else
        {
            msec = (int)(pml->msec / perk_weapReloadMultiplier->current.value);
        }
    }
    else if ((ps->weaponstate == 5
        || ps->weaponstate == 6
        || ps->weaponstate == 12
        || ps->weaponstate == 13
        || ps->weaponstate == 14)
        && ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14
#ifdef KISAK_MP
        && (ps->perks & 8) != 0)
#elif KISAK_SP
        )
#endif 
    {
        if (perk_weapRateMultiplier->current.value == 0.0)
        {
            if (ps->weaponTime < ps->weaponDelay)
                weaponTime = ps->weaponDelay;
            else
                weaponTime = ps->weaponTime;
            msec = weaponTime;
        }
        else
        {
            msec = (int)(pml->msec / perk_weapRateMultiplier->current.value);
        }
    }
    else
    {
        msec = pml->msec;
    }
    if (ps->weaponTime)
    {
        ps->weaponTime -= msec;
        if (ps->weaponTime <= 0)
        {
            if (ps->weaponstate == 5 && WeaponUsesBurstCooldown(ps->weapon) && !BurstFirePending(ps))
            {
                if (player_burstFireCooldown->current.value == 0.0)
                {
                    ps->weaponTime = 1;
                }
                else
                {
                    ps->weaponTime = (int)(player_burstFireCooldown->current.value * 1000.0f);
                }
                PM_ContinueWeaponAnim(ps, 0);
                ps->weaponstate = 0;
                return 0;
            }
            v4 = (ps->weapFlags & 0x100) == 0 && ShotLimitReached(ps, weapDef);
            v3 = weapDef->weapType == WEAPTYPE_GRENADE && weapDef->holdButtonToThrow;
            if ((ps->weaponstate < 15 || ps->weaponstate > 20)
                && (v4 || v3)
                && (pm->cmd.buttons & 1) != 0
                && ps->weapon == pm->cmd.weapon
                && PM_WeaponAmmoAvailable(ps))
            {
                ps->weaponTime = 1;
                if (ps->weaponstate == 7
                    || ps->weaponstate == 9
                    || ps->weaponstate == 11
                    || ps->weaponstate == 10
                    || ps->weaponstate == 8)
                {
#ifdef KISAK_MP 
                    if (G_IsServerGameSystem(ps->clientNum))
                        Com_Printf(19, "end weapon (timeout)\n");
#endif
                    ps->weaponTime = 0;
                    ps->weaponShotCount = 0;
                }
                else if (ps->weaponstate == 6)
                {
                    PM_Weapon_FinishRechamber(ps);
                }
                else if (ps->weaponstate == 5
                    || ps->weaponstate == 6
                    || ps->weaponstate == 12
                    || ps->weaponstate == 13
                    || ps->weaponstate == 14)
                {
                    PM_ContinueWeaponAnim(ps, 0);
                    ps->weaponstate = 0;
                }
            }
            else
            {
#ifdef KISAK_MP
                if (G_IsServerGameSystem(ps->clientNum))
                    Com_Printf(19, "end weapon (timeout)\n");
#endif
                if (((pm->cmd.buttons & 1) == 0 || (ps->weapFlags & 0x100) != 0) && !BurstFirePending(ps))
                    ps->weaponShotCount = 0;
                ps->weaponTime = 0;
            }
        }
    }
    if (!ps->weaponDelay)
        return 0;
    ps->weaponDelay -= msec;
    if (ps->weaponDelay > 0)
        return 0;
    ps->weaponDelay = 0;
    return 1;
}

bool __cdecl WeaponUsesBurstCooldown(uint32_t weaponIdx)
{
    weapFireType_t fireType; // [esp+0h] [ebp-8h]

    if (!weaponIdx)
        return 0;
    fireType = BG_GetWeaponDef(weaponIdx)->fireType;
    return fireType >= WEAPON_FIRETYPE_BURSTFIRE2 && fireType <= WEAPON_FIRETYPE_BURSTFIRE4;
}

void __cdecl PM_Weapon_CheckForChangeWeapon(pmove_t *pm)
{
    int bitNum; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2698, 0, "%s", "ps");
    if (ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14
        && (ps->weaponstate < 15 || ps->weaponstate > 20)
        && ps->weaponstate != 25
        && ps->weaponstate != 26
        && (!ps->weaponTime
            || ps->weaponstate == 7
            || ps->weaponstate == 9
            || ps->weaponstate == 11
            || ps->weaponstate == 10
            || ps->weaponstate == 8
            || ps->weaponstate == 6
            || ps->weaponstate != 5
            && ps->weaponstate != 6
            && ps->weaponstate != 12
            && ps->weaponstate != 13
            && ps->weaponstate != 14
            && !ps->weaponDelay))
    {
        if (Mantle_IsWeaponInactive(ps))
        {
            if (ps->weapon)
                PM_BeginWeaponChange(ps, 0, 1);
        }
        else if ((ps->pm_flags & 8) != 0)
        {
            if (ps->weapon)
                PM_BeginWeaponChange(ps, 0, 1);
        }
        else if ((ps->weapFlags & 0x80) != 0)
        {
            if (ps->weapon)
                PM_BeginWeaponChange(ps, 0, 0);
        }
        else if (ps->weapon == pm->cmd.weapon
            || (ps->pm_flags & 0xC00) != 0 && ps->weapon
            || pm->cmd.weapon && !BG_IsWeaponValid(ps, pm->cmd.weapon))
        {
            if (ps->weapon == pm->cmd.weapon && (ps->weaponstate == 3 || ps->weaponstate == 4))
            {
                PM_Weapon_Idle(ps);
                PM_StartWeaponAnim(ps, 1);
            }
            else if (ps->weapon)
            {
                bitNum = ps->weapon;
                if (!ps)
                    MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
                if (!Com_BitCheckAssert(ps->weapons, bitNum, 16))
                    PM_BeginWeaponChange(ps, 0, 0);
            }
        }
        else
        {
            PM_BeginWeaponChange(ps, pm->cmd.weapon, (ps->mantleState.flags & 0x10) != 0);
        }
    }
}

void __cdecl PM_BeginWeaponChange(playerState_s *ps, uint32_t newweapon, bool quick)
{
    uint32_t NumWeapons; // eax
    int32_t quickDropTime; // edx
    bool v5; // [esp+0h] [ebp-14h]
    bool noammo; // [esp+4h] [ebp-10h]
    int32_t altswitch; // [esp+8h] [ebp-Ch]
    uint32_t oldweapon; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDefOld; // [esp+10h] [ebp-4h]

    if (newweapon >= BG_GetNumWeapons())
    {
        NumWeapons = BG_GetNumWeapons();
        MyAssertHandler(
            ".\\bgame\\bg_weapons.cpp",
            1783,
            0,
            "newweapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
            newweapon,
            NumWeapons);
    }
    if (!newweapon)
        goto LABEL_8;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(ps->weapons, newweapon, 16))
    {
    LABEL_8:
        if (ps->weaponstate != 3 && ps->weaponstate != 4)
        {
            if (newweapon && BG_GetWeaponDef(newweapon)->weapClass == WEAPCLASS_PISTOL)
                quick = 1;
            if (ps->weaponstate == 7
                || ps->weaponstate == 9
                || ps->weaponstate == 11
                || ps->weaponstate == 10
                || ps->weaponstate == 8)
            {
                BG_AddPredictableEventToPlayerstate(2u, ps->weaponstate, ps);
            }
            ps->weaponDelay = 0;
            oldweapon = ps->weapon;
            if (!oldweapon)
                goto LABEL_55;
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, oldweapon, 16) && ps->grenadeTimeLeft <= 0)
            {
                weapDefOld = BG_GetWeaponDef(oldweapon);
                if ((ps->pm_flags & 0x8000) != 0)
                {
                    altswitch = 0;
                }
                else
                {
                    v5 = newweapon && newweapon == weapDefOld->altWeaponIndex;
                    altswitch = v5;
                }
                noammo = PM_WeaponClipEmpty(ps);
                ps->grenadeTimeLeft = 0;
                if (altswitch)
                {
                    PM_AddEvent(ps, 0x18u);
                    PM_StartWeaponAnim(ps, 17);
                }
                else
                {
                    PM_AddEvent(ps, 0x17u);
                    if ((ps->pm_flags & 0x8000) == 0)
                    {
                        if (noammo)
                        {
                            PM_StartWeaponAnim(ps, 21);
                        }
                        else if (quick)
                        {
                            PM_StartWeaponAnim(ps, 19);
                        }
                        else
                        {
                            PM_StartWeaponAnim(ps, 10);
                        }
                    }
                }
#ifdef KISAK_MP
                if (!altswitch && (ps->pm_flags & 4) == 0)
                    BG_AnimScriptEvent(ps, ANIM_ET_DROPWEAPON, 0, 1);
#endif
                ps->weaponstate = quick + 3;
                PM_SetProneMovementOverride(ps);
                if (altswitch)
                {
                    ps->weaponTime = weapDefOld->iAltDropTime;
                }
                else if (noammo)
                {
                    ps->weaponTime = weapDefOld->iEmptyDropTime;
                }
                else
                {
                    if (quick)
                        quickDropTime = weapDefOld->quickDropTime;
                    else
                        quickDropTime = weapDefOld->iDropTime;
                    ps->weaponTime = quickDropTime;
                }
            }
            else
            {
            LABEL_55:
#ifdef KISAK_MP 
                if (G_IsServerGameSystem(ps->clientNum))
                    Com_Printf(19, "end weapon (begin weapon change)\n");
#endif
                ps->weaponTime = 0;
                ps->weaponstate = quick + 3;
                ps->grenadeTimeLeft = 0;
                PM_SetProneMovementOverride(ps);
            }
        }
    }
}

int32_t __cdecl PM_Weapon_ShouldBeFiring(pmove_t *pm, int32_t delayedAction)
{
    bool v3; // [esp+0h] [ebp-10h]
    bool shouldStartFiring; // [esp+7h] [ebp-9h]
    playerState_s *ps; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2768, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    shouldStartFiring = (pm->cmd.buttons & PM_GetWeaponFireButton(ps->weapon)) != 0;
    if (weapDef->freezeMovementWhenFiring && ps->groundEntityNum == ENTITYNUM_NONE)
        shouldStartFiring = 0;
    v3 = delayedAction || BurstFirePending(ps);
    if (shouldStartFiring || v3)
        return 1;
    if (ps->weaponstate == 5)
        PM_ContinueWeaponAnim(ps, 0);
    ps->weaponstate = 0;
    return 0;
}

void __cdecl PM_Weapon_FireWeapon(playerState_s *ps, int32_t delayedAction)
{
    int32_t LocalClientActiveCount; // eax
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (PM_Weapon_CheckFiringAmmo(ps))
    {
        PM_Weapon_StartFiring(ps, delayedAction);
        if (!ps->weaponDelay)
        {
            if (PM_WeaponAmmoAvailable(ps) != -1 && (ps->eFlags & 0x300) == 0)
            {
#ifndef DEDICATED
                LocalClientActiveCount = CL_GetLocalClientActiveCount();
                PM_WeaponUseAmmo(ps, ps->weapon, LocalClientActiveCount);
#else
                PM_WeaponUseAmmo(ps, ps->weapon, 1);
#endif
            }
            if (weapDef->weapType == WEAPTYPE_GRENADE)
                ps->weaponTime = weapDef->iFireTime;
            PM_Weapon_SetFPSFireAnim(ps);
            if (PM_WeaponClipEmpty(ps))
                PM_AddEvent(ps, 0x1Bu);
            else
                PM_AddEvent(ps, 0x1Au);
            PM_HoldBreathFire(ps);
            PM_Weapon_AddFiringAimSpreadScale(ps);
            BG_SwitchWeaponsIfEmpty(ps);
        }
    }
}

void __cdecl PM_HoldBreathFire(playerState_s *ps)
{
    int weapIndex; // [esp+4h] [ebp-Ch]
    int breathHoldTime; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (ps->fWeaponPosFrac == 1.0 && weapDef->overlayReticle && weapDef->weapClass != WEAPCLASS_ITEM)
    {
        breathHoldTime = (int)(player_breath_hold_time->current.value * 1000.0);
        if (ps->holdBreathTimer < breathHoldTime)
        {
            ps->holdBreathTimer += (int)(player_breath_fire_delay->current.value * 1000.0);
            if (ps->holdBreathTimer > breathHoldTime)
                ps->holdBreathTimer = breathHoldTime;
        }
        PM_EndHoldBreath(ps);
    }
}

void __cdecl PM_WeaponUseAmmo(playerState_s *ps, uint32_t wp, int32_t amount)
{
    int32_t v3; // [esp+0h] [ebp-Ch]
    int32_t idx; // [esp+8h] [ebp-4h]

#if KISAK_SP
    if (!player_sustainAmmo->current.enabled )// || !WeaponValidForSustainAmmoCheat(wp))
#else
    if (!player_sustainAmmo->current.enabled || !CG_ShouldPlaySoundOnLocalClient())
#endif
    {
        idx = BG_ClipForWeapon(wp);
        if (ps->ammoclip[idx] < amount)
            v3 = ps->ammoclip[idx];
        else
            v3 = amount;
        ps->ammoclip[idx] -= v3;
    }
}

void __cdecl BG_SwitchWeaponsIfEmpty(playerState_s *ps)
{
    if (!ps->ammoclip[BG_ClipForWeapon(ps->weapon)]
        && !ps->ammo[BG_AmmoForWeapon(ps->weapon)]
            && !BG_GetWeaponDef(ps->weapon)->hasDetonator)
    {
        PM_AddEvent(ps, 0xBu);
    }
}

void __cdecl PM_Weapon_StartFiring(playerState_s *ps, int32_t delayedAction)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!ps->weapon)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 2802, 0, "%s", "ps->weapon != WP_NONE");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (weapDef->weapType != WEAPTYPE_GRENADE)
    {
        ps->weaponDelay = weapDef->iFireDelay;
        ps->weaponTime = weapDef->iFireTime;
        if (weapDef->adsFireOnly)
            ps->weaponDelay = (int)((1.0 - ps->fWeaponPosFrac) * (1.0 / weapDef->fOOPosAnimLength[0]));
        if (weapDef->bBoltAction)
            Com_BitSetAssert(ps->weaponrechamber, ps->weapon, 16);
        if (ps->weaponstate != 5)
        {
            if (ps->fWeaponPosFrac < 1.0)
                ps->weaponRestrictKickTime = weapDef->iFireDelay + weapDef->hipGunKickReducedKickBullets * weapDef->iFireTime;
            else
                ps->weaponRestrictKickTime = weapDef->iFireDelay + weapDef->adsGunKickReducedKickBullets * weapDef->iFireTime;
        }
        goto LABEL_19;
    }
    if (delayedAction)
    {
    LABEL_19:
#ifdef KISAK_MP
        BG_AnimScriptEvent(ps, ANIM_ET_FIREWEAPON, 0, 1);
#endif
        goto LABEL_20;
    }
    if (PM_WeaponAmmoAvailable(ps))
    {
        ps->grenadeTimeLeft = weapDef->fuseTime;
        PM_StartWeaponAnim(ps, 26);
        BG_AddPredictableEventToPlayerstate(0x19u, ps->weapon, ps);
    }
    ps->weaponDelay = weapDef->iHoldFireTime;
    ps->weaponTime = 0;
#ifdef KISAK_MP
    if (G_IsServerGameSystem(ps->clientNum))
        Com_Printf(19, "end weapon (start fire)\n");
#endif
LABEL_20:
    ps->weaponstate = 5;
    PM_SetProneMovementOverride(ps);
    if (weapDef->fireType)
    {
        if (!ps->weaponShotCount)
            ps->weapFlags &= ~0x100u;
        if (++ps->weaponShotCount > 4)
            ps->weaponShotCount = 4;
    }
}

int __cdecl PM_Weapon_CheckFiringAmmo(playerState_s *ps)
{
    int v1; // eax
    bool reloadingW; // [esp+4h] [ebp-10h]
    int ammoNeeded; // [esp+8h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+Ch] [ebp-8h]

    iassert(ps->weapon != WP_NONE);

    weapDef = BG_GetWeaponDef(ps->weapon);

#ifndef DEDICATED
    ammoNeeded = CL_GetLocalClientActiveCount();
#else
    ammoNeeded = 1;
#endif

    if (ammoNeeded <= PM_WeaponAmmoAvailable(ps))
        return 1;

    v1 = BG_AmmoForWeapon(ps->weapon);

    reloadingW = ammoNeeded <= ps->ammo[v1];
    if (weapDef->weapType != WEAPTYPE_GRENADE && ammoNeeded > ps->ammo[v1])
        PM_AddEvent(ps, 0xBu);

    if (reloadingW)
    {
        PM_BeginWeaponReload(ps);
    }
    else
    {
        Com_BitClearAssert(ps->weaponrechamber, ps->weapon, 16);
        PM_ContinueWeaponAnim(ps, 0);
        if (weapDef->weapType != WEAPTYPE_GRENADE)
            ps->weaponTime += 500;
    }
    return 0;
}

void __cdecl PM_Weapon_SetFPSFireAnim(playerState_s *ps)
{
    if (ps->fWeaponPosFrac <= 0.75)
    {
        if (PM_WeaponClipEmpty(ps))
            PM_StartWeaponAnim(ps, 3);
        else
            PM_StartWeaponAnim(ps, 2);
    }
    else if (PM_WeaponClipEmpty(ps))
    {
        PM_StartWeaponAnim(ps, 6);
    }
    else
    {
        PM_StartWeaponAnim(ps, 5);
    }
}

void __cdecl PM_Weapon_AddFiringAimSpreadScale(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->fWeaponPosFrac != 1.0)
    {
        ps->aimSpreadScale = weapDef->fHipSpreadFireAdd * 255.0 + ps->aimSpreadScale;
        if (ps->aimSpreadScale > 255.0)
            ps->aimSpreadScale = 255.0;
    }
}

void __cdecl PM_Weapon_MeleeEnd(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3067, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (weapDef->knifeModel)
    {
        ps->weaponstate = 14;
        ps->weaponTime = weapDef->quickRaiseTime;
        ps->weaponDelay = 0;
        PM_StartWeaponAnim(ps, 20);
        PM_SetProneMovementOverride(ps);
    }
    else
    {
        PM_Weapon_Idle(ps);
    }
}

void __cdecl PM_Weapon_MeleeFire(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3091, 0, "%s", "ps");
    BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = 13;
    PM_AddEvent(ps, 0x1Fu);
    PM_SetProneMovementOverride(ps);
}

void __cdecl PM_Weapon_CheckForMelee(pmove_t *pm, int32_t delayedAction)
{
    int32_t weaponstate; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3148, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3151, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14
        && (ps->weaponstate < 15 || ps->weaponstate > 20)
        && ps->weaponstate != 25
        && ps->weaponstate != 26
        && weapDef->iMeleeDamage
        && !delayedAction
        && (!ps->weaponDelay
            || ps->weaponstate == 7
            || ps->weaponstate == 9
            || ps->weaponstate == 11
            || ps->weaponstate == 10
            || ps->weaponstate == 8)
        && (pm->cmd.buttons & 4) != 0
        && (pm->oldcmd.buttons & 4) == 0
        && (ps->fWeaponPosFrac <= 0.0 || weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE))
    {
        weaponstate = ps->weaponstate;
        if (weaponstate <= 0 || weaponstate > 4)
        {
            PM_MeleeChargeStart(pm);
            PM_Weapon_MeleeInit(ps);
        }
    }
}

void __cdecl PM_Weapon_MeleeInit(playerState_s *ps)
{
    bool v1; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4h] [ebp-8h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3107, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    v1 = (ps->pm_flags & 0x20000) != 0 && PM_WeaponHasChargeMelee(ps);
    if (v1)
    {
        ps->weaponTime = weapDef->meleeChargeTime;
        ps->weaponDelay = weapDef->meleeChargeDelay;
        PM_StartWeaponAnim(ps, 9);
    }
    else
    {
        ps->weaponTime = weapDef->iMeleeTime;
        ps->weaponDelay = weapDef->iMeleeDelay;
        PM_StartWeaponAnim(ps, 8);
    }
#ifdef KISAK_MP
    if (weapDef->knifeModel && v1)
    {
        BG_AnimScriptEvent(ps, ANIM_ET_KNIFE_MELEE_CHARGE, 0, 1);
    }
    else if (weapDef->knifeModel)
    {
        BG_AnimScriptEvent(ps, ANIM_ET_KNIFE_MELEE, 0, 1);
    }
    else
    {
        BG_AnimScriptEvent(ps, ANIM_ET_MELEEATTACK, 0, 1);
    }
#endif
    ps->weaponstate = 12;
    PM_AddEvent(ps, 0x1Eu);
    PM_SetProneMovementOverride(ps);
}

bool __cdecl PM_WeaponHasChargeMelee(playerState_s *ps)
{
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3055, 0, "%s", "ps");
    weapDef = BG_GetWeaponDef(ps->weapon);
    return weapDef->szXAnims[8] && *weapDef->szXAnims[8] && weapDef->meleeChargeTime > 0;
}

void __cdecl PM_Weapon_OffHandPrepare(playerState_s *ps)
{
    WeaponDef *WeaponDef; // eax

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3261, 0, "%s", "ps");
    if (!ps->offHandIndex)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3262, 0, "%s", "ps->offHandIndex != WP_NONE");
    WeaponDef = BG_GetWeaponDef(ps->offHandIndex);
    ps->weaponstate = 16;
    ps->weaponTime = WeaponDef->iHoldFireTime;
    ps->weaponDelay = 0;
    ps->weapFlags |= 2u;
    if (BG_ThrowingBackGrenade(ps))
    {
        PM_StartWeaponAnim(ps, 18);
    }
    else
    {
        BG_AddPredictableEventToPlayerstate(0x20u, ps->offHandIndex, ps);
        PM_StartWeaponAnim(ps, 26);
    }
    PM_SetProneMovementOverride(ps);
}

void __cdecl PM_Weapon_OffHandHold(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3289, 0, "%s", "ps");
    if (!ps->offHandIndex)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3290, 0, "%s", "ps->offHandIndex != WP_NONE");
    ps->weaponstate = 18;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    ps->weapFlags |= 2u;
    if (!BG_ThrowingBackGrenade(ps))
        ps->grenadeTimeLeft = BG_GetWeaponDef(ps->offHandIndex)->fuseTime;
#ifdef KISAK_MP
    if (G_IsServerGameSystem(ps->clientNum))
        Com_Printf(19, "end weapon (offhand hold)\n");
#endif
}

void __cdecl PM_Weapon_OffHandStart(pmove_t *pm)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    iassert(pm);
    ps = pm->ps;
    iassert(ps);
    iassert(ps->offHandIndex != WP_NONE);
    weapDef = BG_GetWeaponDef(ps->offHandIndex);
    if (!weapDef->holdButtonToThrow && (pm->oldcmd.buttons & 0xC000) != 0 && (pm->cmd.buttons & 0xC000) != 0)
    {
        ps->weaponDelay = 1;
    }
    else
    {
        ps->weaponstate = 17;
        ps->weaponTime = weapDef->iFireTime;
        ps->weaponDelay = weapDef->iFireDelay;
        ps->weapFlags |= 2u;
        PM_StartWeaponAnim(ps, 2);
#ifdef KISAK_MP
        BG_AnimScriptEvent(ps, ANIM_ET_FIREWEAPON, 0, 1);
#endif
    }
}

void __cdecl PM_Weapon_OffHand(pmove_t *pm)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3348, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3352, 0, "%s", "ps");
    if (!ps->offHandIndex)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3353, 0, "%s", "ps->offHandIndex != WP_NONE");
    BG_GetWeaponDef(ps->offHandIndex);
    BG_AddPredictableEventToPlayerstate(0x21u, ps->offHandIndex, ps);
    if (!BG_ThrowingBackGrenade(ps))
    {
        if (BG_WeaponAmmo(ps, ps->offHandIndex))
            PM_WeaponUseAmmo(ps, ps->offHandIndex, 1);
        else
            PM_AddEvent(ps, 0xDu);
    }
    ps->weaponstate = 19;
    ps->weapFlags |= 2u;
}

void __cdecl PM_Weapon_OffHandEnd(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3376, 0, "%s", "ps");
    if (ps->weapon)
    {
        ps->weaponTime = BG_GetWeaponDef(ps->weapon)->quickRaiseTime;
        ps->weaponDelay = 0;
        PM_StartWeaponAnim(ps, 20);
    }
    else
    {
        ps->weaponTime = 0;
        ps->weaponDelay = 1;
#ifdef KISAK_MP
        if (G_IsServerGameSystem(ps->clientNum))
            Com_Printf(19, "end weapon (offhand end)\n");
#endif
    }
    ps->throwBackGrenadeTimeLeft = 0;
    ps->throwBackGrenadeOwner = ENTITYNUM_NONE;
    ps->weaponstate = 20;
    ps->weapFlags &= ~2u;
    ps->pm_flags &= ~0x200u;
}

void __cdecl PM_Weapon_CheckForOffHand(pmove_t *pm)
{
    WeaponDef *pWeapDef; // eax
    const char *v2; // eax
    unsigned int FirstAvailableOffhand; // eax
    WeaponDef *pWeapDef2; // eax
    const char *v5; // eax
    WeaponDef *pWeapDef3; // eax
    int bitNum; // [esp+0h] [ebp-14h]
    WeaponDef *pWeapDef4; // [esp+4h] [ebp-10h]
    playerState_s *ps; // [esp+8h] [ebp-Ch]
    unsigned int offHandIndex; // [esp+Ch] [ebp-8h]
    OffhandClass offhandClass; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3458, 0, "%s", "ps");
    if ((ps->eFlags & 0x300) == 0
        && (ps->weapFlags & 0x80) == 0
        && (ps->pm_flags & 0x8000) == 0
        && !PM_Weapon_IsHoldingGrenade(pm)
        && (ps->weaponstate < 15 || ps->weaponstate >= 20)
        && ps->weaponstate != 25
        && ps->weaponstate != 26)
    {
        if ((ps->weapFlags & 2) != 0)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3481, 0, "%s", "!(ps->weapFlags & PWF_USING_OFFHAND)");
        bitNum = pm->cmd.offHandIndex;
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(ps->weapons, bitNum, 16))
        {
            ps->offHandIndex = pm->cmd.offHandIndex;
            if (ps->offHandIndex)
            {
                if (BG_GetWeaponDef(ps->offHandIndex)->offhandClass == OFFHAND_CLASS_NONE)
                {
                    pWeapDef = BG_GetWeaponDef(ps->offHandIndex);
                    v2 = va("ps->offHandIndex = %d (%s)\n", ps->offHandIndex, pWeapDef->szInternalName);
                    MyAssertHandler(
                        ".\\bgame\\bg_weapons.cpp",
                        3486,
                        0,
                        "%s\n\t%s",
                        "ps->offHandIndex == WP_NONE || BG_GetWeaponDef( ps->offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
                        v2);
                }
            }
        }
        if ((pm->cmd.buttons & 0x4000) != 0)
        {
            offhandClass = OFFHAND_CLASS_FRAG_GRENADE;
            FirstAvailableOffhand = BG_GetFirstAvailableOffhand(ps, 1);
        }
        else
        {
            if ((pm->cmd.buttons & 0x8000) == 0)
                return;
            if (ps->offhandSecondary)
            {
                if (ps->offhandSecondary != PLAYER_OFFHAND_SECONDARY_FLASH)
                    MyAssertHandler(
                        ".\\bgame\\bg_weapons.cpp",
                        3501,
                        0,
                        "%s",
                        "ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH");
                offhandClass = OFFHAND_CLASS_FLASH_GRENADE;
                FirstAvailableOffhand = BG_GetFirstAvailableOffhand(ps, 3);
            }
            else
            {
                offhandClass = OFFHAND_CLASS_SMOKE_GRENADE;
                FirstAvailableOffhand = BG_GetFirstAvailableOffhand(ps, 2);
            }
        }
        offHandIndex = FirstAvailableOffhand;
        if (FirstAvailableOffhand && BG_GetWeaponDef(FirstAvailableOffhand)->offhandClass == OFFHAND_CLASS_NONE)
        {
            pWeapDef2 = BG_GetWeaponDef(offHandIndex);
            v5 = va("offHandIndex = %d (%s)\n", offHandIndex, pWeapDef2->szInternalName);
            MyAssertHandler(
                ".\\bgame\\bg_weapons.cpp",
                3512,
                0,
                "%s\n\t%s",
                "offHandIndex == WP_NONE || BG_GetWeaponDef( offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
                v5);
        }
        if (offHandIndex)
        {
            BG_AddPredictableEventToPlayerstate(0x22u, offHandIndex, ps);
            ps->offHandIndex = offHandIndex;
            pWeapDef3 = BG_GetWeaponDef(ps->offHandIndex);
            pWeapDef4 = pWeapDef3;
            if (pWeapDef3->weapType != WEAPTYPE_GRENADE)
                Com_Error(ERR_DROP, "[%s] Only grenades are currently supported for off hand use\n", pWeapDef3->szInternalName);
            if (pWeapDef4->offhandClass == OFFHAND_CLASS_NONE)
                Com_Error(ERR_DROP, "[%s] No offhand class set\n", pWeapDef4->szInternalName);
            if (ps->cursorHintEntIndex == ENTITYNUM_NONE && (!ps->weapon || ps->weaponstate == 20))
                PM_Weapon_OffHandPrepare(ps);
            else
                PM_Weapon_OffHandInit(ps);
        }
        else
        {
            PM_SendEmtpyOffhandEvent(ps, offhandClass);
        }
    }
}

void __cdecl PM_Weapon_OffHandInit(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3233, 0, "%s", "ps");
    if (!ps->offHandIndex)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3234, 0, "%s", "ps->offHandIndex != WP_NONE");
    ps->weaponstate = 15;
    ps->weaponDelay = 0;
    ps->weapFlags &= ~2u;
    ps->throwBackGrenadeOwner = ENTITYNUM_NONE;
    PM_ExitAimDownSight(ps);
    if (ps->weapon)
    {
        ps->weaponTime = BG_GetWeaponDef(ps->weapon)->quickDropTime;
        PM_StartWeaponAnim(ps, 19);
    }
    else
    {
        ps->weaponTime = 100;
    }
}

void __cdecl PM_SendEmtpyOffhandEvent(playerState_s *ps, OffhandClass offhandClass)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3409, 0, "%s", "ps");
    PM_AddEvent(ps, 0xDu);
    if (BG_GetFirstEquippedOffhand(ps, offhandClass))
    {
        if (offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
            PM_AddEvent(ps, 0x43u);
        else
            PM_AddEvent(ps, 0x44u);
    }
}

bool __cdecl PM_Weapon_IsHoldingGrenade(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3429, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps->weapon)
        return 0;
    weapDef = BG_GetWeaponDef(ps->weapon);
    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3437, 0, "%s", "weapDef");
    if (weapDef->weapType != WEAPTYPE_GRENADE)
        return 0;
    return !weapDef->holdButtonToThrow && (pm->cmd.buttons & PM_GetWeaponFireButton(ps->weapon)) != 0;
}

char __cdecl PM_UpdateGrenadeThrow(playerState_s *ps, pml_t *pml)
{
    int weapIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if ((ps->weapFlags & 2) != 0)
    {
        weapIndex = ps->offHandIndex;
        if (!weapIndex)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3548, 0, "%s", "weapIndex != WP_NONE");
    }
    else
    {
        weapIndex = ps->weapon;
        if (!weapIndex)
            return 0;
    }
    weapDef = BG_GetWeaponDef(weapIndex);
    if (weapDef->weapType != WEAPTYPE_GRENADE)
        return 0;
    if (ps->grenadeTimeLeft <= 0)
        return 0;
    if (weapDef->bCookOffHold)
        ps->grenadeTimeLeft -= pml->msec;
    if (ps->grenadeTimeLeft > 0)
        return 0;
    ps->grenadeTimeLeft = -1;
    BG_AddPredictableEventToPlayerstate(0x3Eu, ps->offHandIndex, ps);
    if (!BG_ThrowingBackGrenade(ps))
        PM_WeaponUseAmmo(ps, weapIndex, 1);
    return 1;
}

char __cdecl PM_Weapon_CheckGrenadeHold(pmove_t *pm, int32_t delayedAction)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3590, 0, "%s", "pm");
    if (!delayedAction)
        return 0;
    ps = pm->ps;
    if (!pm->ps->weapon)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3597, 0, "%s", "ps->weapon != WP_NONE");
    if (!PM_Weapon_IsHoldingGrenade(pm))
        return 0;
    ps->weaponDelay = 1;
    return 1;
}

void __cdecl PM_Weapon_CheckForDetonation(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3616, 0, "%s", "pm");
    ps = pm->ps;
    if (pm->ps->weapon)
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        if (!weapDef)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3623, 0, "%s", "weapDef");
        if (weapDef->weapType == WEAPTYPE_GRENADE
            && weapDef->hasDetonator
            && ps->weaponstate != 21
            && ps->weaponstate != 7
            && ps->weaponstate != 9
            && ps->weaponstate != 11
            && ps->weaponstate != 10
            && ps->weaponstate != 8
            && ps->weaponstate != 5
            && ps->weaponstate != 6
            && ps->weaponstate != 12
            && ps->weaponstate != 13
            && ps->weaponstate != 14
            && ps->weaponstate != 12
            && ps->weaponstate != 13
            && ps->weaponstate != 14
            && ps->weaponstate != 1
            && ps->weaponstate != 2
            && ps->weaponstate != 3
            && ps->weaponstate != 4
            && (ps->weaponstate < 15 || ps->weaponstate > 20)
            && ps->weaponstate != 25
            && ps->weaponstate != 26
            && (pm->cmd.buttons & 1) != 0)
        {
            ps->weaponstate = 21;
            ps->weaponTime = weapDef->iDetonateTime;
            ps->weaponDelay = weapDef->iDetonateDelay;
            PM_StartWeaponAnim(ps, 27);
        }
    }
}

void __cdecl PM_Weapon_CheckForGrenadeThrowCancel(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]
    WeaponDef *weapDefa; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3662, 0, "%s", "pm");
    ps = pm->ps;
    if (pm->ps->weaponstate == 16)
    {
        weapDef = BG_GetWeaponDef(ps->offHandIndex);
        if (!weapDef)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3668, 0, "%s", "weapDef");
        if (weapDef->holdButtonToThrow)
        {
            if ((pm->cmd.buttons & 0xC000) == 0)
                PM_Weapon_OffHandEnd(ps);
        }
    }
    else if (ps->weapon)
    {
        weapDefa = BG_GetWeaponDef(ps->weapon);
        if (!weapDefa)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3685, 0, "%s", "weapDef");
        if (weapDefa->weapType == WEAPTYPE_GRENADE
            && ps->weaponstate == 5
            && weapDefa->holdButtonToThrow
            && (pm->cmd.buttons & 1) == 0)
        {
            PM_Weapon_Idle(ps);
            PM_StartWeaponAnim(ps, 1);
        }
    }
}

void __cdecl PM_Detonate(playerState_s *ps, int32_t delayedAction)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3708, 0, "%s", "ps");
    if (delayedAction && ps->weapon)
        PM_AddEvent(ps, 0x3Fu);
    else
        PM_Weapon_Idle(ps);
}

void __cdecl PM_Weapon_CheckForNightVision(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-8h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3726, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3728, 0, "%s", "ps");
    BG_GetWeaponDef(ps->weapon);
    if ((pm->oldcmd.buttons & 0x40000) == 0 && (pm->cmd.buttons & 0x40000) != 0)
    {
        if ((ps->weapFlags & 0x40) != 0)
        {
            ps->weapFlags &= ~0x40u;
            PM_AddEvent(ps, 0x41u);
        }
        else
        {
            ps->weapFlags |= 0x40u;
            PM_AddEvent(ps, 0x40u);
        }
    }
}

void __cdecl PM_Weapon_FinishNightVisionWear(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3781, 0, "%s", "ps");
    if (ps->weaponstate != 25)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3782, 0, "%s", "ps->weaponstate == WEAPON_NIGHTVISION_WEAR");
    if (!ps->weaponTime)
    {
        ps->weaponstate = 0;
        PM_StartWeaponAnim(ps, 0);
    }
}

void __cdecl PM_Weapon_FinishNightVisionRemove(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3793, 0, "%s", "ps");
    if (ps->weaponstate != 26)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3794, 0, "%s", "ps->weaponstate == WEAPON_NIGHTVISION_REMOVE");
    if (!ps->weaponTime)
    {
        ps->weaponstate = 0;
        PM_StartWeaponAnim(ps, 0);
    }
}

void __cdecl Sprint_State_Loop(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3821, 0, "%s", "ps");
    ps->weaponstate = 23;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 24);
}

void __cdecl PM_Weapon_CheckForSprint(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3851, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3853, 0, "%s", "ps");
    if (pm->cmd.weapon
        && ps->weaponstate != 5
        && ps->weaponstate != 6
        && ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14
        && ps->weaponstate != 12
        && ps->weaponstate != 13
        && ps->weaponstate != 14
        && ps->weaponstate != 1
        && ps->weaponstate != 2
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && (ps->weaponstate < 15 || ps->weaponstate > 20)
        && ps->weaponstate != 25
        && ps->weaponstate != 26)
    {
        if ((ps->pm_flags & 0x8000) != 0 && (ps->weaponstate < 22 || ps->weaponstate > 24))
        {
            Sprint_State_Raise(ps);
        }
        else if ((ps->pm_flags & 0x8000) == 0 && (ps->weaponstate == 22 || ps->weaponstate == 23))
        {
            Sprint_State_Drop(ps);
        }
    }
}

void __cdecl Sprint_State_Raise(playerState_s *ps)
{
    WeaponDef *WeaponDef; // eax

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3807, 0, "%s", "ps");
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = 22;
    ps->weaponTime = WeaponDef->sprintInTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 23);
}

void __cdecl Sprint_State_Drop(playerState_s *ps)
{
    WeaponDef *WeaponDef; // eax

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 3835, 0, "%s", "ps");
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = 24;
    ps->weaponTime = WeaponDef->sprintOutTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 25);
}

void __cdecl PM_ResetWeaponState(playerState_s *ps)
{
    PM_Weapon_Idle(ps);
}

void __cdecl BG_WeaponFireRecoil(const playerState_s *ps, float *vGunSpeed, float *kickAVel)
{
    double v3; // st7
    double v4; // [esp+0h] [ebp-58h]
    double v5; // [esp+8h] [ebp-50h]
    double v6; // [esp+10h] [ebp-48h]
    double v7; // [esp+18h] [ebp-40h]
    double v8; // [esp+20h] [ebp-38h]
    double v9; // [esp+28h] [ebp-30h]
    double v10; // [esp+30h] [ebp-28h]
    double v11; // [esp+38h] [ebp-20h]
    float fReducePercent; // [esp+40h] [ebp-18h]
    float fYawKick; // [esp+44h] [ebp-14h]
    float fYawKickb; // [esp+44h] [ebp-14h]
    float fYawKicka; // [esp+44h] [ebp-14h]
    float fYawKickc; // [esp+44h] [ebp-14h]
    float fPitchKick; // [esp+48h] [ebp-10h]
    float fPitchKickb; // [esp+48h] [ebp-10h]
    float fPitchKicka; // [esp+48h] [ebp-10h]
    float fPitchKickc; // [esp+48h] [ebp-10h]
    int weapIndex; // [esp+4Ch] [ebp-Ch]
    float fPosLerp; // [esp+50h] [ebp-8h]
    WeaponDef *weapDef; // [esp+54h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    fPosLerp = ps->fWeaponPosFrac;
    fReducePercent = 1.0;
    if (ps->weaponRestrictKickTime > 0)
    {
        if (fPosLerp == 1.0)
            v3 = weapDef->adsGunKickReducedKickPercent * 0.009999999776482582;
        else
            v3 = weapDef->hipGunKickReducedKickPercent * 0.009999999776482582;
        fReducePercent = v3;
    }
    if (fPosLerp == 1.0)
    {
        v11 = weapDef->fAdsViewKickPitchMax - weapDef->fAdsViewKickPitchMin;
        fPitchKick = random() * v11 + weapDef->fAdsViewKickPitchMin;
        v10 = weapDef->fAdsViewKickYawMax - weapDef->fAdsViewKickYawMin;
        fYawKick = random() * v10 + weapDef->fAdsViewKickYawMin;
    }
    else
    {
        v9 = weapDef->fHipViewKickPitchMax - weapDef->fHipViewKickPitchMin;
        fPitchKick = random() * v9 + weapDef->fHipViewKickPitchMin;
        v8 = weapDef->fHipViewKickYawMax - weapDef->fHipViewKickYawMin;
        fYawKick = random() * v8 + weapDef->fHipViewKickYawMin;
    }
    fPitchKickb = fPitchKick * fReducePercent;
    fYawKickb = fYawKick * fReducePercent;
    *kickAVel = -fPitchKickb;
    kickAVel[1] = fYawKickb;
    kickAVel[2] = kickAVel[1] * -0.5;
    if (fPosLerp <= 0.0)
    {
        v5 = weapDef->fHipGunKickPitchMax - weapDef->fHipGunKickPitchMin;
        fPitchKicka = random() * v5 + weapDef->fHipGunKickPitchMin;
        v4 = weapDef->fHipGunKickYawMax - weapDef->fHipGunKickYawMin;
        fYawKicka = random() * v4 + weapDef->fHipGunKickYawMin;
    }
    else
    {
        v7 = weapDef->fAdsGunKickPitchMax - weapDef->fAdsGunKickPitchMin;
        fPitchKicka = random() * v7 + weapDef->fAdsGunKickPitchMin;
        v6 = weapDef->fAdsGunKickYawMax - weapDef->fAdsGunKickYawMin;
        fYawKicka = random() * v6 + weapDef->fAdsGunKickYawMin;
    }
    fPitchKickc = fPitchKicka * fReducePercent;
    fYawKickc = fYawKicka * fReducePercent;
    *vGunSpeed = *vGunSpeed + fPitchKickc;
    vGunSpeed[1] = vGunSpeed[1] + fYawKickc;
}

double __cdecl BG_GetBobCycle(const playerState_s *ps)
{
    return (float)((double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
        + (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
        + 6.283185482025146);
}

double __cdecl BG_GetVerticalBobFactor(const playerState_s *ps, float cycle, float speed, float maxAmp)
{
    double v4; // st7
    float v7; // [esp+4h] [ebp-18h]
    float v8; // [esp+8h] [ebp-14h]
    float v9; // [esp+Ch] [ebp-10h]
    float v10; // [esp+10h] [ebp-Ch]
    float amplitude; // [esp+18h] [ebp-4h]

    if (ps->viewHeightTarget == 11)
    {
        amplitude = speed * bg_bobAmplitudeProne->current.vector[1];
    }
    else if (ps->viewHeightTarget == 40)
    {
        amplitude = speed * bg_bobAmplitudeDucked->current.vector[1];
    }
    else
    {
        if ((ps->pm_flags & 0x8000) != 0)
            v4 = speed * bg_bobAmplitudeSprinting->current.vector[1];
        else
            v4 = speed * bg_bobAmplitudeStanding->current.vector[1];
        amplitude = v4;
    }
    if (maxAmp < (double)amplitude)
        amplitude = maxAmp;
    v10 = cycle + cycle;
    v9 = sin(v10);
    v8 = cycle * 4.0 + 1.570796370506287;
    v7 = sin(v8);
    return (float)((v7 * 0.2000000029802322 + v9) * 0.75 * amplitude);
}

double __cdecl BG_GetHorizontalBobFactor(const playerState_s *ps, float cycle, float speed, float maxAmp)
{
    double v4; // st7
    float v7; // [esp+4h] [ebp-8h]
    float amplitude; // [esp+8h] [ebp-4h]

    if (ps->viewHeightTarget == 11)
    {
        amplitude = speed * bg_bobAmplitudeProne->current.value;
    }
    else if (ps->viewHeightTarget == 40)
    {
        amplitude = speed * bg_bobAmplitudeDucked->current.value;
    }
    else
    {
        if ((ps->pm_flags & 0x8000) != 0)
            v4 = speed * bg_bobAmplitudeSprinting->current.value;
        else
            v4 = speed * bg_bobAmplitudeStanding->current.value;
        amplitude = v4;
    }
    if (maxAmp < (double)amplitude)
        amplitude = maxAmp;
    v7 = sin(cycle);
    return (float)(v7 * amplitude);
}

void __cdecl BG_CalculateWeaponAngles(weaponState_t *ws, float *angles)
{
    float fLean; // [esp+10h] [ebp-8h]
    const playerState_s *ps; // [esp+14h] [ebp-4h]

    ps = ws->ps;
    *angles = 0.0;
    angles[1] = 0.0;
    angles[2] = 0.0;
    if (ps->leanf != 0.0)
    {
        fLean = GetLeanFraction(ps->leanf);
        angles[2] = angles[2] - (fLean + fLean);
    }
    BG_CalculateWeaponPosition_BaseAngles(ws, angles);
    BG_CalculateWeaponPosition_IdleAngles(ws, angles);
    BG_CalculateWeaponPosition_BobOffset(ws, angles);
    BG_CalculateWeaponPosition_DamageKick(ws, angles);
    BG_CalculateWeaponPosition_GunRecoil(ws, angles);
    *angles = AngleDelta(*angles, ws->swayAngles[0]);
    angles[1] = AngleDelta(angles[1], ws->swayAngles[1]);
}

void __cdecl BG_CalculateWeaponPosition_BaseAngles(weaponState_t *ws, float *angles)
{
    int weapIndex; // [esp+0h] [ebp-Ch]
    const playerState_s *ps; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (BG_IsAimDownSightWeapon(weapIndex))
        *angles = ps->fWeaponPosFrac * weapDef->fAdsAimPitch + *angles;
    BG_CalculateWeaponPosition_BasePosition_angles(ws, angles);
}

void __cdecl BG_CalculateWeaponPosition_BasePosition_angles(weaponState_t *ws, float *angles)
{
    double v2; // st7
    float scale; // [esp+Ch] [ebp-38h]
    float v4; // [esp+10h] [ebp-34h]
    float v5; // [esp+14h] [ebp-30h]
    float v6; // [esp+18h] [ebp-2Ch]
    float fMinSpeed; // [esp+1Ch] [ebp-28h]
    float fFrac; // [esp+20h] [ebp-24h]
    float fDeltaa; // [esp+24h] [ebp-20h]
    float fDelta; // [esp+24h] [ebp-20h]
    float fDeltab; // [esp+24h] [ebp-20h]
    int weapIndex; // [esp+28h] [ebp-1Ch]
    int i; // [esp+2Ch] [ebp-18h]
    const playerState_s *ps; // [esp+30h] [ebp-14h]
    WeaponDef *weapDef; // [esp+34h] [ebp-10h]
    float vTargetAng[3]; // [esp+38h] [ebp-Ch] BYREF

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ((ps->eFlags & 8) != 0)
    {
        fMinSpeed = weapDef->fProneRotMinSpeed;
    }
    else if ((ps->eFlags & 4) != 0)
    {
        fMinSpeed = weapDef->fDuckedRotMinSpeed;
    }
    else
    {
        fMinSpeed = weapDef->fStandRotMinSpeed;
    }
    if (fMinSpeed >= (double)ws->xyspeed || ps->weaponstate == 7)
    {
        vTargetAng[0] = 0.0;
        vTargetAng[1] = 0.0;
        vTargetAng[2] = 0.0;
    }
    else
    {
        fFrac = (ws->xyspeed - fMinSpeed) / ((double)ps->speed - fMinSpeed);
        v5 = 0.0 - fFrac;
        if (v5 < 0.0)
            v6 = (ws->xyspeed - fMinSpeed) / ((double)ps->speed - fMinSpeed);
        else
            v6 = 0.0;
        v4 = v6 - 1.0;
        if (v4 < 0.0)
            scale = v6;
        else
            scale = 1.0;
        if ((ps->eFlags & 8) != 0)
        {
            Vec3Scale(weapDef->vProneRot, scale, vTargetAng);
        }
        else if ((ps->eFlags & 4) != 0)
        {
            Vec3Scale(weapDef->vDuckedRot, scale, vTargetAng);
        }
        else
        {
            Vec3Scale(weapDef->vStandRot, scale, vTargetAng);
        }
    }
    if (ps->fWeaponPosFrac != 0.0)
    {
        fDeltaa = 1.0 - ps->fWeaponPosFrac;
        Vec3Scale(vTargetAng, fDeltaa, vTargetAng);
    }
    for (i = 0; i < 3; ++i)
    {
        if (vTargetAng[i] != ws->vLastMoveAng[i])
        {
            if (ps->viewHeightCurrent == 11.0)
                v2 = (vTargetAng[i] - ws->vLastMoveAng[i]) * ws->frametime * weapDef->fPosProneRotRate;
            else
                v2 = (vTargetAng[i] - ws->vLastMoveAng[i]) * ws->frametime * weapDef->fPosRotRate;
            fDelta = v2;
            if (vTargetAng[i] <= (double)ws->vLastMoveAng[i])
            {
                if (fDelta > ws->frametime * -0.1000000014901161)
                    fDelta = ws->frametime * -0.1000000014901161;
                ws->vLastMoveAng[i] = ws->vLastMoveAng[i] + fDelta;
                if (vTargetAng[i] > (double)ws->vLastMoveAng[i])
                    ws->vLastMoveAng[i] = vTargetAng[i];
            }
            else
            {
                if (fDelta < ws->frametime * 0.1000000014901161)
                    fDelta = ws->frametime * 0.1000000014901161;
                ws->vLastMoveAng[i] = ws->vLastMoveAng[i] + fDelta;
                if (vTargetAng[i] < (double)ws->vLastMoveAng[i])
                    ws->vLastMoveAng[i] = vTargetAng[i];
            }
        }
    }
    if (ps->fWeaponPosFrac == 0.0)
    {
        Vec3Add(angles, ws->vLastMoveAng, angles);
    }
    else if (ps->fWeaponPosFrac < 0.5)
    {
        fDeltab = 1.0 - (ps->fWeaponPosFrac + ps->fWeaponPosFrac);
        Vec3Mad(angles, fDeltab, ws->vLastMoveAng, angles);
    }
}

void __cdecl BG_CalculateWeaponPosition_IdleAngles(weaponState_t *ws, float *angles)
{
    float v2; // [esp+4h] [ebp-38h]
    float v3; // [esp+8h] [ebp-34h]
    float v4; // [esp+Ch] [ebp-30h]
    float v5; // [esp+10h] [ebp-2Ch]
    float v6; // [esp+14h] [ebp-28h]
    float v7; // [esp+18h] [ebp-24h]
    float fTargScale; // [esp+1Ch] [ebp-20h]
    float fTargScalea; // [esp+1Ch] [ebp-20h]
    float fTargFactor; // [esp+24h] [ebp-18h]
    float fFactorSpeed; // [esp+28h] [ebp-14h]
    float idleSpeed; // [esp+2Ch] [ebp-10h]
    int weapIndex; // [esp+30h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+34h] [ebp-8h]
    const playerState_s *ps; // [esp+38h] [ebp-4h]

    fFactorSpeed = 0.5;
    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (BG_IsAimDownSightWeapon(weapIndex))
    {
        fTargScale = (weapDef->fAdsIdleAmount - weapDef->fHipIdleAmount) * ps->fWeaponPosFrac + weapDef->fHipIdleAmount;
        idleSpeed = (weapDef->adsIdleSpeed - weapDef->hipIdleSpeed) * ps->fWeaponPosFrac + weapDef->hipIdleSpeed;
    }
    else if (weapDef->fHipIdleAmount == 0.0)
    {
        fTargScale = 80.0;
        idleSpeed = 1.0;
    }
    else
    {
        fTargScale = weapDef->fHipIdleAmount;
        idleSpeed = weapDef->hipIdleSpeed;
    }
    if ((ps->eFlags & 8) != 0)
    {
        fTargFactor = weapDef->fIdleProneFactor;
    }
    else if ((ps->eFlags & 4) != 0)
    {
        fTargFactor = weapDef->fIdleCrouchFactor;
    }
    else
    {
        fTargFactor = 1.0;
    }
    if ((weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE || ps->fWeaponPosFrac == 0.0)
        && ws->fLastIdleFactor != fTargFactor)
    {
        if (ws->fLastIdleFactor >= (double)fTargFactor)
        {
            ws->fLastIdleFactor = ws->fLastIdleFactor - ws->frametime * fFactorSpeed;
            if (fTargFactor > (double)ws->fLastIdleFactor)
                ws->fLastIdleFactor = fTargFactor;
        }
        else
        {
            ws->fLastIdleFactor = ws->frametime * fFactorSpeed + ws->fLastIdleFactor;
            if (fTargFactor < (double)ws->fLastIdleFactor)
                ws->fLastIdleFactor = fTargFactor;
        }
    }
    fTargScalea = fTargScale * ws->fLastIdleFactor;
    if (weapDef->overlayReticle)
        fTargScalea = (1.0 - ps->fWeaponPosFrac) * fTargScalea;
    *ws->weapIdleTime += (int)(ws->frametime * 1000.0 * idleSpeed);
    v7 = (double)*ws->weapIdleTime * 0.0005000000237487257;
    v4 = sin(v7);
    angles[2] = fTargScalea * v4 * 0.009999999776482582 + angles[2];
    v6 = (double)*ws->weapIdleTime * 0.000699999975040555;
    v3 = sin(v6);
    angles[1] = fTargScalea * v3 * 0.009999999776482582 + angles[1];
    v5 = (double)*ws->weapIdleTime * EQUAL_EPSILON;
    v2 = sin(v5);
    *angles = fTargScalea * v2 * 0.009999999776482582 + *angles;
}

void __cdecl BG_CalculateWeaponPosition_BobOffset(weaponState_t *ws, float *angles)
{
    float scale; // [esp+Ch] [ebp-3Ch]
    float v3; // [esp+10h] [ebp-38h]
    float v4; // [esp+14h] [ebp-34h]
    float HorizontalBobFactor; // [esp+1Ch] [ebp-2Ch]
    float vAngOfs[3]; // [esp+20h] [ebp-28h] BYREF
    float fBobCycle; // [esp+2Ch] [ebp-1Ch]
    float cycle; // [esp+30h] [ebp-18h]
    float speed; // [esp+34h] [ebp-14h]
    int weapIndex; // [esp+38h] [ebp-10h]
    float fPositionLerp; // [esp+3Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+40h] [ebp-8h]
    WeaponDef *weapDef; // [esp+44h] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    fBobCycle = (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
        + (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
        + 6.283185482025146;
    cycle = fBobCycle + 0.7853981852531433 + 6.283185482025146;
    speed = ws->xyspeed * 0.1599999964237213;
    vAngOfs[0] = BG_GetVerticalBobFactor(ps, cycle, speed, 10.0) * -1.0;
    vAngOfs[1] = BG_GetHorizontalBobFactor(ps, cycle, speed, 10.0) * -1.0;
    cycle = cycle - 0.4712389409542084;
    speed = speed * 1.5;
    HorizontalBobFactor = BG_GetHorizontalBobFactor(ps, cycle, speed, 10.0);
    v4 = 0.0 - HorizontalBobFactor;
    if (v4 < 0.0)
        v3 = 0.0;
    else
        v3 = HorizontalBobFactor;
    vAngOfs[2] = v3;
    fPositionLerp = ps->fWeaponPosFrac;
    if (fPositionLerp != 0.0)
    {
        speed = 1.0 - (1.0 - weapDef->fAdsBobFactor) * fPositionLerp;
        vAngOfs[0] = vAngOfs[0] * speed;
        vAngOfs[1] = vAngOfs[1] * speed;
        vAngOfs[2] = vAngOfs[2] * speed;
    }
    if (weapDef->overlayReticle)
    {
        scale = 1.0 - fPositionLerp;
        Vec3Scale(vAngOfs, scale, vAngOfs);
    }
    Vec3Add(angles, vAngOfs, angles);
}

void __cdecl BG_CalculateWeaponPosition_DamageKick(weaponState_t *ws, float *angles)
{
    float fFrac; // [esp+4h] [ebp-24h]
    float fReturnTime; // [esp+Ch] [ebp-1Ch]
    float fDeflectTime; // [esp+10h] [ebp-18h]
    float fRatio; // [esp+14h] [ebp-14h]
    float fRatiob; // [esp+14h] [ebp-14h]
    float fRatioc; // [esp+14h] [ebp-14h]
    float fRatiod; // [esp+14h] [ebp-14h]
    float fRatioa; // [esp+14h] [ebp-14h]
    float fRatioe; // [esp+14h] [ebp-14h]
    float fRatiof; // [esp+14h] [ebp-14h]
    int weapIndex; // [esp+18h] [ebp-10h]
    float fFactor; // [esp+1Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+20h] [ebp-8h]
    WeaponDef *weapDef; // [esp+24h] [ebp-4h]

    if (ws->damageTime)
    {
        ps = ws->ps;
        weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        fFactor = ps->fWeaponPosFrac * 0.5 + 0.5;
        fDeflectTime = fFactor * 100.0;
        fReturnTime = fFactor * 400.0;
        if (ps->fWeaponPosFrac != 0.0 && weapDef->overlayReticle)
            fFactor = (1.0 - ps->fWeaponPosFrac * 0.75) * fFactor;
        fRatio = (float)(ws->time - ws->damageTime);
        if (fDeflectTime <= (double)fRatio)
        {
            fRatioa = 1.0 - (fRatio - fDeflectTime) / fReturnTime;
            if (fRatioa > 0.0)
            {
                fFrac = 1.0 - fRatioa;
                fRatioe = 1.0 - GetLeanFraction(fFrac);
                fRatiof = fRatioe * fFactor;
                *angles = fRatiof * ws->v_dmg_pitch * 0.5 + *angles;
                angles[1] = angles[1] - fRatiof * ws->v_dmg_roll;
                angles[2] = fRatiof * ws->v_dmg_roll * 0.5 + angles[2];
            }
        }
        else
        {
            fRatiob = fRatio / fDeflectTime;
            fRatioc = GetLeanFraction(fRatiob);
            fRatiod = fRatioc * fFactor;
            *angles = fRatiod * ws->v_dmg_pitch * 0.5 + *angles;
            angles[1] = angles[1] - fRatiod * ws->v_dmg_roll;
            angles[2] = fRatiod * ws->v_dmg_roll * 0.5 + angles[2];
        }
    }
}

void __cdecl BG_CalculateWeaponPosition_GunRecoil(weaponState_t *ws, float *angles)
{
    int v2; // eax
    float fTimeStep; // [esp+18h] [ebp-2Ch]
    float fGunKickAccel; // [esp+1Ch] [ebp-28h]
    float fGunKickSpeedDecay; // [esp+20h] [ebp-24h]
    float fGunKickStaticDecay; // [esp+24h] [ebp-20h]
    float fTotalTime; // [esp+28h] [ebp-1Ch]
    float fGunKickSpeedMax; // [esp+2Ch] [ebp-18h]
    int bCanStop; // [esp+30h] [ebp-14h]
    int weapIndex; // [esp+34h] [ebp-10h]
    float fPosLerp; // [esp+38h] [ebp-Ch]
    const playerState_s *ps; // [esp+3Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+40h] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (BG_IsAimDownSightWeapon(weapIndex))
    {
        fPosLerp = ps->fWeaponPosFrac;
        fGunKickAccel = (weapDef->fAdsGunKickAccel - weapDef->fHipGunKickAccel) * fPosLerp + weapDef->fHipGunKickAccel;
        fGunKickSpeedMax = (weapDef->fAdsGunKickSpeedMax - weapDef->fHipGunKickSpeedMax) * fPosLerp
            + weapDef->fHipGunKickSpeedMax;
        fGunKickSpeedDecay = (weapDef->fAdsGunKickSpeedDecay - weapDef->fHipGunKickSpeedDecay) * fPosLerp
            + weapDef->fHipGunKickSpeedDecay;
        fGunKickStaticDecay = (weapDef->fAdsGunKickStaticDecay - weapDef->fHipGunKickStaticDecay) * fPosLerp
            + weapDef->fHipGunKickStaticDecay;
        fTotalTime = ws->frametime;
        while (fTotalTime > 0.0f)
        {
            if (fTotalTime <= 0.004999999888241291f)
            {
                fTimeStep = fTotalTime;
                fTotalTime = 0.0f;
                v2 = BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
                    ws->vGunOffset,
                    ws->vGunSpeed,
                    fTimeStep,
                    weapDef->fGunMaxPitch,
                    fGunKickAccel,
                    fGunKickSpeedMax,
                    fGunKickSpeedDecay,
                    fGunKickStaticDecay);
            }
            else
            {
                fTimeStep = 0.0049999999f;
                fTotalTime = fTotalTime - 0.004999999888241291f;
                v2 = BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
                    ws->vGunOffset,
                    ws->vGunSpeed,
                    0.0049999999f,
                    weapDef->fGunMaxPitch,
                    fGunKickAccel,
                    fGunKickSpeedMax,
                    fGunKickSpeedDecay,
                    fGunKickStaticDecay);
            }
            bCanStop = v2;
            if (BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
                &ws->vGunOffset[1],
                &ws->vGunSpeed[1],
                fTimeStep,
                weapDef->fGunMaxYaw,
                fGunKickAccel,
                fGunKickSpeedMax,
                fGunKickSpeedDecay,
                fGunKickStaticDecay))
            {
                if (bCanStop)
                    break;
            }
        }
        Vec3Add(angles, ws->vGunOffset, angles);
    }
}

int __cdecl BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
    float *fOffset,
    float *speed,
    float fTimeStep,
    float fOfsCap,
    float fGunKickAccel,
    float fGunKickSpeedMax,
    float fGunKickSpeedDecay,
    float fGunKickStaticDecay)
{
    float v9; // [esp+0h] [ebp-14h]
    float v10; // [esp+4h] [ebp-10h]
    int bCanStop; // [esp+10h] [ebp-4h]

    bCanStop = 0;
    v10 = I_fabs(*fOffset);
    if (v10 >= 0.25 || (v9 = I_fabs(*speed), v9 >= 1.0))
    {
        *fOffset = *speed * fTimeStep + *fOffset;
        if (fOfsCap >= (double)*fOffset)
        {
            if (*fOffset < -fOfsCap)
            {
                *fOffset = -fOfsCap;
                if (*speed < 0.0)
                    *speed = 0.0;
            }
        }
        else
        {
            *fOffset = fOfsCap;
            if (*speed > 0.0)
                *speed = 0.0;
        }
        if (*fOffset <= 0.0)
        {
            if (*fOffset < 0.0)
                *speed = fGunKickAccel * fTimeStep + *speed;
        }
        else
        {
            *speed = *speed - fGunKickAccel * fTimeStep;
        }
        *speed = *speed - *speed * fGunKickSpeedDecay * fTimeStep;
        if (*speed <= 0.0)
        {
            *speed = fGunKickStaticDecay * fTimeStep + *speed;
            if (*speed > 0.0)
                *speed = 0.0;
        }
        else
        {
            *speed = *speed - fGunKickStaticDecay * fTimeStep;
            if (*speed < 0.0)
                *speed = 0.0;
        }
        if (fGunKickSpeedMax >= (double)*speed)
        {
            if (*speed < -fGunKickSpeedMax)
                *speed = -fGunKickSpeedMax;
        }
        else
        {
            *speed = fGunKickSpeedMax;
        }
    }
    else
    {
        *fOffset = 0.0;
        *speed = 0.0;
        return 1;
    }
    return bCanStop;
}

void __cdecl BG_CalculateViewAngles(viewState_t *vs, float *angles)
{
    *angles = 0.0;
    angles[1] = 0.0;
    angles[2] = 0.0;
    BG_CalculateView_DamageKick(vs, angles);
    BG_CalculateView_IdleAngles(vs, angles);
    BG_CalculateView_BobAngles(vs, angles);
    BG_CalculateView_Velocity(vs, angles);
}

void __cdecl BG_CalculateView_DamageKick(viewState_t *vs, float *angles)
{
    float fFrac; // [esp+4h] [ebp-1Ch]
    float fRatio; // [esp+Ch] [ebp-14h]
    float fRatiob; // [esp+Ch] [ebp-14h]
    float fRatioc; // [esp+Ch] [ebp-14h]
    float fRatiod; // [esp+Ch] [ebp-14h]
    float fRatioa; // [esp+Ch] [ebp-14h]
    float fRatioe; // [esp+Ch] [ebp-14h]
    float fRatiof; // [esp+Ch] [ebp-14h]
    int weapIndex; // [esp+10h] [ebp-10h]
    float fFactor; // [esp+14h] [ebp-Ch]
    playerState_s *ps; // [esp+18h] [ebp-8h]
    WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    if (vs->damageTime)
    {
        ps = vs->ps;
        weapIndex = BG_GetViewmodelWeaponIndex(vs->ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        fFactor = 1.0 - ps->fWeaponPosFrac * 0.5;
        if (ps->fWeaponPosFrac != 0.0 && weapDef->overlayReticle)
            fFactor = (ps->fWeaponPosFrac * 0.5 + 1.0) * fFactor;
        fRatio = (float)(vs->time - vs->damageTime);
        if (fRatio >= 100.0)
        {
            fRatioa = 1.0 - (fRatio - 100.0) / 400.0;
            if (fRatioa > 0.0)
            {
                fFrac = 1.0 - fRatioa;
                fRatioe = 1.0 - GetLeanFraction(fFrac);
                fRatiof = fRatioe * fFactor;
                *angles = fRatiof * vs->v_dmg_pitch + *angles;
                angles[2] = fRatiof * vs->v_dmg_roll + angles[2];
            }
        }
        else
        {
            fRatiob = fRatio / 100.0;
            fRatioc = GetLeanFraction(fRatiob);
            fRatiod = fRatioc * fFactor;
            *angles = fRatiod * vs->v_dmg_pitch + *angles;
            angles[2] = fRatiod * vs->v_dmg_roll + angles[2];
        }
    }
}

void __cdecl BG_CalculateView_IdleAngles(viewState_t *vs, float *angles)
{
    float v2; // [esp+4h] [ebp-30h]
    float v3; // [esp+8h] [ebp-2Ch]
    float v4; // [esp+Ch] [ebp-28h]
    float v5; // [esp+10h] [ebp-24h]
    float fTargScale; // [esp+14h] [ebp-20h]
    float fTargScalea; // [esp+14h] [ebp-20h]
    float fTargScaleb; // [esp+14h] [ebp-20h]
    float fTargScalec; // [esp+14h] [ebp-20h]
    float fTargFactor; // [esp+1Ch] [ebp-18h]
    float fFactorSpeed; // [esp+20h] [ebp-14h]
    float idleSpeed; // [esp+24h] [ebp-10h]
    int weapIndex; // [esp+28h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+2Ch] [ebp-8h]
    playerState_s *ps; // [esp+30h] [ebp-4h]

    fFactorSpeed = 0.5;
    ps = vs->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(vs->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (weapDef->overlayReticle)
    {
        if (BG_IsAimDownSightWeapon(weapIndex))
        {
            fTargScale = (weapDef->fAdsIdleAmount - weapDef->fHipIdleAmount) * ps->fWeaponPosFrac + weapDef->fHipIdleAmount;
            idleSpeed = (weapDef->adsIdleSpeed - weapDef->hipIdleSpeed) * ps->fWeaponPosFrac + weapDef->hipIdleSpeed;
        }
        else if (weapDef->fHipIdleAmount == 0.0)
        {
            fTargScale = 80.0;
            idleSpeed = 1.0;
        }
        else
        {
            fTargScale = weapDef->fHipIdleAmount;
            idleSpeed = weapDef->hipIdleSpeed;
        }
        if ((ps->eFlags & 8) != 0)
        {
            fTargFactor = weapDef->fIdleProneFactor;
        }
        else if ((ps->eFlags & 4) != 0)
        {
            fTargFactor = weapDef->fIdleCrouchFactor;
        }
        else
        {
            fTargFactor = 1.0;
        }
        if (weapDef->overlayReticle && ps->fWeaponPosFrac != 0.0 && vs->fLastIdleFactor != fTargFactor)
        {
            if (vs->fLastIdleFactor >= (double)fTargFactor)
            {
                vs->fLastIdleFactor = vs->fLastIdleFactor - vs->frametime * fFactorSpeed;
                if (fTargFactor > (double)vs->fLastIdleFactor)
                    vs->fLastIdleFactor = fTargFactor;
            }
            else
            {
                vs->fLastIdleFactor = vs->frametime * fFactorSpeed + vs->fLastIdleFactor;
                if (fTargFactor < (double)vs->fLastIdleFactor)
                    vs->fLastIdleFactor = fTargFactor;
            }
        }
        fTargScalea = fTargScale * vs->fLastIdleFactor;
        fTargScaleb = fTargScalea * ps->fWeaponPosFrac;
        fTargScalec = fTargScaleb * ps->holdBreathScale;
        *vs->weapIdleTime += (int)(ps->holdBreathScale * vs->frametime * 1000.0 * idleSpeed);
        v5 = (double)*vs->weapIdleTime * 0.000699999975040555;
        v3 = sin(v5);
        angles[1] = fTargScalec * v3 * 0.009999999776482582 + angles[1];
        v4 = (double)*vs->weapIdleTime * EQUAL_EPSILON;
        v2 = sin(v4);
        *angles = fTargScalec * v2 * 0.009999999776482582 + *angles;
    }
}

void __cdecl BG_CalculateView_BobAngles(viewState_t *vs, float *angles)
{
    float v2; // [esp+Ch] [ebp-38h]
    float v3; // [esp+10h] [ebp-34h]
    float HorizontalBobFactor; // [esp+18h] [ebp-2Ch]
    float vAngOfs[3]; // [esp+1Ch] [ebp-28h] BYREF
    float fBobCycle; // [esp+28h] [ebp-1Ch]
    float cycle; // [esp+2Ch] [ebp-18h]
    float speed; // [esp+30h] [ebp-14h]
    int weapIndex; // [esp+34h] [ebp-10h]
    float fPositionLerp; // [esp+38h] [ebp-Ch]
    playerState_s *ps; // [esp+3Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+40h] [ebp-4h]

    ps = vs->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (weapDef->overlayReticle)
    {
        fBobCycle = (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
            + (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
            + 6.283185482025146;
        cycle = fBobCycle + 0.7853981852531433 + 6.283185482025146;
        speed = vs->xyspeed * 0.1599999964237213;
        vAngOfs[0] = BG_GetVerticalBobFactor(ps, cycle, speed, 10.0) * -1.0;
        vAngOfs[1] = BG_GetHorizontalBobFactor(ps, cycle, speed, 10.0) * -1.0;
        cycle = cycle - 0.4712389409542084;
        speed = speed * 1.5;
        HorizontalBobFactor = BG_GetHorizontalBobFactor(ps, cycle, speed, 10.0);
        v3 = 0.0 - HorizontalBobFactor;
        if (v3 < 0.0)
            v2 = 0.0;
        else
            v2 = HorizontalBobFactor;
        vAngOfs[2] = v2;
        fPositionLerp = ps->fWeaponPosFrac;
        if (fPositionLerp != 0.0)
        {
            speed = 1.0 - (1.0 - weapDef->fAdsBobFactor) * fPositionLerp;
            vAngOfs[0] = vAngOfs[0] * speed;
            vAngOfs[1] = vAngOfs[1] * speed;
            vAngOfs[2] = vAngOfs[2] * speed;
        }
        Vec3Scale(vAngOfs, fPositionLerp, vAngOfs);
        Vec3Add(angles, vAngOfs, angles);
    }
}

void __cdecl BG_CalculateView_Velocity(viewState_t *vs, float *angles)
{
    float delta; // [esp+10h] [ebp-18h]
    float deltaa; // [esp+10h] [ebp-18h]
    float fBobCycle; // [esp+14h] [ebp-14h]
    float deltaB; // [esp+18h] [ebp-10h]
    float deltaBa; // [esp+18h] [ebp-10h]
    int weapIndex; // [esp+1Ch] [ebp-Ch]
    playerState_s *ps; // [esp+20h] [ebp-8h]
    WeaponDef *weapDef; // [esp+24h] [ebp-4h]

    ps = vs->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(vs->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ((ps->eFlags & 0x300) == 0 && ps->fWeaponPosFrac != 0.0 && weapDef->fAdsViewBobMult != 0.0)
    {
        fBobCycle = (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
            + (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.141592741012573
            + 6.283185482025146;
        delta = BG_GetVerticalBobFactor(ps, fBobCycle, vs->xyspeed, 45.0);
        deltaa = ps->fWeaponPosFrac * weapDef->fAdsViewBobMult * delta;
        *angles = *angles - deltaa;
        deltaB = BG_GetHorizontalBobFactor(ps, fBobCycle, vs->xyspeed, 45.0);
        deltaBa = ps->fWeaponPosFrac * weapDef->fAdsViewBobMult * deltaB;
        angles[1] = angles[1] - deltaBa;
    }
}

void __cdecl BG_CalculateWeaponPosition_Sway(
    const playerState_s *ps,
    float *swayViewAngles,
    float *swayOffset,
    float *swayAngles,
    float ssSwayScale,
    int32_t frametime)
{
    float v6; // [esp+10h] [ebp-70h]
    float v7; // [esp+14h] [ebp-6Ch]
    float v8; // [esp+18h] [ebp-68h]
    float v9; // [esp+1Ch] [ebp-64h]
    float v10; // [esp+20h] [ebp-60h]
    float v11; // [esp+24h] [ebp-5Ch]
    float scale; // [esp+28h] [ebp-58h]
    float v13; // [esp+30h] [ebp-50h]
    float v14; // [esp+34h] [ebp-4Ch]
    float v15; // [esp+38h] [ebp-48h]
    float v16; // [esp+3Ch] [ebp-44h]
    float deltaOffset_4; // [esp+44h] [ebp-3Ch]
    float deltaOffset_8; // [esp+48h] [ebp-38h]
    float swayPitchScale; // [esp+4Ch] [ebp-34h]
    float swayPitchScalea; // [esp+4Ch] [ebp-34h]
    float dt; // [esp+50h] [ebp-30h]
    float swayVertScale; // [esp+54h] [ebp-2Ch]
    float swayVertScalea; // [esp+54h] [ebp-2Ch]
    float swayLerpSpeed; // [esp+58h] [ebp-28h]
    float swayHorizScale; // [esp+5Ch] [ebp-24h]
    float swayHorizScalea; // [esp+5Ch] [ebp-24h]
    float deltaAngles[3]; // [esp+60h] [ebp-20h] BYREF
    float swayYawScale; // [esp+6Ch] [ebp-14h]
    float f; // [esp+70h] [ebp-10h]
    int32_t weapIndex; // [esp+74h] [ebp-Ch]
    float swayMaxAngle; // [esp+78h] [ebp-8h]
    WeaponDef *weapDef; // [esp+7Ch] [ebp-4h]

    f = ps->fWeaponPosFrac;
    if (frametime)
    {
        weapIndex = BG_GetViewmodelWeaponIndex(ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        dt = (double)frametime * EQUAL_EPSILON;
        if (BG_IsAimDownSightWeapon(weapIndex))
        {
            if (f > 0.0 && weapDef->overlayReticle)
                return;
            swayMaxAngle = (weapDef->adsSwayMaxAngle - weapDef->swayMaxAngle) * f + weapDef->swayMaxAngle;
            swayLerpSpeed = (weapDef->adsSwayLerpSpeed - weapDef->swayLerpSpeed) * f + weapDef->swayLerpSpeed;
            swayPitchScale = (weapDef->adsSwayPitchScale - weapDef->swayPitchScale) * f + weapDef->swayPitchScale;
            swayYawScale = (weapDef->adsSwayYawScale - weapDef->swayYawScale) * f + weapDef->swayYawScale;
            swayHorizScale = (weapDef->adsSwayHorizScale - weapDef->swayHorizScale) * f + weapDef->swayHorizScale;
            swayVertScale = (weapDef->adsSwayVertScale - weapDef->swayVertScale) * f + weapDef->swayVertScale;
        }
        else
        {
            swayMaxAngle = weapDef->swayMaxAngle;
            swayLerpSpeed = weapDef->swayLerpSpeed;
            swayPitchScale = weapDef->swayPitchScale;
            swayYawScale = weapDef->swayYawScale;
            swayHorizScale = weapDef->swayHorizScale;
            swayVertScale = weapDef->swayVertScale;
        }
        swayPitchScalea = swayPitchScale * ssSwayScale;
        swayYawScale = swayYawScale * ssSwayScale;
        swayHorizScalea = swayHorizScale * ssSwayScale;
        swayVertScalea = swayVertScale * ssSwayScale;
        AnglesSubtract((float*)ps->viewangles, swayViewAngles, deltaAngles);
        if (dt == 0.0)
            MyAssertHandler(".\\bgame\\bg_weapons.cpp", 4978, 0, "%s", "dt");
        scale = 1.0 / (dt * 60.0);
        Vec3Scale(deltaAngles, scale, deltaAngles);
        v11 = deltaAngles[0] - swayMaxAngle;
        if (v11 < 0.0)
            v16 = deltaAngles[0];
        else
            v16 = swayMaxAngle;
        v15 = -swayMaxAngle;
        v10 = v15 - deltaAngles[0];
        if (v10 < 0.0)
            v9 = v16;
        else
            v9 = -swayMaxAngle;
        deltaAngles[0] = v9;
        v8 = deltaAngles[1] - swayMaxAngle;
        if (v8 < 0.0)
            v14 = deltaAngles[1];
        else
            v14 = swayMaxAngle;
        v13 = -swayMaxAngle;
        v7 = v13 - deltaAngles[1];
        if (v7 < 0.0)
            v6 = v14;
        else
            v6 = -swayMaxAngle;
        deltaAngles[1] = v6;
        deltaOffset_4 = v6 * swayHorizScalea;
        deltaOffset_8 = deltaAngles[0] * swayVertScalea;
        swayOffset[1] = DiffTrack(deltaOffset_4, swayOffset[1], swayLerpSpeed, dt);
        swayOffset[2] = DiffTrack(deltaOffset_8, swayOffset[2], swayLerpSpeed, dt);
        deltaAngles[0] = deltaAngles[0] * swayPitchScalea;
        deltaAngles[1] = deltaAngles[1] * swayYawScale;
        *swayAngles = DiffTrackAngle(deltaAngles[0], *swayAngles, swayLerpSpeed, dt);
        swayAngles[1] = DiffTrackAngle(deltaAngles[1], swayAngles[1], swayLerpSpeed, dt);
        *swayViewAngles = ps->viewangles[0];
        swayViewAngles[1] = ps->viewangles[1];
        swayViewAngles[2] = ps->viewangles[2];
    }
}

int __cdecl BG_PlayerWeaponCountPrimaryTypes(const playerState_s *ps)
{
    int weapCount; // [esp+0h] [ebp-10h]
    int weapIndex; // [esp+4h] [ebp-Ch]
    int resultCount; // [esp+Ch] [ebp-4h]

    if (!ps)
    {
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5005, 0, "%s", "ps");
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5012, 0, "%s", "ps");
    }
    weapCount = BG_GetNumWeapons();
    resultCount = 0;
    for (weapIndex = 1; weapIndex < weapCount; ++weapIndex)
    {
        if (BG_GetWeaponDef(weapIndex)->inventoryType == WEAPINVENTORY_PRIMARY)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, weapIndex, 16))
                ++resultCount;
        }
    }
    return resultCount;
}

bool __cdecl BG_PlayerWeaponsFull_Primaries(const playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5036, 0, "%s", "ps");
    return BG_PlayerWeaponCountPrimaryTypes(ps) >= 2;
}

char __cdecl BG_PlayerHasCompatibleWeapon(const playerState_s *ps, uint32_t weaponIndex)
{
    int32_t ammoIndex; // [esp+4h] [ebp-Ch]
    int32_t weapCount; // [esp+8h] [ebp-8h]
    int32_t idx; // [esp+Ch] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5049, 0, "%s", "ps");
    ammoIndex = BG_GetWeaponDef(weaponIndex)->iAmmoIndex;
    weapCount = BG_GetNumWeapons();
    for (idx = 1; idx < weapCount; ++idx)
    {
        if (BG_GetWeaponDef(idx)->iAmmoIndex == ammoIndex)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, idx, 16))
                return 1;
        }
    }
    return 0;
}

bool __cdecl BG_ThrowingBackGrenade(const playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5068, 0, "%s", "ps");
    return ps->throwBackGrenadeOwner != ENTITYNUM_NONE;
}

WeaponDef *__cdecl BG_LoadWeaponDef(const char *name)
{
#ifndef DEDICATED
    if (*(_BYTE *)fs_gameDirVar->current.integer || !useFastFile->current.enabled)
        return BG_LoadWeaponDef_LoadObj(name);
    else
        return BG_LoadWeaponDef_FastFile(name);
#else
    return BG_LoadWeaponDef_FastFile(name);
#endif
}

WeaponDef *__cdecl BG_LoadWeaponDef_FastFile(const char *name)
{
    if (!*name)
        return 0;
    return DB_FindXAssetHeader(ASSET_TYPE_WEAPON, name).weapon;
}

void __cdecl BG_AssertOffhandIndexOrNone(uint32_t offHandIndex)
{
    WeaponDef *WeaponDef; // eax
    const char *v2; // eax

    if (offHandIndex && BG_GetWeaponDef(offHandIndex)->offhandClass == OFFHAND_CLASS_NONE && !alwaysfails)
    {
        WeaponDef = BG_GetWeaponDef(offHandIndex);
        v2 = va(
            "Weapon #%d (%s) expected to be offhand weapon or no weapon at all, but is not.",
            offHandIndex,
            WeaponDef->szInternalName);
        MyAssertHandler(".\\bgame\\bg_weapons.cpp", 5118, 0, v2);
    }
}

void __cdecl BG_StringCopy(uint8_t *member, const char *keyValue)
{
    char v2; // al

    do
    {
        v2 = *keyValue;
        *member++ = *keyValue++;
    } while (v2);
}

int BG_ValidateWeaponNumberOffhand(unsigned int weaponIndex)
{
    int result; // r3
    OffhandClass offhandClass; // r11

    if (weaponIndex >= bg_lastParsedWeaponIndex + 1)
        return 0;
    if (!weaponIndex)
        return 1;
    if (BG_GetWeaponDef(weaponIndex)->offhandClass == OFFHAND_CLASS_NONE)
        return 0;
    offhandClass = BG_GetWeaponDef(weaponIndex)->offhandClass;
    result = 0;
    if (offhandClass < OFFHAND_CLASS_COUNT)
        return 1;
    return result;
}