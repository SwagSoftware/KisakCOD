#include <qcommon/qcommon.h>

#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>

#include <xanim/dobj.h>
#include <DynEntity/DynEntity_client.h>
#include <stringed/stringed_hooks.h>
#include <aim_assist/aim_assist.h>
#include <script/scr_const.h>
#include <xanim/dobj_utils.h>
#include <gfx_d3d/r_scene.h>
#include <sound/snd_public.h>
#include <qcommon/cmd.h>
#include <server_mp/server.h>
#include <EffectsCore/fx_system.h>
#include <game/bullet.h>
#include <game_mp/g_main_mp.h>

const float MYLERP_START = 0.3f;
const float MYLERP_END = 0.1f;

int removeMeWhenMPStopsCrashingInHere;

int g_animRateOffsets[33] =
{
  -1,
  -1,
  -1,
  -1,
  888,
  -1,
  -1,
  896,
  900,
  904,
  912,
  920,
  928,
  936,
  956,
  932,
  944,
  940,
  952,
  948,
  960,
  964,
  968,
  972,
  976,
  -1,
  980,
  992,
  -1,
  -1,
  -1,
  -1,
  -1
}; // idb

bool __cdecl CG_JavelinADS(int localClientNum)
{
    int weapIdx; // [esp+4h] [ebp-Ch]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapIdx = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
    if (weapIdx <= 0)
        return 0;
    if (cgArray[0].predictedPlayerState.fWeaponPosFrac == 1.0)
        return BG_GetWeaponDef(weapIdx)->overlayInterface == WEAPOVERLAYINTERFACE_JAVELIN;
    return 0;
}

int __cdecl CG_WeaponDObjHandle(int weaponNum)
{
    return weaponNum + 1024;
}

void __cdecl CG_RegisterWeapon(int localClientNum, unsigned int weaponNum)
{
    unsigned int NumWeapons; // eax
    char *v3; // eax
    __int64 _C; // [esp+Ch] [ebp-34h]
    const char *blendTime; // [esp+14h] [ebp-2Ch]
    weaponInfo_s *weapInfo; // [esp+18h] [ebp-28h]
    unsigned int dobjHandle; // [esp+1Ch] [ebp-24h]
    unsigned __int8 boneIndex; // [esp+23h] [ebp-1Dh] BYREF
    DObj_s *obj; // [esp+24h] [ebp-1Ch]
    int tagIndex; // [esp+28h] [ebp-18h]
    WeaponDef *weapDef; // [esp+2Ch] [ebp-14h]
    DObjModel_s dobjModels[2]; // [esp+30h] [ebp-10h] BYREF

    removeMeWhenMPStopsCrashingInHere = weaponNum;
    if (weaponNum)
    {
        if (weaponNum >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                ".\\cgame\\cg_weapons.cpp",
                615,
                0,
                "weaponNum doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weaponNum,
                NumWeapons);
        }
        if ((int)(weaponNum + 4) > 132)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 618, 0, "%s", "weaponNum + WEAPON_HINT_OFFSET <= LAST_WEAPON_HINT");
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        weapInfo = &cg_weaponsArray[0][weaponNum];
        weapDef = BG_GetWeaponDef(weaponNum);
        if (!weapInfo->registered)
        {
            SCR_UpdateLoadScreen();
            memset((unsigned __int8 *)weapInfo, 0, sizeof(weaponInfo_s));
            weapInfo->registered = 1;
            //weapInfo->item = (const gitem_s *)(4 * weaponNum + 9917736);
            weapInfo->item = &bg_itemlist[4 * weaponNum];
            weapInfo->iPrevAnim = -1;
            if (weapDef->gunXModel[0] && weapDef->handXModel)
            {
                dobjModels[0].boneName = 0;
                dobjModels[1].boneName = scr_const.tag_weapon;
                dobjModels[0].ignoreCollision = 0;
                dobjModels[1].ignoreCollision = 0;
                dobjModels[0].model = weapDef->handXModel;
                dobjModels[1].model = weapDef->gunXModel[0];
                if (weapInfo->tree)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 649, 0, "%s", "!weapInfo->tree");
                weapInfo->tree = CG_CreateWeaponViewModelXAnim(weapDef);
                if (!dobjModels[0].model)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 652, 0, "%s", "dobjModels[0].model");
                if (!dobjModels[1].model)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 653, 0, "%s", "dobjModels[1].model");
                dobjHandle = CG_WeaponDObjHandle(weaponNum);
                obj = Com_ClientDObjCreate(dobjModels, 2u, weapInfo->tree, dobjHandle, localClientNum);
                weapInfo->viewModelDObj = obj;
                weapInfo->handModel = weapDef->handXModel;
                weapInfo->weapModelIdx = 0;
                XAnimClearTreeGoalWeights(weapInfo->tree, 0, 0.0);
                XAnimSetGoalWeight(obj, 0, 1.0, 0.0, 1.0, 0, 1u, 0);
                XAnimSetGoalWeight(obj, 1u, 1.0, 0.0, 1.0, 0, 1u, 0);
                if (*weapDef->szXAnims[32])
                {
                    XAnimSetGoalWeight(obj, 0x20u, 1.0, 0.0, 0.0, 0, 1u, 0);
                    XAnimSetTime(weapInfo->tree, 32, 1.0);
                }
                for (tagIndex = 0; tagIndex < 8 && weapDef->hideTags[tagIndex]; ++tagIndex)
                {
                    boneIndex = -2;
                    if (DObjGetBoneIndex(obj, weapDef->hideTags[tagIndex], &boneIndex))
                    {
                        weapInfo->partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
                    }
                    else
                    {
                        blendTime = weapDef->szInternalName;
                        v3 = SL_ConvertToString(weapDef->hideTags[tagIndex]);
                        Com_PrintError(14, "CG_RegisterWeapon: No such bone tag (%s) for weapon (%s)\n", v3, blendTime);
                    }
                }
                DObjSetHidePartBits(obj, weapInfo->partBits);
                DObjUpdateClientInfo(weapInfo->viewModelDObj, 0.050000001, 0);
            }
            if (weapDef->hudIcon)
                cgMedia.stanceMaterials[weaponNum - 129] = weapDef->hudIcon;
            else
                cgMedia.stanceMaterials[weaponNum - 129] = 0;
            weapInfo->translatedDisplayName = SEH_StringEd_GetString(weapDef->szDisplayName);
            if (!weapInfo->translatedDisplayName)
            {
                if (loc_warnings->current.enabled)
                {
                    if (loc_warningsAsErrors->current.enabled)
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate display name \"%s\"",
                            weapDef->szInternalName,
                            weapDef->szDisplayName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate display name \"%s\"\n",
                            weapDef->szInternalName,
                            weapDef->szDisplayName);
                }
                weapInfo->translatedDisplayName = weapDef->szDisplayName;
            }
            weapInfo->translatedModename = SEH_StringEd_GetString(weapDef->szModeName);
            if (!weapInfo->translatedModename)
            {
                if (loc_warnings->current.enabled)
                {
                    if (loc_warningsAsErrors->current.enabled)
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate mode name \"%s\"",
                            weapDef->szInternalName,
                            weapDef->szModeName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate mode name \"%s\"\n",
                            weapDef->szInternalName,
                            weapDef->szModeName);
                }
                weapInfo->translatedModename = weapDef->szModeName;
            }
            weapInfo->translatedAIOverlayDescription = SEH_StringEd_GetString(weapDef->szOverlayName);
            if (!weapInfo->translatedAIOverlayDescription)
            {
                if (loc_warnings->current.enabled)
                {
                    if (loc_warningsAsErrors->current.enabled)
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate AI overlay description \"%s\"",
                            weapDef->szInternalName,
                            weapDef->szOverlayName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate AI overlay description \"%s\"\n",
                            weapDef->szInternalName,
                            weapDef->szOverlayName);
                }
                weapInfo->translatedAIOverlayDescription = weapDef->szOverlayName;
            }
        }
    }
}

XAnimTree_s *__cdecl CG_CreateWeaponViewModelXAnim(WeaponDef *weapDef)
{
    int v2; // [esp+0h] [ebp-14h]
    signed int animIndex; // [esp+8h] [ebp-Ch]
    XAnimTree_s *pAnimTree; // [esp+Ch] [ebp-8h]
    XAnim_s *pAnims; // [esp+10h] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 554, 0, "%s", "weapDef");
    pAnims = XAnimCreateAnims("VIEWMODEL", 0x21u, (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    if (!pAnims)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 558, 0, "%s", "pAnims");
    XAnimBlend(pAnims, 0, "root", 1u, 0x20u, 0);
    for (animIndex = 1; animIndex < 33; ++animIndex)
    {
        if (*weapDef->szXAnims[animIndex])
            v2 = animIndex;
        else
            v2 = 1;
        BG_CreateXAnim(pAnims, animIndex, (char *)weapDef->szXAnims[v2]);
    }
    pAnimTree = XAnimCreateTree(pAnims, (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    if (!pAnimTree)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 573, 0, "%s", "pAnimTree");
    if (!weapDef->szXAnims[1] || !*weapDef->szXAnims[1])
        Com_Error(ERR_DROP, "CG_RegisterWeapon: No idle anim specified for [%s]", weapDef->szDisplayName);
    if (*weapDef->szXAnims[31] && XAnimIsLooped(pAnims, 0x1Fu))
        Com_Error(ERR_DROP, "CG_RegisterWeapon: ADS anim [%s] cannot be looping", weapDef->szXAnims[31]);
    if (*weapDef->szXAnims[32] && XAnimIsLooped(pAnims, 0x20u))
        Com_Error(ERR_DROP, "CG_RegisterWeapon: ADS anim [%s] cannot be looping", weapDef->szXAnims[32]);
    return pAnimTree;
}

void __cdecl CG_UpdateWeaponViewmodels(int localClientNum)
{
    unsigned int weaponIndex; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (weaponIndex = 1; weaponIndex < BG_GetNumWeapons(); ++weaponIndex)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (cg_weaponsArray[0][weaponIndex].weapModelIdx != cgArray[0].nextSnap->ps.weaponmodels[weaponIndex])
            ChangeViewmodelDobj(
                localClientNum,
                weaponIndex,
                cgArray[0].nextSnap->ps.weaponmodels[weaponIndex],
                cg_weaponsArray[0][weaponIndex].handModel,
                cg_weaponsArray[0][weaponIndex].gogglesModel,
                cg_weaponsArray[0][weaponIndex].rocketModel,
                cg_weaponsArray[0][weaponIndex].knifeModel,
                1);
    }
}

void __cdecl ChangeViewmodelDobj(
    int localClientNum,
    unsigned int weaponNum,
    unsigned __int8 weaponModel,
    XModel *newHands,
    XModel *newGoggles,
    XModel *newRocket,
    XModel *newKnife,
    bool updateClientInfo)
{
    unsigned int NumWeapons; // eax
    weaponInfo_s *weapInfo; // [esp+8h] [ebp-34h]
    unsigned int dobjHandle; // [esp+Ch] [ebp-30h]
    int mdlIdx; // [esp+10h] [ebp-2Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-28h]
    XAnimTree_s *pAnimTree; // [esp+18h] [ebp-24h]
    DObjModel_s dobjModels[4]; // [esp+1Ch] [ebp-20h] BYREF

    if (weaponNum)
    {
        if (weaponNum >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                ".\\cgame\\cg_weapons.cpp",
                760,
                0,
                "weaponNum doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weaponNum,
                NumWeapons);
        }
        if (!newHands)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 761, 0, "%s", "newHands");
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        weapInfo = &cg_weaponsArray[0][weaponNum];
        weapDef = BG_GetWeaponDef(weaponNum);
        if (weapDef->gunXModel[weaponModel])
        {
            weapInfo->handModel = newHands;
            weapInfo->gogglesModel = newGoggles;
            weapInfo->rocketModel = newRocket;
            weapInfo->knifeModel = newKnife;
            weapInfo->weapModelIdx = weaponModel;
            dobjHandle = CG_WeaponDObjHandle(weaponNum);
            if (weapInfo->viewModelDObj)
            {
                pAnimTree = DObjGetTree(weapInfo->viewModelDObj);
                Com_SafeClientDObjFree(dobjHandle, localClientNum);
            }
            else
            {
                if (weapInfo->tree)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 785, 0, "%s", "!weapInfo->tree");
                pAnimTree = CG_CreateWeaponViewModelXAnim(weapDef);
                weapInfo->tree = pAnimTree;
            }
            if (!pAnimTree)
                MyAssertHandler(".\\cgame\\cg_weapons.cpp", 788, 0, "%s", "pAnimTree");
            dobjModels[0].boneName = 0;
            dobjModels[0].ignoreCollision = 0;
            dobjModels[0].model = weapInfo->handModel;
            dobjModels[1].boneName = scr_const.tag_weapon;
            dobjModels[1].ignoreCollision = 0;
            dobjModels[1].model = weapDef->gunXModel[weaponModel];
            mdlIdx = 2;
            if (weapInfo->gogglesModel)
            {
                if (overrideNVGModelWithKnife->current.enabled)
                    dobjModels[2].boneName = scr_const.tag_gasmask2;
                else
                    dobjModels[2].boneName = scr_const.tag_gasmask;
                dobjModels[2].model = weapInfo->gogglesModel;
                dobjModels[2].ignoreCollision = 0;
                mdlIdx = 3;
            }
            if (weapInfo->rocketModel)
            {
                dobjModels[mdlIdx].boneName = scr_const.tag_clip;
                dobjModels[mdlIdx].ignoreCollision = 0;
                dobjModels[mdlIdx++].model = weapInfo->rocketModel;
            }
            if (weapInfo->knifeModel)
            {
                dobjModels[mdlIdx].boneName = scr_const.tag_knife_attach;
                dobjModels[mdlIdx].ignoreCollision = 0;
                dobjModels[mdlIdx++].model = weapInfo->knifeModel;
            }
            if (mdlIdx > 4)
                MyAssertHandler(".\\cgame\\cg_weapons.cpp", 830, 0, "%s", "mdlIdx <= MYMODELCOUNT");
            weapInfo->viewModelDObj = Com_ClientDObjCreate(dobjModels, mdlIdx, pAnimTree, dobjHandle, localClientNum);
            DObjSetHidePartBits(weapInfo->viewModelDObj, weapInfo->partBits);
            if (updateClientInfo)
                DObjUpdateClientInfo(weapInfo->viewModelDObj, 0.050000001, 0);
        }
    }
}

void __cdecl CG_UpdateHandViewmodels(int localClientNum, XModel *handModel)
{
    unsigned int weaponIndex; // [esp+Ch] [ebp-4h]

    if (!handModel)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 967, 0, "%s", "handModel");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (weaponIndex = 1; weaponIndex < BG_GetNumWeapons(); ++weaponIndex)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (cg_weaponsArray[0][weaponIndex].handModel != handModel)
            ChangeViewmodelDobj(
                localClientNum,
                weaponIndex,
                cgArray[0].predictedPlayerState.weaponmodels[weaponIndex],
                handModel,
                cg_weaponsArray[0][weaponIndex].gogglesModel,
                cg_weaponsArray[0][weaponIndex].rocketModel,
                cg_weaponsArray[0][weaponIndex].knifeModel,
                1);
    }
}

void __cdecl CG_RegisterItemVisuals(int localClientNum, unsigned int weapIdx)
{
    int modelIdx; // [esp+4h] [ebp-4h]

    if (weapIdx >= 0x80)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            1194,
            0,
            "weapIdx doesn't index MAX_WEAPONS\n\t%i not in [0, %i)",
            weapIdx,
            128);
    for (modelIdx = 0; modelIdx < 16; ++modelIdx)
    {
        if (bg_itemlist[128 * modelIdx + weapIdx].giType != IT_WEAPON)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 1199, 0, "%s", "item->giType == IT_WEAPON");
    }
    CG_RegisterWeapon(localClientNum, weapIdx);
}

void __cdecl CG_RegisterItems(int localClientNum)
{
    char v1; // al
    char *v2; // [esp+8h] [ebp-98h]
    char *ConfigString; // [esp+Ch] [ebp-94h]
    char items[132]; // [esp+10h] [ebp-90h] BYREF
    int i; // [esp+98h] [ebp-8h]
    int digit; // [esp+9Ch] [ebp-4h]

    ConfigString = CL_GetConfigString(localClientNum, 0x90Au);
    v2 = items;
    do
    {
        v1 = *ConfigString;
        *v2++ = *ConfigString++;
    } while (v1);
    for (i = 1; i < 128; ++i)
    {
        digit = items[i / 4];
        if (digit > 57)
            digit -= 87;
        else
            digit -= 48;
        if ((digit & (1 << (i & 3))) != 0)
            CG_RegisterItemVisuals(localClientNum, i);
    }
}

void __cdecl CG_HoldBreathInit(cg_s *cgameGlob)
{
    cgameGlob->holdBreathTime = -1;
    cgameGlob->holdBreathInTime = 0;
    cgameGlob->holdBreathDelay = 0;
    cgameGlob->holdBreathFrac = 0.0;
}

void __cdecl CG_UpdateViewModelPose(const DObj_s* obj, int localClientNum)
{
    if (obj)
        DObjClearSkel(obj);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    AxisToAngles((mat3x3 &)cgArray[0].viewModelAxis, cgArray[0].viewModelPose.angles);
    cgArray[0].viewModelPose.origin[0] = cgArray[0].viewModelAxis[3][0];
    cgArray[0].viewModelPose.origin[1] = cgArray[0].viewModelAxis[3][1];
    cgArray[0].viewModelPose.origin[2] = cgArray[0].viewModelAxis[3][2];
}

bool __cdecl CG_IsPlayerCrouching(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsCrouchingAnim(ci, cent->nextState.legsAnim);
}

bool __cdecl CG_IsPlayerProne(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsProneAnim(ci, cent->nextState.legsAnim);
}

bool __cdecl CG_IsPlayerADS(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsAds(ci, cent->nextState.legsAnim);
}

void __cdecl CG_GuessSpreadForWeapon(
    int localClientNum,
    const centity_s *cent,
    const WeaponDef *weapDef,
    float *minSpread,
    float *maxSpread)
{
    clientInfo_t *ci; // [esp+4h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cent->nextState.number >= 0x40u)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            2551,
            0,
            "cent->nextState.number doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cent->nextState.number,
            64);
    ci = &cgArray[0].bgs.clientinfo[cent->nextState.number];
    if (CG_IsPlayerProne(ci, cent))
    {
        *minSpread = weapDef->fHipSpreadProneMin;
        *maxSpread = weapDef->hipSpreadProneMax;
    }
    else if (CG_IsPlayerCrouching(ci, cent))
    {
        *minSpread = weapDef->fHipSpreadDuckedMin;
        *maxSpread = weapDef->hipSpreadDuckedMax;
    }
    else
    {
        *minSpread = weapDef->fHipSpreadStandMin;
        *maxSpread = weapDef->hipSpreadStandMax;
    }
}

void __cdecl CG_GetPlayerViewOrigin(int localClientNum, const playerState_s *ps, float *origin)
{
    DObj_s *obj; // [esp+0h] [ebp-8h]
    centity_s *turretEnt; // [esp+4h] [ebp-4h]

    if ((ps->eFlags & 0x300) != 0)
    {
        if (ps->viewlocked == PLAYERVIEWLOCK_NONE)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2677, 0, "%s", "ps->viewlocked");
        if (ps->viewlocked_entNum == 1023)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2678, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
        turretEnt = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
        obj = Com_GetClientDObj(turretEnt->nextState.number, 0);
        if (!obj)
            Com_Error(ERR_DROP, "CG_GetPlayerViewOrigin: Unable to get DObj for turret entity %i", turretEnt->nextState.number);
        if (!CG_DObjGetWorldTagPos(&turretEnt->pose, obj, scr_const.tag_player, origin))
            Com_Error(ERR_DROP, "CG_GetPlayerViewOrigin: Couldn't find [tag_player] on turret");
    }
    else
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        BG_GetPlayerViewOrigin(ps, origin, cgArray[0].time);
    }
}
void __cdecl CG_AddPlayerWeapon(
    int localClientNum,
    const GfxScaledPlacement* placement,
    const playerState_s* ps,
    centity_s* cent,
    int bDrawGun)
{
    unsigned int v5; // eax
    unsigned int fLeanDist; // [esp+Ch] [ebp-48h]
    bool v7; // [esp+10h] [ebp-44h]
    BOOL v8; // [esp+14h] [ebp-40h]
    bool v9; // [esp+18h] [ebp-3Ch]
    snapshot_s* nextSnap; // [esp+28h] [ebp-2Ch]
    const weaponInfo_s* weapInfo; // [esp+30h] [ebp-24h]
    int weaponNum; // [esp+34h] [ebp-20h]
    float lightingOrigin[3]; // [esp+44h] [ebp-10h] BYREF
    const WeaponDef* weapDef; // [esp+50h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    nextSnap = cgArray[0].nextSnap;
    v9 = (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum;
    v8 = v9 && !cgArray[0].renderingThirdPerson;
    if (ps)
        weaponNum = BG_GetViewmodelWeaponIndex(ps);
    else
        weaponNum = cent->nextState.weapon;
    if (weaponNum > 0 && (cent->nextState.lerp.eFlags & 0x300) == 0)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        weapInfo = &cg_weaponsArray[0][weaponNum];
        if (!weapInfo->viewModelDObj)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2907, 0, "%s", "weapInfo->viewModelDObj");
        if (ps)
        {
            UnitQuatToAxis(placement->base.quat, (mat3x3&)cgArray[0].viewModelAxis);
            cgArray[0].viewModelAxis[3][0] = placement->base.origin[0];
            cgArray[0].viewModelAxis[3][1] = placement->base.origin[1];
            cgArray[0].viewModelAxis[3][2] = placement->base.origin[2];
            CG_UpdateViewModelPose(weapInfo->viewModelDObj, localClientNum);
            if (bDrawGun)
            {
                lightingOrigin[0] = ps->origin[0];
                lightingOrigin[1] = ps->origin[1];
                lightingOrigin[2] = ps->origin[2];
                lightingOrigin[2] = lightingOrigin[2] + ps->viewHeightCurrent;
                AddLeanToPosition(lightingOrigin, ps->viewangles[1], ps->leanf, 16.0, 20.0);
                R_AddDObjToScene(weapInfo->viewModelDObj, &cgArray[0].viewModelPose, 0x3FFu, 3u, lightingOrigin, 0.0);
                weapDef = BG_GetWeaponDef(weaponNum);
                if (!weapDef)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2930, 0, "%s", "weapDef");
                v7 = CG_LookingThroughNightVision(localClientNum) && weapDef->laserSightDuringNightvision;
                if (cg_laserForceOn->current.enabled || v7)
                    CG_Laser_Add(
                        cent,
                        weapInfo->viewModelDObj,
                        &cgArray[0].viewModelPose,
                        cgArray[0].refdef.viewOffset,
                        LASER_OWNER_PLAYER);
                cgArray[0].refdef.dof.viewModelStart = (weapDef->adsDofStart - ps->dofViewmodelStart) * ps->fWeaponPosFrac
                    + ps->dofViewmodelStart;
                cgArray[0].refdef.dof.viewModelEnd = (weapDef->adsDofEnd - ps->dofViewmodelEnd) * ps->fWeaponPosFrac
                    + ps->dofViewmodelEnd;
            }
            HoldBreathUpdate(localClientNum);
        }
        if (cent->bMuzzleFlash && (!v8 || ps))
        {
            cent->bMuzzleFlash = 0;
            if (bDrawGun)
            {
                if (ps)
                {
                    fLeanDist = scr_const.tag_flash;
                    v5 = CG_WeaponDObjHandle(weaponNum);
                    WeaponFlash(localClientNum, v5, weaponNum, 1, fLeanDist);
                }
                else
                {
                    WeaponFlash(localClientNum, cent->nextState.number, weaponNum, 0, scr_const.tag_flash);
                }
            }
        }
    }
}

void __cdecl WeaponFlash(
    int localClientNum,
    unsigned int dobjHandle,
    unsigned int weaponNum,
    int bViewFlash,
    unsigned int flashTag)
{
    const FxEffectDef *viewFlashEffect; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponNum);
    if (bViewFlash)
        viewFlashEffect = weapDef->viewFlashEffect;
    else
        viewFlashEffect = weapDef->worldFlashEffect;
    if (viewFlashEffect)
        CG_PlayBoltedEffect(localClientNum, viewFlashEffect, dobjHandle, flashTag);
}

void __cdecl HoldBreathUpdate(int localClientNum)
{
    float deltaTime; // [esp+10h] [ebp-10h]
    int playbackId; // [esp+14h] [ebp-Ch]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].holdBreathDelay > 0)
        cgArray[0].holdBreathDelay -= cgArray[0].frametime;
    if ((cgArray[0].predictedPlayerState.weapFlags & 4) != 0)
    {
        deltaTime = (double)cgArray[0].frametime * 0.001000000047497451;
        cgArray[0].holdBreathFrac = DiffTrack(
            1.0,
            cgArray[0].holdBreathFrac,
            player_breath_snd_lerp->current.value,
            deltaTime);
        if (cgArray[0].holdBreathTime >= 0)
        {
            if (cgArray[0].holdBreathTime > cgArray[0].holdBreathInTime)
                CG_PlayClientSoundAlias(localClientNum, cgMedia.playerHeartBeatSound);
        }
        else
        {
            cgArray[0].holdBreathTime = 0;
            if (cgArray[0].holdBreathDelay > 0)
            {
                cgArray[0].holdBreathInTime = 0;
            }
            else
            {
                playbackId = CG_PlayClientSoundAlias(localClientNum, cgMedia.playerBreathInSound);
                SND_GetKnownLength(playbackId, &cgArray[0].holdBreathInTime);
                cgArray[0].holdBreathDelay = (int)(player_breath_snd_delay->current.value * 1000.0);
            }
        }
        cgArray[0].holdBreathTime += cgArray[0].frametime;
    }
    else
    {
        if (cgArray[0].holdBreathTime >= 0)
        {
            cgArray[0].holdBreathTime += cgArray[0].frametime;
            if (cgArray[0].holdBreathTime <= (int)(player_breath_hold_time->current.value * 1000.0))
            {
                if (cgArray[0].holdBreathDelay <= 0)
                {
                    CG_PlayClientSoundAlias(localClientNum, cgMedia.playerBreathOutSound);
                    cgArray[0].holdBreathDelay = (int)(player_breath_snd_delay->current.value * 1000.0);
                }
            }
            else
            {
                CG_PlayClientSoundAlias(localClientNum, cgMedia.playerBreathGaspSound);
            }
        }
        cgArray[0].holdBreathTime = -1;
        cgArray[0].holdBreathInTime = 0;
        cgArray[0].holdBreathFrac = 0.0;
    }
    HoldBreathSoundLerp(localClientNum, cgArray[0].holdBreathFrac);
}

void __cdecl HoldBreathSoundLerp(int localClientNum, float lerp)
{
    int channelIndex; // [esp+0h] [ebp-10Ch]
    float channelVolumes[64]; // [esp+Ch] [ebp-100h] BYREF

    if (lerp == 0.0)
    {
        SND_DeactivateChannelVolumes(1, 0);
    }
    else
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        for (channelIndex = 0; channelIndex < SND_GetEntChannelCount(); ++channelIndex)
            channelVolumes[channelIndex] = (cgsArray[0].holdBreathParams.sound.channelvolume[channelIndex] - 1.0) * lerp + 1.0;
        SND_SetChannelVolumes(1, channelVolumes, 0);
    }
}

void __cdecl CG_UpdateViewWeaponAnim(int localClientNum)
{
    float dtime; // [esp+8h] [ebp-14h]
    weaponInfo_s* weapInfo; // [esp+Ch] [ebp-10h]
    int weaponIndex; // [esp+14h] [ebp-8h]
    playerState_s* ps; // [esp+18h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ps = &cgArray->predictedPlayerState;
    if (cgArray->predictedPlayerState.pm_type < 7)
    {
        weaponIndex = BG_GetViewmodelWeaponIndex(ps);
        if (weaponIndex > 0)
        {
            CG_RegisterWeapon(localClientNum, weaponIndex);
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1095,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            weapInfo = &cg_weaponsArray[0][weaponIndex];
            if (!weapInfo->viewModelDObj)
                MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3066, 0, "%s", "weapInfo->viewModelDObj");
            UpdateViewmodelAttachments(localClientNum, weaponIndex, ps->weaponmodels[weaponIndex], weapInfo);
            WeaponRunXModelAnims(localClientNum, ps, weapInfo);
            dtime = (double)cgArray->frametime * 0.001000000047497451;
            DObjUpdateClientInfo(weapInfo->viewModelDObj, dtime, 1);
            ProcessWeaponNoteTracks(localClientNum, ps);
        }
    }
    else
    {
        ResetWeaponAnimTrees(localClientNum, ps);
    }
}

void __cdecl WeaponRunXModelAnims(int localClientNum, const playerState_s* ps, weaponInfo_s* weapInfo)
{
    BOOL v3; // [esp+14h] [ebp-34h]
    BOOL v4; // [esp+18h] [ebp-30h]
    BOOL v5; // [esp+1Ch] [ebp-2Ch]
    DObj_s* obj; // [esp+24h] [ebp-24h]
    float transitionTime; // [esp+28h] [ebp-20h]
    int weaponIndex; // [esp+38h] [ebp-10h]
    signed int i; // [esp+3Ch] [ebp-Ch]
    XAnimTree_s* pAnimTree; // [esp+40h] [ebp-8h]
    const WeaponDef* weapDef; // [esp+44h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    obj = weapInfo->viewModelDObj;
    if (!weapInfo->viewModelDObj)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 329, 0, "%s", "obj");
    pAnimTree = DObjGetTree(obj);
    weaponIndex = BG_GetViewmodelWeaponIndex(ps);
    if (weaponIndex <= 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 335, 0, "%s", "weaponIndex > WP_NONE");
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->aimDownSight)
    {
        v5 = ps->weaponstate == 7 && ps->weaponTime - weapDef->iPositionReloadTransTime > 0;
        v4 = (ps->pm_flags & 0x10) != 0 && (ps->weapFlags & 2) == 0;
        v3 = !v5 && v4;
        PlayADSAnim(ps->fWeaponPosFrac, weaponIndex, obj, 32 - v3);
    }
    else if (*weapDef->szXAnims[32])
    {
        PlayADSAnim(0.0, weaponIndex, obj, 32);
    }
    if (ps->weapAnim != weapInfo->iPrevAnim || weaponIndex != cgArray->prevViewmodelWeapon)
    {
        transitionTime = 0.0;
        switch (ps->weapAnim & 0xFFFFFDFF)
        {
        case 0u:
        case 1u:
            i = 1;
            break;
        case 2u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 3, transitionTime);
            goto LABEL_64;
        case 3u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 5, transitionTime);
            goto LABEL_64;
        case 4u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 6, transitionTime);
            goto LABEL_64;
        case 5u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 28, transitionTime);
            goto LABEL_64;
        case 6u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 29, transitionTime);
            goto LABEL_64;
        case 7u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 30, transitionTime);
            goto LABEL_64;
        case 8u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 7, transitionTime);
            goto LABEL_64;
        case 9u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 8, transitionTime);
            goto LABEL_64;
        case 0xAu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 15, transitionTime);
            goto LABEL_64;
        case 0xBu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 13, transitionTime);
            goto LABEL_64;
        case 0xCu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 14, transitionTime);
            goto LABEL_64;
        case 0xDu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 9, transitionTime);
            goto LABEL_64;
        case 0xEu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 10, transitionTime);
            goto LABEL_64;
        case 0xFu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 11, transitionTime);
            goto LABEL_64;
        case 0x10u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 12, transitionTime);
            goto LABEL_64;
        case 0x11u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 17, transitionTime);
            goto LABEL_64;
        case 0x12u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 16, transitionTime);
            goto LABEL_64;
        case 0x13u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 19, transitionTime);
            goto LABEL_64;
        case 0x14u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 18, transitionTime);
            goto LABEL_64;
        case 0x15u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 21, transitionTime);
            goto LABEL_64;
        case 0x16u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 20, transitionTime);
            goto LABEL_64;
        case 0x17u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 22, transitionTime);
            goto LABEL_64;
        case 0x18u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 23, transitionTime);
            goto LABEL_64;
        case 0x19u:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 24, transitionTime);
            goto LABEL_64;
        case 0x1Au:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 4, transitionTime);
            goto LABEL_64;
        case 0x1Bu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 25, transitionTime);
            goto LABEL_64;
        case 0x1Cu:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 26, transitionTime);
            goto LABEL_64;
        case 0x1Du:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 27, transitionTime);
            goto LABEL_64;
        default:
            StartWeaponAnim(localClientNum, weaponIndex, obj, 1, transitionTime);
            Com_Printf(19, "WeaponRunXModelAnims: Unknown weapon animation %i\n", ps->weapAnim & 0xFFFFFDFF);
        LABEL_64:
            weapInfo->iPrevAnim = ps->weapAnim;
            cgArray[0].prevViewmodelWeapon = weaponIndex;
            return;
        }
        while (1)
        {
            if (i >= 31)
            {
                if (ps->ammoclip[BG_ClipForWeapon(weaponIndex)])
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 1, transitionTime);
                else
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 2, transitionTime);
                goto LABEL_64;
            }
            if (!XAnimHasFinished(pAnimTree, i))
            {
                weapInfo->iPrevAnim = -1;
                if ((ps->weapAnim & 1) == 0)
                    return;
                transitionTime = 0.5;
            }
            ++i;
        }
    }
}
void __cdecl StartWeaponAnim(
    int localClientNum,
    unsigned int weaponNum,
    DObj_s* obj,
    int animIndex,
    float transitionTime)
{
    float rate; // [esp+20h] [ebp-1Ch]
    XAnim_s* anims; // [esp+2Ch] [ebp-10h]
    signed int i; // [esp+30h] [ebp-Ch]
    WeaponDef* weapDef; // [esp+34h] [ebp-8h]

    if (animIndex <= 0 || animIndex >= 31)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            159,
            0,
            "%s",
            "(animIndex > WEAP_ANIM_VIEWMODEL_START) && (animIndex < WEAP_ANIM_VIEWMODEL_END)");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1095,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    anims = XAnimGetAnims(cg_weaponsArray[0][weaponNum].tree);
    if (!anims)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 163, 0, "%s", "anims");
    weapDef = BG_GetWeaponDef(weaponNum);
    rate = GetWeaponAnimRate(weapDef, anims, animIndex);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.weaponstate == 7
        || cgArray[0].predictedPlayerState.weaponstate == 9
        || cgArray[0].predictedPlayerState.weaponstate == 11
        || cgArray[0].predictedPlayerState.weaponstate == 10
        || cgArray[0].predictedPlayerState.weaponstate == 8)
        && (cgArray[0].predictedPlayerState.perks & 4) != 0)
    {
        if (perk_weapReloadMultiplier->current.value == 0.0)
            rate = 1000.0;
        else
            rate = rate / perk_weapReloadMultiplier->current.value;
    }
    else if (cgArray[0].predictedPlayerState.weaponstate == 6 && (cgArray[0].predictedPlayerState.perks & 8) != 0)
    {
        if (perk_weapRateMultiplier->current.value == 0.0)
            rate = 1000.0;
        else
            rate = rate / perk_weapRateMultiplier->current.value;
    }
    for (i = 1; i < 31; ++i)
    {
        if (animIndex == i)
            XAnimSetGoalWeight(obj, i, 1.0, transitionTime, rate, 0, 1u, 1);
        else
            XAnimSetGoalWeight(obj, i, 0.0, transitionTime, 1.0, 0, 0, 0);
    }
}

double __cdecl GetWeaponAnimRate(WeaponDef *weapDef, XAnim_s *anims, unsigned int animIndex)
{
    int offset; // [esp+8h] [ebp-8h]
    int time; // [esp+Ch] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 129, 0, "%s", "weapDef");
    if (!anims)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 130, 0, "%s", "anims");
    if (animIndex > 0x20)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            131,
            0,
            "%s\n\t(animIndex) = %i",
            "(((animIndex >= 0) && (animIndex < NUM_WEAP_ANIMS)))",
            animIndex);
    offset = g_animRateOffsets[animIndex];
    if (offset < 0)
        return 1.0;
    time = *(int *)((char *)&weapDef->szInternalName + offset);
    if (time < 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 138, 0, "%s\n\t(time) = %i", "((time >= 0))", time);
    if (!time)
        return 0.0;
    return (float)((double)XAnimGetLengthMsec(anims, animIndex) / (double)time);
}

void __cdecl PlayADSAnim(float weaponPosFrac, int weaponNum, DObj_s *obj, int animIndex)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v5; // eax
    float time; // [esp+18h] [ebp-4h]

    if (animIndex != 31 && animIndex != 32)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            199,
            0,
            "%s",
            "(animIndex == WEAP_ANIM_ADS_UP) || (animIndex == WEAP_ANIM_ADS_DOWN)");
    if (animIndex == 31)
    {
        XAnimSetGoalWeight(obj, 0x1Fu, 1.0, 0.1, 0.0, 0, 1u, 0);
        XAnimSetGoalWeight(obj, 0x20u, 0.0, 0.1, 0.0, 0, 0, 0);
    }
    else
    {
        XAnimSetGoalWeight(obj, 0x1Fu, 0.0, 0.1, 0.0, 0, 0, 0);
        XAnimSetGoalWeight(obj, 0x20u, 1.0, 0.1, 0.0, 0, 1u, 0);
    }
    Tree = DObjGetTree(obj);
    XAnimSetTime(Tree, 0x1Fu, weaponPosFrac);
    time = 1.0 - weaponPosFrac;
    v5 = DObjGetTree(obj);
    XAnimSetTime(v5, 0x20u, time);
}

void __cdecl ResetWeaponAnimTrees(int localClientNum, const playerState_s *ps)
{
    DObj_s *obj; // [esp+1Ch] [ebp-Ch]
    XAnimTree_s *animTree; // [esp+20h] [ebp-8h]
    unsigned int weapIndex; // [esp+24h] [ebp-4h]

    for (weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        obj = cg_weaponsArray[0][weapIndex].viewModelDObj;
        if (obj)
        {
            animTree = DObjGetTree(obj);
            if (!animTree)
                MyAssertHandler(".\\cgame\\cg_weapons.cpp", 525, 0, "%s", "animTree");
            XAnimClearTreeGoalWeights(animTree, 0, 0.0);
            XAnimSetGoalWeight(obj, 0, 1.0, 0.0, 1.0, 0, 1u, 0);
            if (ps->ammoclip[BG_ClipForWeapon(weapIndex)])
                StartWeaponAnim(localClientNum, weapIndex, obj, 1, 0.0);
            else
                StartWeaponAnim(localClientNum, weapIndex, obj, 2, 0.0);
        }
    }
}

char __cdecl UpdateViewmodelAttachments(
    int localClientNum,
    unsigned int weaponNum,
    unsigned __int8 weaponModel,
    weaponInfo_s *weapInfo)
{
    XModel *newKnife; // [esp+0h] [ebp-14h]
    WeaponDef *weapDef; // [esp+8h] [ebp-Ch]
    XModel *newRocket; // [esp+10h] [ebp-4h]
    weaponInfo_s *weapInfoa; // [esp+28h] [ebp+14h]

    if (!weapInfo)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 902, 0, "%s", "weapInfo");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1095,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapInfoa = &cg_weaponsArray[0][weaponNum];
    newRocket = 0;
    newKnife = 0;
    weapDef = BG_GetWeaponDef(weaponNum);
    if (ViewmodelRocketShouldBeAttached(localClientNum, weapDef))
        newRocket = weapDef->rocketModel;
    if (ViewmodelKnifeShouldBeAttached(localClientNum, weapDef))
        newKnife = weapDef->knifeModel;
    if (!weapInfoa->gogglesModel && newRocket == weapInfoa->rocketModel && newKnife == weapInfoa->knifeModel)
        return 0;
    ChangeViewmodelDobj(localClientNum, weaponNum, weaponModel, weapInfoa->handModel, 0, newRocket, newKnife, 0);
    return 1;
}

bool __cdecl ViewmodelRocketShouldBeAttached(int localClientNum, WeaponDef* weapDef)
{
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 850, 0, "%s", "weapDef");
    if (!weapDef->rocketModel)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.ammoclip[BG_ClipForWeapon(cgArray[0].predictedPlayerState.weapon)])
        return 1;
    return (cgArray[0].predictedPlayerState.weaponstate == 7
        || cgArray[0].predictedPlayerState.weaponstate == 9
        || cgArray[0].predictedPlayerState.weaponstate == 11
        || cgArray[0].predictedPlayerState.weaponstate == 10
        || cgArray[0].predictedPlayerState.weaponstate == 8)
        && weapDef->iReloadTime - cgArray[0].predictedPlayerState.weaponTime > weapDef->reloadShowRocketTime;
}

bool __cdecl ViewmodelKnifeShouldBeAttached(int localClientNum, WeaponDef* weapDef)
{
    unsigned int anim; // [esp+4h] [ebp-8h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 881, 0, "%s", "weapDef");
    if (!weapDef->knifeModel)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    anim = cgArray[0].predictedPlayerState.weapAnim & 0xFFFFFDFF;
    return anim == 8 || anim == 9;
}

void __cdecl ProcessWeaponNoteTracks(int localClientNum, const playerState_s *predictedPlayerState)
{
    unsigned int NumWeapons; // eax
    int noteListSize; // [esp+0h] [ebp-14h]
    XAnimNotify_s *noteList; // [esp+4h] [ebp-10h] BYREF
    int weapIndex; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(predictedPlayerState);
    if (weapIndex)
    {
        if (weapIndex >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                ".\\cgame\\cg_weapons.cpp",
                1022,
                0,
                "weapIndex doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weapIndex,
                NumWeapons);
        }
        weapDef = BG_GetWeaponDef(weapIndex);
        noteListSize = DObjGetClientNotifyList(&noteList);
        for (i = 0; i < noteListSize; ++i)
        {
            if (I_stricmp(noteList[i].name, "end"))
            {
                if (I_stricmp(noteList[i].name, "NVG_on_powerup"))
                {
                    if (!I_stricmp(noteList[i].name, "NVG_off_powerdown"))
                        CG_PlayClientSoundAlias(localClientNum, cgMedia.nightVisionOff);
                }
                else
                {
                    CG_PlayClientSoundAlias(localClientNum, cgMedia.nightVisionOn);
                }
                PlayNoteMappedSoundAliases(localClientNum, noteList[i].name, weapDef);
            }
        }
    }
}

void __cdecl PlayNoteMappedSoundAliases(int localClientNum, const char *noteName, const WeaponDef *weapDef)
{
    char *soundName; // [esp+0h] [ebp-Ch]
    int mapIdx; // [esp+4h] [ebp-8h]
    unsigned int noteNameSL; // [esp+8h] [ebp-4h]

    if (weapDef->notetrackSoundMapKeys[0])
    {
        noteNameSL = SL_FindLowercaseString(noteName);
        if (noteNameSL)
        {
            for (mapIdx = 0; mapIdx < 16 && weapDef->notetrackSoundMapKeys[mapIdx]; ++mapIdx)
            {
                if (weapDef->notetrackSoundMapValues[mapIdx] && weapDef->notetrackSoundMapKeys[mapIdx] == noteNameSL)
                {
                    soundName = SL_ConvertToString(weapDef->notetrackSoundMapValues[mapIdx]);
                    if (soundName)
                        CG_PlayClientSoundAliasByName(localClientNum, soundName);
                }
            }
        }
    }
}
void __cdecl CG_AddViewWeapon(int localClientNum)
{
    double v1; // st7
    double v2; // st7
    int v3; // [esp+Ch] [ebp-12Ch]
    float* vGunSpeed; // [esp+10h] [ebp-128h]
    float* vGunOffset; // [esp+14h] [ebp-124h]
    float* vLastMoveAng; // [esp+18h] [ebp-120h]
    playerEntity_t* pe; // [esp+2Ch] [ebp-10Ch]
    weaponState_t ws; // [esp+30h] [ebp-108h] BYREF
    cg_s* cgameGlob; // [esp+88h] [ebp-B0h]
    int drawgun; // [esp+8Ch] [ebp-ACh]
    float vAxis2[3][3]; // [esp+90h] [ebp-A8h] BYREF
    float angles[3]; // [esp+B4h] [ebp-84h] BYREF
    GfxScaledPlacement placement; // [esp+C0h] [ebp-78h] BYREF
    int weaponIndex; // [esp+E0h] [ebp-58h]
    float fZoom; // [esp+E4h] [ebp-54h] BYREF
    playerState_s* ps; // [esp+E8h] [ebp-50h]
    float axis[3][3]; // [esp+ECh] [ebp-4Ch] BYREF
    WeaponDef* weapDef; // [esp+110h] [ebp-28h]
    float vAxis[3][3]; // [esp+114h] [ebp-24h] BYREF

    drawgun = 1;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = &cgArray[0];
    ps = &cgArray[0].predictedPlayerState;
    cgArray[0].refdef.dof.viewModelStart = 0.0;
    cgameGlob->refdef.dof.viewModelEnd = 0.0;
    if (ps->pm_type != 4 && ps->pm_type != 5 && !cgameGlob->renderingThirdPerson)
    {
        if (cgameGlob->cubemapShot || !cg_drawGun->current.enabled || CG_GetWeapReticleZoom(cgameGlob, &fZoom))
            drawgun = 0;
        if ((ps->eFlags & 0x300) == 0)
        {
            weaponIndex = BG_GetViewmodelWeaponIndex(ps);
            if (weaponIndex <= 0)
            {
                cgameGlob->gunPitch = cgameGlob->refdefViewAngles[0];
                cgameGlob->gunYaw = cgameGlob->refdefViewAngles[1];
                cgameGlob->gunXOfs = 0.0;
                cgameGlob->gunYOfs = 0.0;
                cgameGlob->gunZOfs = 0.0;
            }
            else
            {
                CalculateWeaponPosition_Sway(cgameGlob);
                CalculateWeaponPostion_PositionToADS(cgameGlob, ps);
                CalculateWeaponPosition(cgameGlob, placement.base.origin);
                Vec3Mad(placement.base.origin, cg_gun_x->current.value, cgameGlob->viewModelAxis[0], placement.base.origin);
                Vec3Mad(placement.base.origin, cg_gun_y->current.value, cgameGlob->viewModelAxis[1], placement.base.origin);
                Vec3Mad(placement.base.origin, cg_gun_z->current.value, cgameGlob->viewModelAxis[2], placement.base.origin);
                pe = &cgameGlob->playerEntity;
                ws.ps = ps;
                ws.xyspeed = cgameGlob->xyspeed;
                ws.frametime = (double)cgameGlob->frametime * 0.001000000047497451;
                ws.vLastMoveAng[0] = cgameGlob->playerEntity.vLastMoveAng[0];
                ws.vLastMoveAng[1] = cgameGlob->playerEntity.vLastMoveAng[1];
                ws.vLastMoveAng[2] = cgameGlob->playerEntity.vLastMoveAng[2];
                ws.fLastIdleFactor = cgameGlob->playerEntity.fLastIdleFactor;
                ws.time = cgameGlob->time - ps->deltaTime;
                if (cgameGlob->damageTime)
                    v3 = cgameGlob->damageTime - ps->deltaTime;
                else
                    v3 = 0;
                ws.damageTime = v3;
                ws.v_dmg_pitch = cgameGlob->v_dmg_pitch;
                ws.v_dmg_roll = cgameGlob->v_dmg_roll;
                ws.vGunOffset[0] = cgameGlob->vGunOffset[0];
                ws.vGunOffset[1] = cgameGlob->vGunOffset[1];
                ws.vGunOffset[2] = cgameGlob->vGunOffset[2];
                ws.vGunSpeed[0] = cgameGlob->vGunSpeed[0];
                ws.vGunSpeed[1] = cgameGlob->vGunSpeed[1];
                ws.vGunSpeed[2] = cgameGlob->vGunSpeed[2];
                ws.swayAngles[0] = cgameGlob->swayAngles[0];
                ws.swayAngles[1] = cgameGlob->swayAngles[1];
                ws.swayAngles[2] = cgameGlob->swayAngles[2];
                ws.weapIdleTime = &cgameGlob->weapIdleTime;
                BG_CalculateWeaponAngles(&ws, angles);
                AnglesToAxis(angles, vAxis);
                AnglesToAxis(ps->viewangles, vAxis2);
                MatrixMultiply(vAxis, vAxis2, axis);
                AxisToQuat(axis, placement.base.quat);
                weapDef = BG_GetWeaponDef(weaponIndex);
                if (!BG_IsAimDownSightWeapon(ps->weapon) || ps->fWeaponPosFrac == 0.0 || weapDef->overlayReticle)
                {
                    cgameGlob->gunPitch = cgameGlob->refdefViewAngles[0];
                    cgameGlob->gunYaw = cgameGlob->refdefViewAngles[1];
                }
                else
                {
                    UnitQuatToAngles(placement.base.quat, angles);
                    v1 = AngleNormalize360(angles[0]);
                    cgameGlob->gunPitch = v1;
                    v2 = AngleNormalize360(angles[1]);
                    cgameGlob->gunYaw = v2;
                }
                vLastMoveAng = cgameGlob->playerEntity.vLastMoveAng;
                cgameGlob->playerEntity.vLastMoveAng[0] = ws.vLastMoveAng[0];
                vLastMoveAng[1] = ws.vLastMoveAng[1];
                vLastMoveAng[2] = ws.vLastMoveAng[2];
                pe->fLastIdleFactor = ws.fLastIdleFactor;
                vGunOffset = cgameGlob->vGunOffset;
                cgameGlob->vGunOffset[0] = ws.vGunOffset[0];
                vGunOffset[1] = ws.vGunOffset[1];
                vGunOffset[2] = ws.vGunOffset[2];
                vGunSpeed = cgameGlob->vGunSpeed;
                cgameGlob->vGunSpeed[0] = ws.vGunSpeed[0];
                vGunSpeed[1] = ws.vGunSpeed[1];
                vGunSpeed[2] = ws.vGunSpeed[2];
                placement.scale = 1.0;
                CG_AddPlayerWeapon(localClientNum, &placement, ps, &cgameGlob->predictedPlayerEntity, drawgun);
            }
        }
    }
}

void __cdecl CalculateWeaponPosition_Sway(cg_s *cgameGlob)
{
    float ssScale; // [esp+8h] [ebp-14h]
    float ssScalea; // [esp+8h] [ebp-14h]
    int ssDT; // [esp+Ch] [ebp-10h]
    int weapIndex; // [esp+10h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-8h]
    float ssSwayScale; // [esp+18h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    ssDT = cgameGlob->shellshock.duration + cgameGlob->shellshock.startTime - cgameGlob->time;
    if (ssDT <= 0)
    {
        ssSwayScale = 1.0;
    }
    else
    {
        ssScale = 1.0;
        if (!cgameGlob->shellshock.parms)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 1344, 0, "%s", "cgameGlob->shellshock.parms");
        if (ssDT < cgameGlob->shellshock.parms->view.fadeTime)
            ssScale = (double)ssDT / (double)cgameGlob->shellshock.parms->view.fadeTime;
        ssScalea = (3.0 - ssScale * 2.0) * ssScale * ssScale;
        ssSwayScale = (weapDef->swayShellShockScale - 1.0) * ssScalea + 1.0;
    }
    BG_CalculateWeaponPosition_Sway(
        &cgameGlob->predictedPlayerState,
        cgameGlob->swayViewAngles,
        cgameGlob->swayOffset,
        cgameGlob->swayAngles,
        ssSwayScale,
        cgameGlob->frametime);
}

void __cdecl CalculateWeaponPosition(cg_s *cgameGlob, float *origin)
{
    int delta; // [esp+10h] [ebp-24h]
    float fLean; // [esp+14h] [ebp-20h]
    float right[3]; // [esp+18h] [ebp-1Ch] BYREF
    float fDist; // [esp+24h] [ebp-10h]
    float tempAngles[3]; // [esp+28h] [ebp-Ch] BYREF

    *origin = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    if (cgameGlob->predictedPlayerState.leanf != 0.0 && cgameGlob->predictedPlayerState.fWeaponPosFrac < 1.0)
    {
        tempAngles[0] = 0.0;
        tempAngles[1] = 0.0;
        tempAngles[2] = 0.0;
        fLean = GetLeanFraction(cgameGlob->predictedPlayerState.leanf);
        tempAngles[2] = (float)0.0 - (fLean + fLean);
        fDist = (1.0 - cgameGlob->predictedPlayerState.fWeaponPosFrac) * fLean * 1.600000023841858;
        AngleVectors(tempAngles, 0, right, 0);
        Vec3Mad(origin, fDist, right, origin);
    }
    CalculateWeaponPosition_BasePosition(cgameGlob, origin);
    CalculateWeaponPosition_SwayMovement(cgameGlob, origin);
    CalculateWeaponPosition_ToWorldPosition(cgameGlob, origin);
    delta = cgameGlob->time - cgameGlob->landTime;
    if (delta >= 150)
    {
        if (delta < 450)
            origin[2] = cgameGlob->landChange * 0.25 * (double)(450 - delta) / 300.0 + origin[2];
    }
    else
    {
        origin[2] = cgameGlob->landChange * 0.25 * (double)delta / 150.0 + origin[2];
    }
    CalculateWeaponPosition_SaveOffsetMovement(cgameGlob, origin);
}

void __cdecl CalculateWeaponPosition_SwayMovement(const cg_s *cgameGlob, float *origin)
{
    origin[1] = origin[1] - cgameGlob->swayOffset[1];
    origin[2] = origin[2] + cgameGlob->swayOffset[2];
}

void __cdecl CalculateWeaponPosition_BasePosition(cg_s *cgameGlob, float *origin)
{
    float vGunOfs[3]; // [esp+8h] [ebp-Ch] BYREF

    vGunOfs[0] = 0.0;
    vGunOfs[1] = 0.0;
    vGunOfs[2] = 0.0;
    CalculateWeaponPosition_BasePosition_movement(cgameGlob, vGunOfs);
    cgameGlob->playerEntity.vPositionLastOrg[0] = vGunOfs[0];
    cgameGlob->playerEntity.vPositionLastOrg[1] = vGunOfs[1];
    cgameGlob->playerEntity.vPositionLastOrg[2] = vGunOfs[2];
    Vec3Add(origin, vGunOfs, origin);
}

void __cdecl CalculateWeaponPosition_BasePosition_movement(cg_s *cgameGlob, float *origin)
{
    double v2; // st7
    double v3; // st6
    float scale; // [esp+Ch] [ebp-4Ch]
    float v5; // [esp+10h] [ebp-48h]
    bool v6; // [esp+14h] [ebp-44h]
    float lerp; // [esp+1Ch] [ebp-3Ch]
    float lerpa; // [esp+1Ch] [ebp-3Ch]
    playerEntity_t *pe; // [esp+20h] [ebp-38h]
    float delta; // [esp+24h] [ebp-34h]
    float deltaa; // [esp+24h] [ebp-34h]
    float frac; // [esp+28h] [ebp-30h]
    bool crouched; // [esp+33h] [ebp-25h]
    bool prone; // [esp+35h] [ebp-23h]
    bool moving; // [esp+37h] [ebp-21h]
    float targetPos[3]; // [esp+38h] [ebp-20h] BYREF
    int weapIndex; // [esp+44h] [ebp-14h]
    float minSpeed; // [esp+48h] [ebp-10h]
    int i; // [esp+4Ch] [ebp-Ch]
    playerState_s *ps; // [esp+50h] [ebp-8h]
    WeaponDef *weapDef; // [esp+54h] [ebp-4h]

    ps = &cgameGlob->predictedPlayerState;
    pe = &cgameGlob->playerEntity;
    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ((cgameGlob->predictedPlayerEntity.nextState.lerp.eFlags & 8) != 0)
    {
        minSpeed = weapDef->fProneMoveMinSpeed + cg_gun_move_minspeed->current.value;
    }
    else
    {
        if ((cgameGlob->predictedPlayerEntity.nextState.lerp.eFlags & 4) != 0)
            v2 = weapDef->fDuckedMoveMinSpeed + cg_gun_move_minspeed->current.value;
        else
            v2 = weapDef->fStandMoveMinSpeed + cg_gun_move_minspeed->current.value;
        minSpeed = v2;
    }
    moving = minSpeed < (double)cgameGlob->xyspeed;
    v6 = ps->weaponstate == 25 || ps->weaponstate == 26;
    prone = ps->viewHeightTarget == 11;
    crouched = ps->viewHeightTarget == 40;
    if (minSpeed >= (double)cgameGlob->xyspeed || ps->weaponstate == 7 || v6)
    {
        targetPos[0] = 0.0;
        targetPos[1] = 0.0;
        targetPos[2] = 0.0;
    }
    else
    {
        frac = (cgameGlob->xyspeed - minSpeed) / ((double)ps->speed - minSpeed);
        if (frac <= 0.0)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 1513, 0, "%s\n\t(frac) = %g", "(frac > 0)", frac);
        v5 = frac - 1.0;
        if (v5 < 0.0)
            scale = frac;
        else
            scale = 1.0;
        if ((cgameGlob->predictedPlayerEntity.nextState.lerp.eFlags & 8) != 0)
        {
            Vec3Scale(weapDef->vProneMove, scale, targetPos);
        }
        else if ((cgameGlob->predictedPlayerEntity.nextState.lerp.eFlags & 4) != 0)
        {
            Vec3Scale(weapDef->vDuckedMove, scale, targetPos);
        }
        else
        {
            Vec3Scale(weapDef->vStandMove, scale, targetPos);
        }
        targetPos[0] = cg_gun_move_f->current.value * scale + targetPos[0];
        targetPos[1] = cg_gun_move_r->current.value * scale + targetPos[1];
        targetPos[2] = cg_gun_move_u->current.value * scale + targetPos[2];
    }
    if ((!moving || !v6 || !prone) && (crouched || prone))
    {
        lerp = 1.0;
        if (v6)
        {
            if (ps->weaponstate == 25)
            {
                lerpa = (double)ps->weaponTime / (double)weapDef->nightVisionWearTime;
            }
            else
            {
                if (ps->weaponstate != 26)
                    MyAssertHandler(".\\cgame\\cg_weapons.cpp", 1549, 0, "%s", "ps->weaponstate == WEAPON_NIGHTVISION_REMOVE");
                lerpa = (double)ps->weaponTime / (double)weapDef->nightVisionWearTime;
            }
            if (MYLERP_END <= (double)lerpa)
            {
                if (MYLERP_START <= (double)lerpa)
                    lerp = 0.0;
                else
                    lerp = 1.0 - (lerpa - MYLERP_END) / (MYLERP_START - MYLERP_END);
            }
            else
            {
                lerp = 1.0;
            }
        }
        if (ps->viewHeightTarget == 40)
        {
            Vec3Mad(targetPos, lerp, weapDef->vDuckedOfs, targetPos);
            targetPos[0] = targetPos[0] + cg_gun_ofs_f->current.value;
            targetPos[1] = targetPos[1] + cg_gun_ofs_r->current.value;
            targetPos[2] = targetPos[2] + cg_gun_ofs_u->current.value;
        }
        else if (ps->viewHeightTarget == 11)
        {
            Vec3Mad(targetPos, lerp, weapDef->vProneOfs, targetPos);
            targetPos[0] = targetPos[0] + cg_gun_ofs_f->current.value;
            targetPos[1] = targetPos[1] + cg_gun_ofs_r->current.value;
            targetPos[2] = targetPos[2] + cg_gun_ofs_u->current.value;
        }
    }
    for (i = 0; i < 3; ++i)
    {
        if (targetPos[i] != pe->vLastMoveOrg[i])
        {
            if (ps->viewHeightCurrent == 11.0)
                v3 = weapDef->fPosProneMoveRate + cg_gun_move_rate->current.value;
            else
                v3 = weapDef->fPosMoveRate + cg_gun_move_rate->current.value;
            delta = (double)cgameGlob->frametime * 0.001000000047497451 * (targetPos[i] - pe->vLastMoveOrg[i]) * v3;
            if (targetPos[i] <= (double)pe->vLastMoveOrg[i])
            {
                if (delta > (double)cgameGlob->frametime * 0.001000000047497451 * -0.1000000014901161)
                    delta = (double)cgameGlob->frametime * 0.001000000047497451 * -0.1000000014901161;
                pe->vLastMoveOrg[i] = pe->vLastMoveOrg[i] + delta;
                if (targetPos[i] > (double)pe->vLastMoveOrg[i])
                    pe->vLastMoveOrg[i] = targetPos[i];
            }
            else
            {
                if (delta < (double)cgameGlob->frametime * 0.001000000047497451 * 0.1000000014901161)
                    delta = (double)cgameGlob->frametime * 0.001000000047497451 * 0.1000000014901161;
                pe->vLastMoveOrg[i] = pe->vLastMoveOrg[i] + delta;
                if (targetPos[i] < (double)pe->vLastMoveOrg[i])
                    pe->vLastMoveOrg[i] = targetPos[i];
            }
        }
    }
    if (ps->fWeaponPosFrac == 0.0)
    {
        Vec3Add(origin, cgameGlob->playerEntity.vLastMoveOrg, origin);
    }
    else if (ps->fWeaponPosFrac < 0.5)
    {
        deltaa = 1.0 - (ps->fWeaponPosFrac + ps->fWeaponPosFrac);
        Vec3Mad(origin, deltaa, cgameGlob->playerEntity.vLastMoveOrg, origin);
    }
}

void __cdecl CalculateWeaponPosition_ToWorldPosition(const cg_s *cgameGlob, float *origin)
{
    float vOffset[3]; // [esp+0h] [ebp-Ch] BYREF

    vOffset[0] = *origin;
    vOffset[1] = origin[1];
    vOffset[2] = origin[2];
    MatrixTransformVector43(vOffset, cgameGlob->viewModelAxis, origin);
}

void __cdecl CalculateWeaponPosition_SaveOffsetMovement(cg_s *cgameGlob, float *origin)
{
    float fPosLerp; // [esp+0h] [ebp-8h]
    int weapIndex; // [esp+4h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    if (!BG_IsAimDownSightWeapon(weapIndex)
        || (fPosLerp = cgameGlob->predictedPlayerState.fWeaponPosFrac, fPosLerp == 0.0))
    {
        cgameGlob->gunXOfs = 0.0;
        cgameGlob->gunYOfs = 0.0;
        cgameGlob->gunZOfs = 0.0;
    }
    else
    {
        cgameGlob->gunXOfs = (*origin - cgameGlob->refdef.vieworg[0]) * fPosLerp;
        cgameGlob->gunYOfs = (origin[1] - cgameGlob->refdef.vieworg[1]) * fPosLerp;
        cgameGlob->gunZOfs = (origin[2] - cgameGlob->refdef.vieworg[2]) * fPosLerp;
    }
}

void __cdecl CalculateWeaponPostion_PositionToADS(cg_s *cgameGlob, playerState_s *ps)
{
    int weapIndex; // [esp+4h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    if (BG_IsAimDownSightWeapon(weapIndex))
    {
        if (ps->fWeaponPosFrac == 1.0)
        {
            cgameGlob->playerEntity.bPositionToADS = 0;
        }
        else if (ps->fWeaponPosFrac == 0.0)
        {
            cgameGlob->playerEntity.bPositionToADS = 1;
        }
        cgameGlob->playerEntity.fLastWeaponPosFrac = ps->fWeaponPosFrac;
    }
}

void __cdecl CG_NextWeapon_f()
{
    if (cgArray[0].nextSnap)
    {
        if (WeaponCycleAllowed(cgArray))
        {
            cgArray[0].weaponSelectTime = cgArray[0].time;
            CG_MenuShowNotify(0, 1);
            CycleWeapPrimary(0, 1, 0);
        }
    }
}

bool __cdecl WeaponCycleAllowed(cg_s *cgameGlob)
{
    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3240, 0, "%s", "cgameGlob");
    if ((cgameGlob->predictedPlayerState.pm_flags & 0xC00) != 0)
        return 0;
    if ((cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0)
        return 0;
    if (cgameGlob->time - cgameGlob->weaponSelectTime >= cg_weaponCycleDelay->current.integer)
        return (cgameGlob->nextSnap->ps.otherFlags & 4) != 0;
    return 0;
}

void __cdecl CG_PrevWeapon_f()
{
    if (cgArray[0].nextSnap)
    {
        if (WeaponCycleAllowed(cgArray))
        {
            cgArray[0].weaponSelectTime = cgArray[0].time;
            CycleWeapPrimary(0, 0, 0);
        }
    }
}

void __cdecl CG_OutOfAmmoChange(int localClientNum)
{
    unsigned int bitNum; // [esp+0h] [ebp-14h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].nextSnap && cgArray[0].predictedPlayerState.pm_type < 7)
    {
        if (!cgArray[0].predictedPlayerState.weapon && cgArray[0].weaponLatestPrimaryIdx)
        {
            bitNum = cgArray[0].weaponLatestPrimaryIdx;
            if (cgArray == (cg_s *)-287036)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(cgArray[0].predictedPlayerState.weapons, bitNum, 16))
            {
                CG_SelectWeaponIndex(localClientNum, cgArray[0].weaponLatestPrimaryIdx);
                return;
            }
        }
        weapDef = BG_GetWeaponDef(cgArray[0].predictedPlayerState.weapon);
        if (!weapDef->cancelAutoHolsterWhenEmpty)
        {
            if (weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
            {
                if (!VerifyPlayerAltModeWeapon(localClientNum, weapDef))
                    return;
                if (BG_WeaponAmmo(&cgArray[0].predictedPlayerState, weapDef->altWeaponIndex))
                    goto LABEL_17;
            }
            if (!CycleWeapPrimary(localClientNum, 1, 1) && weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
                LABEL_17:
            CG_SelectWeaponIndex(localClientNum, weapDef->altWeaponIndex);
        }
    }
}

char __cdecl VerifyPlayerAltModeWeapon(int localClientNum, const WeaponDef *weapDef)
{
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3305, 0, "%s", "weapDef");
    if (weapDef->inventoryType != WEAPINVENTORY_ALTMODE)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3306, 0, "%s", "weapDef->inventoryType == WEAPINVENTORY_ALTMODE");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Com_BitCheckAssert(cgArray[0].predictedPlayerState.weapons, weapDef->altWeaponIndex, 16))
        return 1;
    Com_PrintError(
        14,
        "Player is holding alt-mode weapon \"%s\", but does not posses it's original, \"%s\".\n",
        weapDef->szInternalName,
        weapDef->szAltWeaponName);
    CG_SelectWeaponIndex(localClientNum, 0);
    return 0;
}

char __cdecl CycleWeapPrimary(int localClientNum, int cycleForward, int bIgnoreEmpty)
{
    unsigned int weaponSelect; // [esp+0h] [ebp-24h]
    unsigned int bitNum; // [esp+4h] [ebp-20h]
    unsigned int highestWeapIndex; // [esp+8h] [ebp-1Ch]
    int startIndex; // [esp+10h] [ebp-14h]
    int weaponIndex; // [esp+18h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+20h] [ebp-4h]
    WeaponDef *weapDefa; // [esp+20h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].nextSnap)
        return 0;
    if ((cgArray[0].nextSnap->ps.otherFlags & 4) == 0)
        return 0;
    if (cgArray[0].predictedPlayerState.pm_type >= 7)
        return 0;
    weaponIndex = cgArray[0].weaponSelect;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (BG_GetNumWeapons() < 2)
        return 0;
    if (weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
    {
        if (VerifyPlayerAltModeWeapon(localClientNum, weapDef))
        {
            CG_SelectWeaponIndex(localClientNum, weapDef->altWeaponIndex);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (weapDef->inventoryType == WEAPINVENTORY_PRIMARY || !cgArray[0].weaponLatestPrimaryIdx)
            goto LABEL_21;
        bitNum = cgArray[0].weaponLatestPrimaryIdx;
        if (cgArray == (cg_s *)-287036)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(cgArray[0].predictedPlayerState.weapons, bitNum, 16))
        {
            CG_SelectWeaponIndex(localClientNum, cgArray[0].weaponLatestPrimaryIdx);
            return 1;
        }
        else
        {
        LABEL_21:
            highestWeapIndex = BG_GetNumWeapons() - 1;
            if (!weaponIndex)
                weaponIndex = 1;
            startIndex = weaponIndex;
            while (1)
            {
                weaponIndex = (highestWeapIndex + weaponIndex + 2 * (cycleForward != 0) - 1 - 1) % highestWeapIndex + 1;
                if (weaponIndex == startIndex)
                    break;
                if (cgArray == (cg_s *)-287036)
                    MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
                if (Com_BitCheckAssert(cgArray[0].predictedPlayerState.weapons, weaponIndex, 16)
                    && (!bIgnoreEmpty || BG_WeaponAmmo(&cgArray[0].predictedPlayerState, weaponIndex)))
                {
                    weapDefa = BG_GetWeaponDef(weaponIndex);
                    if (weapDefa->inventoryType != WEAPINVENTORY_ALTMODE
                        && weapDefa->inventoryType != WEAPINVENTORY_ITEM
                        && weapDefa->inventoryType != WEAPINVENTORY_OFFHAND)
                    {
                        CG_SelectWeaponIndex(localClientNum, weaponIndex);
                        return 1;
                    }
                }
            }
            weaponSelect = cgArray[0].weaponSelect;
            if (cgArray == (cg_s *)-287036)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (!Com_BitCheckAssert(cgArray[0].predictedPlayerState.weapons, weaponSelect, 16))
                CG_SelectWeaponIndex(localClientNum, 0);
            return 0;
        }
    }
}

unsigned int __cdecl CG_AltWeaponToggleIndex(int localClientNum, const cg_s *cgameGlob)
{
    const playerState_s *ps; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    int newPrimaryIdx; // [esp+8h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3501, 0, "%s", "cgameGlob");
    ps = &cgameGlob->predictedPlayerState;
    weapDef = BG_GetWeaponDef(cgameGlob->weaponSelect);
    if (weapDef->altWeaponIndex)
        return weapDef->altWeaponIndex;
    newPrimaryIdx = NextWeapInCycle(localClientNum, ps, WEAPINVENTORY_PRIMARY, cgameGlob->weaponSelect, 1, 1, 1);
    if (newPrimaryIdx)
        return BG_GetWeaponDef(newPrimaryIdx)->altWeaponIndex;
    newPrimaryIdx = NextWeapInCycle(localClientNum, ps, WEAPINVENTORY_PRIMARY, cgameGlob->weaponSelect, 1, 0, 1);
    if (newPrimaryIdx)
        return BG_GetWeaponDef(newPrimaryIdx)->altWeaponIndex;
    else
        return 0;
}

int __cdecl NextWeapInCycle(
    int localClientNum,
    const playerState_s *ps,
    weapInventoryType_t type,
    unsigned int startWeaponIndex,
    bool cycleForward,
    bool skipEmpties,
    bool skipHaveNoAlts)
{
    unsigned int highestWeapIndex; // [esp+0h] [ebp-14h]
    int weaponIndex; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3373, 0, "%s", "ps");
    if (BG_GetNumWeapons() < 2)
        return 0;
    highestWeapIndex = BG_GetNumWeapons() - 1;
    if (!startWeaponIndex)
        startWeaponIndex = 1;
    weaponIndex = startWeaponIndex;
    while (1)
    {
        weaponIndex = (highestWeapIndex + weaponIndex + (cycleForward ? 1 : -1) - 1) % highestWeapIndex + 1;
        if (weaponIndex == startWeaponIndex)
            break;
        weapDef = BG_GetWeaponDef(weaponIndex);
        if (weapDef->inventoryType == type)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, weaponIndex, 16)
                && (!skipEmpties || BG_WeaponAmmo(ps, weaponIndex))
                && (!skipHaveNoAlts || weapDef->altWeaponIndex))
            {
                return weaponIndex;
            }
        }
    }
    return 0;
}

void __cdecl CG_ActionSlotDown_f()
{
    ActionSlotType v0; // [esp+0h] [ebp-24h]
    unsigned int bitNum; // [esp+4h] [ebp-20h]
    int slot; // [esp+10h] [ebp-14h] BYREF
    unsigned int weapon; // [esp+14h] [ebp-10h]
    int localClientNum; // [esp+18h] [ebp-Ch]
    bool didSomething; // [esp+1Fh] [ebp-5h]
    playerState_s *ps; // [esp+20h] [ebp-4h]

    localClientNum = 0;
    if (ActionSlotUsageAllowed(cgArray) && cgArray[0].time - cgArray[0].lastActionSlotTime >= 250 && ActionParms(&slot))
    {
        ps = &cgArray[0].predictedPlayerState;
        didSomething = 0;
        v0 = cgArray[0].predictedPlayerState.actionSlotType[slot];
        switch (v0)
        {
        case ACTIONSLOTTYPE_SPECIFYWEAPON:
            weapon = ps->actionSlotParam[slot].specifyWeapon.index;
            if (weapon == cgArray[0].weaponSelect)
            {
                if (weapon == cgArray[0].weaponLatestPrimaryIdx)
                    goto LABEL_15;
                bitNum = cgArray[0].weaponLatestPrimaryIdx;
                if (!ps)
                    MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
                if (!Com_BitCheckAssert(ps->weapons, bitNum, 16))
                {
                LABEL_15:
                    didSomething = CycleWeapPrimary(localClientNum, 1, 0);
                }
                else
                {
                    CG_SelectWeaponIndex(localClientNum, cgArray[0].weaponLatestPrimaryIdx);
                    didSomething = 1;
                }
            }
            else
            {
                if (!ps)
                    MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
                if (Com_BitCheckAssert(ps->weapons, weapon, 16) && ps->weapon != weapon)
                {
                    didSomething = 1;
                    CG_SelectWeaponIndex(localClientNum, weapon);
                }
            }
            break;
        case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
            didSomething = ToggleWeaponAltMode(localClientNum);
            break;
        case ACTIONSLOTTYPE_NIGHTVISION:
            cgArray[0].extraButtons |= 0x40000u;
            didSomething = 1;
            break;
        }
        cgArray[0].ammoFadeTime = cgArray[0].time;
        if (didSomething)
        {
            cgArray[0].lastActionSlotTime = cgArray[0].time;
            if ((ps->eFlags & 0x300) != 0 && ps->actionSlotType[slot] == ACTIONSLOTTYPE_SPECIFYWEAPON)
                cgArray[0].extraButtons |= 0x20u;
        }
    }
}

char __cdecl ToggleWeaponAltMode(int localClientNum)
{
    unsigned int weapIdx; // [esp+4h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.weaponstate == 1
        || cgArray[0].predictedPlayerState.weaponstate == 2
        || cgArray[0].predictedPlayerState.weaponstate == 3
        || cgArray[0].predictedPlayerState.weaponstate == 4)
        && cgArray[0].predictedPlayerState.weaponstate != 1)
    {
        return 0;
    }
    weapIdx = CG_AltWeaponToggleIndex(localClientNum, cgArray);
    if (!weapIdx)
        return 0;
    CG_SelectWeaponIndex(localClientNum, weapIdx);
    return 1;
}

bool __cdecl ActionSlotUsageAllowed(cg_s *cgameGlob)
{
    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 3545, 0, "%s", "cgameGlob");
    if (!cgameGlob->nextSnap)
        return 0;
    if (cgameGlob->predictedPlayerState.weaponstate == 3 || cgameGlob->predictedPlayerState.weaponstate == 4)
        return 0;
    if ((cgameGlob->predictedPlayerState.pm_flags & 0xC00) != 0)
        return 0;
    if ((cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0)
        return 0;
    return (cgameGlob->nextSnap->ps.otherFlags & 4) != 0;
}

char __cdecl ActionParms(int *slotResult)
{
    const char *v2; // eax
    int slot; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        v2 = Cmd_Argv(1);
        slot = atoi(v2);
        if (slot >= 1 && slot <= 4)
        {
            *slotResult = slot - 1;
            return 1;
        }
        else
        {
            Com_Printf(0, "+/-actionslot; number given is out of range.  Was %i, expected 1 thru %i.\n", slot, 4);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "USAGE: +/-actionslot <number>\n");
        return 0;
    }
}

void __cdecl CG_ActionSlotUp_f()
{
    int slot[2]; // [esp+Ch] [ebp-8h] BYREF

    slot[1] = 0;
    if (ActionSlotUsageAllowed(cgArray))
        ActionParms(slot);
}

void __cdecl CG_EjectWeaponBrass(int localClientNum, const entityState_s *ent, int event)
{
    unsigned int number; // [esp+0h] [ebp-28h]
    const FxEffectDef *viewShellEjectEffect; // [esp+4h] [ebp-24h]
    const FxEffectDef *viewLastShotEjectEffect; // [esp+8h] [ebp-20h]
    bool v6; // [esp+Ch] [ebp-1Ch]
    snapshot_s *nextSnap; // [esp+10h] [ebp-18h]
    const FxEffectDef *fxDef; // [esp+20h] [ebp-8h]
    const WeaponDef *weaponDef; // [esp+24h] [ebp-4h]

    if (cg_brass->current.enabled && ent->eType < 17 && ent->weapon)
    {
        if (ent->weapon < BG_GetNumWeapons())
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1071,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            nextSnap = cgArray[0].nextSnap;
            v6 = (nextSnap->ps.otherFlags & 6) != 0 && ent->number == nextSnap->ps.clientNum;
            weaponDef = BG_GetWeaponDef(ent->weapon);
            if (weaponDef->viewLastShotEjectEffect && weaponDef->worldLastShotEjectEffect && event == 27)
            {
                if (v6)
                    viewLastShotEjectEffect = weaponDef->viewLastShotEjectEffect;
                else
                    viewLastShotEjectEffect = weaponDef->worldLastShotEjectEffect;
                fxDef = viewLastShotEjectEffect;
            }
            else
            {
                if (v6)
                    viewShellEjectEffect = weaponDef->viewShellEjectEffect;
                else
                    viewShellEjectEffect = weaponDef->worldShellEjectEffect;
                fxDef = viewShellEjectEffect;
            }
            if (fxDef)
            {
                if (v6)
                    number = CG_WeaponDObjHandle(ent->weapon);
                else
                    number = ent->number;
                CG_PlayBoltedEffect(localClientNum, fxDef, number, scr_const.tag_brass);
            }
        }
        else
        {
            Com_Error(ERR_DROP, "CG_EjectWeaponBrass: ent->weapon >= BG_GetNumWeapons()");
        }
    }
}

void __cdecl CG_FireWeapon(
    int localClientNum,
    centity_s *cent,
    int event,
    unsigned __int16 tagName,
    unsigned int weapon,
    const playerState_s *ps)
{
    snapshot_s *nextSnap; // [esp+Ch] [ebp-3Ch]
    const weaponInfo_s *weapInfo; // [esp+14h] [ebp-34h]
    snd_alias_list_t *firesound; // [esp+18h] [ebp-30h]
    DObj_s *obj; // [esp+1Ch] [ebp-2Ch]
    int playbackId; // [esp+20h] [ebp-28h]
    float origin[3]; // [esp+24h] [ebp-24h] BYREF
    int msec; // [esp+30h] [ebp-18h] BYREF
    int isPlayer; // [esp+34h] [ebp-14h]
    cg_s *cgameGlob; // [esp+38h] [ebp-10h]
    const WeaponDef *weaponDef; // [esp+3Ch] [ebp-Ch]
    const entityState_s *ent; // [esp+40h] [ebp-8h]
    int playerUsingTurret; // [esp+44h] [ebp-4h]

    ent = &cent->nextState;
    if (!weapon)
        weapon = ent->weapon;
    if (weapon)
    {
        if (weapon < BG_GetNumWeapons())
        {
            iassert(localClientNum == 0);
            weapInfo = &cg_weaponsArray[0][weapon];
            weaponDef = BG_GetWeaponDef(weapon);
            iassert(weaponDef);
            cent->bMuzzleFlash = 1;
            iassert(localClientNum == 0);
            cgameGlob = cgArray;
            nextSnap = cgArray[0].nextSnap;
            isPlayer = (nextSnap->ps.otherFlags & 6) != 0 && ent->number == nextSnap->ps.clientNum;

            if (sv_clientSideBullets->current.enabled)
                DrawBulletImpacts(localClientNum, cent, weaponDef, tagName, ps);

            if (isPlayer)
            {
                CG_UpdateViewModelPose(weapInfo->viewModelDObj, localClientNum);
                BG_WeaponFireRecoil(&cgameGlob->predictedPlayerState, cgameGlob->vGunSpeed, cgameGlob->kickAVel);
            }

            playerUsingTurret = 0;
            if (ent->eType == 11 && (ps->eFlags & 0x300) != 0 && ps->viewlocked_entNum == ent->number)
            {
                playerUsingTurret = 1;
                isPlayer = 1;
            }

            if (ent->eType == 11)
                WeaponFlash(localClientNum, ent->number, weapon, playerUsingTurret, tagName);

            if (ent->eType == 12)
            {
                WeaponFlash(localClientNum, ent->number, weapon, 0, tagName);
                CG_EjectWeaponBrass(localClientNum, ent, event);
                Veh_IncTurretBarrelRoll(localClientNum, ent->number, heli_barrelRotation->current.value);
            }

            firesound = weaponDef->fireSound;
            if (isPlayer && weaponDef->fireSoundPlayer)
                firesound = weaponDef->fireSoundPlayer;

            if (event == 27)
            {
                if (isPlayer && weaponDef->fireLastSoundPlayer)
                {
                    firesound = weaponDef->fireLastSoundPlayer;
                }
                else if (weaponDef->fireLastSound)
                {
                    firesound = weaponDef->fireLastSound;
                }
            }
            if (firesound)
            {
                if (isPlayer)
                {
                    if (!weapInfo->viewModelDObj || !CG_DObjGetWorldTagPos(&cgameGlob->viewModelPose, weapInfo->viewModelDObj, tagName, origin))
                    {
                        BG_EvaluateTrajectory(&ent->lerp.pos, cgameGlob->time, origin);
                    }
                }
                else
                {
                    obj = Com_GetClientDObj(ent->number, localClientNum);
                    if (!obj || !CG_DObjGetWorldTagPos(&cent->pose, obj, tagName, origin))
                        BG_EvaluateTrajectory(&ent->lerp.pos, cgameGlob->time, origin);
                }
                playbackId = CG_PlaySoundAlias(localClientNum, ent->number, origin, firesound);
                if (cent->nextState.eType == 1 && !weaponDef->silenced && weaponDef->weapType != WEAPTYPE_GRENADE)
                {
                    SND_GetKnownLength(playbackId, &msec);
                    CG_CompassAddWeaponPingInfo(localClientNum, cent, origin, msec);
                }
            }
            if (!BG_GetWeaponDef(weapon)->bBoltAction)
                CG_EjectWeaponBrass(localClientNum, ent, event);
            if (isPlayer)
                TakeClipOnlyWeaponIfEmpty(localClientNum, &cgameGlob->predictedPlayerState);
        }
        else
        {
            Com_Error(ERR_DROP, "CG_FireWeapon: weapon >= BG_GetNumWeapons()");
        }
    }
}

void __cdecl DrawBulletImpacts(
    int localClientNum,
    const centity_s *ent,
    const WeaponDef *weaponDef,
    unsigned __int16 boneName,
    const playerState_s *ps)
{
    double v5; // st7
    float v6; // [esp+20h] [ebp-138h]
    float v7; // [esp+24h] [ebp-134h]
    float v8; // [esp+28h] [ebp-130h]
    float v10; // [esp+40h] [ebp-118h]
    snapshot_s *nextSnap; // [esp+48h] [ebp-110h]
    float velocity[3]; // [esp+4Ch] [ebp-10Ch] BYREF
    unsigned __int8 boneIndex; // [esp+5Bh] [ebp-FDh] BYREF
    int weaponNum; // [esp+5Ch] [ebp-FCh]
    int ads; // [esp+60h] [ebp-F8h]
    int shotCount; // [esp+64h] [ebp-F4h]
    float origin[3]; // [esp+68h] [ebp-F0h] BYREF
    float range; // [esp+74h] [ebp-E4h]
    float dist; // [esp+78h] [ebp-E0h]
    cg_s *cgameGlob; // [esp+7Ch] [ebp-DCh]
    int shot; // [esp+80h] [ebp-D8h]
    int dobjNumber; // [esp+84h] [ebp-D4h]
    orientation_t orient; // [esp+88h] [ebp-D0h] BYREF
    bool drawTracers; // [esp+BBh] [ebp-9Dh]
    float minSpread; // [esp+BCh] [ebp-9Ch] BYREF
    float maxSpread; // [esp+C0h] [ebp-98h] BYREF
    float viewang[3]; // [esp+C4h] [ebp-94h] BYREF
    float tracerStart[3]; // [esp+D0h] [ebp-88h] BYREF
    clientInfo_t *ci; // [esp+DCh] [ebp-7Ch]
    BulletFireParams dst; // [esp+E0h] [ebp-78h] BYREF
    float aimSpreadScale; // [esp+120h] [ebp-38h]
    float aimSpreadAmount; // [esp+124h] [ebp-34h]
    orientation_t gunOrient; // [esp+128h] [ebp-30h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    nextSnap = cgArray[0].nextSnap;
    if ((nextSnap->ps.otherFlags & 6) != 0 && ent->nextState.number == nextSnap->ps.clientNum)
    {
        weaponNum = BG_GetViewmodelWeaponIndex(ps);
        dobjNumber = CG_WeaponDObjHandle(weaponNum);
        aimSpreadScale = cgameGlob->lastFrame.aimSpreadScale / 255.0;
        ads = ps->fWeaponPosFrac == 1.0;
        BG_GetSpreadForWeapon(ps, weaponDef, &minSpread, &maxSpread);
        CG_GetPlayerViewOrigin(localClientNum, ps, origin);
        viewang[0] = cgameGlob->gunPitch;
        viewang[1] = cgameGlob->gunYaw;
        viewang[2] = 0.0;
        boneIndex = 0;
        if (!CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex))
            return;
        if (cgameGlob->renderingThirdPerson)
        {
            ci = &cgameGlob->bgs.clientinfo[ent->nextState.number];
            tracerStart[0] = origin[0];
            tracerStart[1] = origin[1];
            tracerStart[2] = origin[2];
            AngleVectors(ci->playerAngles, orient.axis[0], orient.axis[1], orient.axis[2]);
            Vec3Mad(tracerStart, 30.0, orient.axis[0], tracerStart);
        }
        else
        {
            if (!FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &gunOrient))
                return;
            tracerStart[0] = gunOrient.origin[0];
            tracerStart[1] = gunOrient.origin[1];
            tracerStart[2] = gunOrient.origin[2];
            AngleVectors(viewang, orient.axis[0], orient.axis[1], orient.axis[2]);
        }
        drawTracers = cg_firstPersonTracerChance->current.value * 32768.0 > (double)rand();
        goto LABEL_33;
    }
    if (ent->nextState.eType != 1)
    {
        if (ent->nextState.eType == 11)
        {
            minSpread = weaponDef->playerSpread;
            maxSpread = weaponDef->playerSpread;
            aimSpreadScale = 1.0;
            ads = 0;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if (!CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex)
                || !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient))
            {
                return;
            }
        }
        else
        {
            if (ent->nextState.eType != 14 && ent->nextState.eType != 12)
            {
                Com_PrintError(14, "Unknown eType %i in CG_DrawBulletImpacts()\n", ent->nextState.eType);
                return;
            }
            minSpread = weaponDef->fAdsSpread;
            maxSpread = weaponDef->fAdsSpread;
            ads = 1;
            aimSpreadScale = 0.0;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if (!CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex)
                || !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient))
            {
                return;
            }
        }
        origin[0] = orient.origin[0];
        origin[1] = orient.origin[1];
        origin[2] = orient.origin[2];
        tracerStart[0] = orient.origin[0];
        tracerStart[1] = orient.origin[1];
        tracerStart[2] = orient.origin[2];
        drawTracers = 0;
        goto LABEL_33;
    }
    CG_GuessSpreadForWeapon(localClientNum, ent, weaponDef, &minSpread, &maxSpread);
    ci = &cgameGlob->bgs.clientinfo[ent->nextState.number];
    ads = CG_IsPlayerADS(ci, ent);
    Vec3Sub(ent->nextState.lerp.pos.trBase, ent->currentState.pos.trBase, velocity);
    if (cgameGlob->nextSnap->serverTime != cgameGlob->snap->serverTime)
    {
        v8 = 1000.0 / (double)(cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime);
        Vec3Scale(velocity, v8, velocity);
    }
    v10 = Vec3Length(velocity);
    v7 = v10 - 255.0;
    if (v7 < 0.0)
        v6 = v10;
    else
        v6 = 255.0;
    dist = v6;
    aimSpreadScale = v6 / 255.0;
    dobjNumber = ent->nextState.number;
    boneIndex = 0;
    if (CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex)
        && FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient))
    {
        origin[0] = orient.origin[0];
        origin[1] = orient.origin[1];
        origin[2] = orient.origin[2];
        tracerStart[0] = orient.origin[0];
        tracerStart[1] = orient.origin[1];
        tracerStart[2] = orient.origin[2];
        drawTracers = 0;
        AngleVectors(ci->playerAngles, orient.axis[0], orient.axis[1], orient.axis[2]);
    LABEL_33:
        if (ads)
            v5 = (maxSpread - weaponDef->fAdsSpread) * aimSpreadScale + weaponDef->fAdsSpread;
        else
            v5 = (maxSpread - minSpread) * aimSpreadScale + minSpread;
        aimSpreadAmount = v5;
        memset((unsigned __int8 *)&dst, 0, sizeof(dst));
        dst.weaponEntIndex = 1022;
        dst.ignoreEntIndex = ent->nextState.number;
        dst.damageMultiplier = 1.0;
        dst.methodOfDeath = (weaponDef->bRifleBullet != 0) + 1;
        if (weaponDef->weapClass == WEAPCLASS_SPREAD)
        {
            shotCount = weaponDef->shotCount;
            range = weaponDef->fMinDamageRange;
        }
        else
        {
            shotCount = 1;
            range = 8192.0;
        }
        dst.origStart[0] = origin[0];
        dst.origStart[1] = origin[1];
        dst.origStart[2] = origin[2];
        for (shot = 0; shot < shotCount; ++shot)
        {
            dst.dir[0] = orient.axis[0][0];
            dst.dir[1] = orient.axis[0][1];
            dst.dir[2] = orient.axis[0][2];
            dst.start[0] = origin[0];
            dst.start[1] = origin[1];
            dst.start[2] = origin[2];
            CG_BulletEndpos(
                shot + ps->commandTime,
                aimSpreadAmount,
                dst.start,
                dst.end,
                dst.dir,
                orient.axis[0],
                orient.axis[1],
                orient.axis[2],
                range);
            dst.damageMultiplier = 1.0;
            FireBulletPenetrate(localClientNum, &dst, weaponDef, ent, tracerStart, drawTracers);
        }
    }
}

void __cdecl FireBulletPenetrate(
    int localClientNum,
    BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    float *tracerStart,
    bool drawTracer)
{
    float v6; // [esp+Ch] [ebp-1F0h]
    float v7; // [esp+10h] [ebp-1ECh]
    float v8; // [esp+14h] [ebp-1E8h]
    float v9; // [esp+18h] [ebp-1E4h]
    float v10; // [esp+1Ch] [ebp-1E0h]
    float v11; // [esp+20h] [ebp-1DCh]
    double value; // [esp+24h] [ebp-1D8h]
    float v13; // [esp+2Ch] [ebp-1D0h]
    bool v14; // [esp+30h] [ebp-1CCh]
    __int16 v15; // [esp+34h] [ebp-1C8h]
    int v16; // [esp+38h] [ebp-1C4h]
    unsigned int v17; // [esp+3Ch] [ebp-1C0h]
    unsigned int v18; // [esp+40h] [ebp-1BCh]
    int v19; // [esp+44h] [ebp-1B8h]
    float v20; // [esp+50h] [ebp-1ACh]
    float v21[3]; // [esp+54h] [ebp-1A8h] BYREF
    int v22; // [esp+60h] [ebp-19Ch]
    int v23; // [esp+64h] [ebp-198h]
    int v24; // [esp+68h] [ebp-194h]
    int v25; // [esp+6Ch] [ebp-190h]
    int v26; // [esp+70h] [ebp-18Ch]
    int contents; // [esp+78h] [ebp-184h]
    int v28; // [esp+7Ch] [ebp-180h]
    int v29; // [esp+80h] [ebp-17Ch]
    int targetEntityNum; // [esp+84h] [ebp-178h]
    int number; // [esp+88h] [ebp-174h]
    float v32; // [esp+90h] [ebp-16Ch]
    float v33; // [esp+94h] [ebp-168h]
    float v34[3]; // [esp+98h] [ebp-164h] BYREF
    float SurfacePenetrationDepth; // [esp+A4h] [ebp-158h]
    float v36; // [esp+A8h] [ebp-154h]
    int v37; // [esp+ACh] [ebp-150h]
    float v[4]; // [esp+B4h] [ebp-148h] BYREF
    float diff[3]; // [esp+C4h] [ebp-138h] BYREF
    int perks; // [esp+D8h] [ebp-124h]
    int hitContents; // [esp+DCh] [ebp-120h]
    int damage; // [esp+E0h] [ebp-11Ch]
    int surfType; // [esp+E4h] [ebp-118h]
    int entityNum; // [esp+E8h] [ebp-114h]
    int sourceEntityNum; // [esp+ECh] [ebp-110h]
    BulletTraceResults revBr; // [esp+F4h] [ebp-108h] BYREF
    float lastHitPos[3]; // [esp+140h] [ebp-BCh] BYREF
    float depth; // [esp+14Ch] [ebp-B0h]
    int weapType; // [esp+150h] [ebp-ACh]
    int penetrateIndex; // [esp+154h] [ebp-A8h]
    unsigned __int16 traceHitEntityId; // [esp+158h] [ebp-A4h]
    cg_s *cgameGlob; // [esp+15Ch] [ebp-A0h]
    bool allSolid; // [esp+163h] [ebp-99h]
    BulletFireParams revBp; // [esp+164h] [ebp-98h] BYREF
    int weaponIndex; // [esp+1A4h] [ebp-58h]
    bool revTraceHit; // [esp+1ABh] [ebp-51h]
    BulletTraceResults br; // [esp+1ACh] [ebp-50h] BYREF
    float maxDepth; // [esp+1F4h] [ebp-8h]
    bool traceHit; // [esp+1FBh] [ebp-1h]

    iassert(bp);
    iassert(weapDef);
    iassert(attacker);

    weaponIndex = BG_GetWeaponIndex(weapDef);
    weapType = weapDef->weapType;
    if (weapType)
        drawTracer = 0;

    traceHit = BulletTrace(localClientNum, bp, weapDef, attacker, &br, 0);
    if (traceHit)
    {
        traceHitEntityId = Trace_GetEntityHitId(&br.trace);

        if (drawTracer)
            CG_SpawnTracer(localClientNum, tracerStart, br.hitPos);

        if (!weapType)
        {
            DynEntCl_EntityImpactEvent(&br.trace, localClientNum, attacker->nextState.number, bp->start, br.hitPos, 0);
            DynEntCl_DynEntImpactEvent(localClientNum, attacker->nextState.number, bp->start, br.hitPos, weapDef->damage, 0);
            hitContents = br.trace.contents;
            damage = weapDef->damage;
            surfType = (br.trace.surfaceFlags & 0x1F00000) >> 20;
            entityNum = traceHitEntityId;
            sourceEntityNum = attacker->nextState.number;

            if (!sv_clientSideBullets->current.enabled || !IsEntityAPlayer(localClientNum, entityNum))
                CG_BulletHitEvent(
                    localClientNum,
                    sourceEntityNum,
                    entityNum,
                    weaponIndex,
                    bp->start,
                    br.hitPos,
                    br.trace.normal,
                    surfType,
                    41,
                    0,
                    damage,
                    hitContents);
        }
        if (weapDef->penetrateType && !br.trace.startsolid)
        {
            iassert(localClientNum == 0);
            cgameGlob = cgArray;
            for (penetrateIndex = 0; penetrateIndex < 5; ++penetrateIndex)
            {
                maxDepth = BG_GetSurfacePenetrationDepth(weapDef, br.depthSurfaceType);
                if (attacker->nextState.eType == 1)
                {
                    perks = cgameGlob->bgs.clientinfo[attacker->nextState.clientNum].perks;
                    if ((perks & 0x20) != 0)
                        maxDepth = maxDepth * perk_bulletPenetrationMultiplier->current.value;
                }
                if (maxDepth <= 0.0)
                    break;
                lastHitPos[0] = br.hitPos[0];
                lastHitPos[1] = br.hitPos[1];
                lastHitPos[2] = br.hitPos[2];
                if (!BG_AdvanceTrace(bp, &br, 0.13500001))
                    break;
                traceHit = BulletTrace(localClientNum, bp, weapDef, attacker, &br, br.depthSurfaceType);
                revBp = *bp; // Com_Memcpy((char *)&revBp, (char *)bp, 64);
                //LODWORD(diff[4]) = bp->dir;
                revBp.dir[0] = -bp->dir[0];
                revBp.dir[1] = -bp->dir[1];
                revBp.dir[2] = -bp->dir[2];
                // LODWORD(diff[3]) = bp->end;
                revBp.start[0] = bp->end[0];
                revBp.start[1] = bp->end[1];
                revBp.start[2] = bp->end[2];
                Vec3Mad(lastHitPos, 0.0099999998, revBp.dir, revBp.end);
                revBr = br; // Com_Memcpy((char *)&revBr, (char *)&br, 68);
                revBr.trace.normal[0] = -revBr.trace.normal[0];
                revBr.trace.normal[1] = -revBr.trace.normal[1];
                revBr.trace.normal[2] = -revBr.trace.normal[2];

                if (traceHit)
                    BG_AdvanceTrace(&revBp, &revBr, 0.0099999998);

                revTraceHit = BulletTrace(localClientNum, &revBp, weapDef, attacker, &revBr, revBr.depthSurfaceType);
                allSolid = revTraceHit && revBr.trace.allsolid || br.trace.startsolid && revBr.trace.startsolid;

                if (revTraceHit || allSolid)
                {
                    traceHitEntityId = Trace_GetEntityHitId(&revBr.trace);
                    if (allSolid)
                    {
                        Vec3Sub(revBp.end, revBp.start, diff);
                        v13 = Vec3Length(diff);
                    }
                    else
                    {
                        Vec3Sub(lastHitPos, revBr.hitPos, v);
                        v13 = Vec3Length(v);
                    }
                    depth = v13;
                    if (v13 < 1.0)
                        depth = 1.0;

                    if (revTraceHit)
                    {
                        if (attacker->nextState.eType == 1
                            && (v37 = cgameGlob->bgs.clientinfo[attacker->nextState.clientNum].perks, (v37 & 0x20) != 0))
                        {
                            value = perk_bulletPenetrationMultiplier->current.value;
                            v36 = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType) * value;
                            v11 = v36 - maxDepth;
                            v10 = v11 < 0.0 ? v36 : maxDepth;
                            maxDepth = v10;
                        }
                        else
                        {
                            SurfacePenetrationDepth = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType);
                            v9 = SurfacePenetrationDepth - maxDepth;
                            v8 = v9 < 0.0 ? SurfacePenetrationDepth : maxDepth;
                            maxDepth = v8;
                        }
                        if (maxDepth <= 0.0)
                            return;
                    }
                    bp->damageMultiplier = bp->damageMultiplier - depth / maxDepth;
                    if (bp->damageMultiplier <= 0.0)
                        return;
                    if (!allSolid && !weapType)
                    {
                        Vec3Sub(revBr.hitPos, br.hitPos, v34);
                        v33 = Vec3LengthSq(v34);
                        v32 = bullet_penetrationMinFxDist->current.value;
                        v7 = v32 * v32;
                        if (v7 < (double)v33)
                        {
                            if (!traceHit || (br.trace.surfaceFlags & 4) == 0)
                            {
                                contents = revBr.trace.contents;
                                v28 = weapDef->damage;
                                v29 = (revBr.trace.surfaceFlags & 0x1F00000) >> 20;
                                targetEntityNum = traceHitEntityId;
                                number = attacker->nextState.number;
                                if (!sv_clientSideBullets->current.enabled || !IsEntityAPlayer(localClientNum, targetEntityNum))
                                    CG_BulletHitEvent(
                                        localClientNum,
                                        number,
                                        targetEntityNum,
                                        weaponIndex,
                                        revBp.start,
                                        revBr.hitPos,
                                        bp->dir,
                                        v29,
                                        41,
                                        4u,
                                        v28,
                                        contents);
                            }
                            if (traceHit)
                            {
                                v22 = br.trace.contents;
                                v23 = weapDef->damage;
                                v24 = (br.trace.surfaceFlags & 0x1F00000) >> 20;
                                v25 = traceHitEntityId;
                                v26 = attacker->nextState.number;
                                if (!sv_clientSideBullets->current.enabled || !IsEntityAPlayer(localClientNum, v25))
                                    CG_BulletHitEvent(
                                        localClientNum,
                                        v26,
                                        v25,
                                        weaponIndex,
                                        bp->start,
                                        br.hitPos,
                                        br.trace.normal,
                                        v24,
                                        41,
                                        0,
                                        v23,
                                        v22);
                                DynEntCl_DynEntImpactEvent(
                                    localClientNum,
                                    attacker->nextState.number,
                                    bp->start,
                                    br.hitPos,
                                    weapDef->damage,
                                    0);
                            }
                        }
                    }
                }
                else if (traceHit && !br.trace.allsolid)
                {
                    Vec3Sub(lastHitPos, br.hitPos, v21);
                    v20 = Vec3LengthSq(v21);
                    v6 = bullet_penetrationMinFxDist->current.value * bullet_penetrationMinFxDist->current.value;
                    if (v6 < (double)v20)
                    {
                        traceHitEntityId = Trace_GetEntityHitId(&br.trace);
                        if (!weapType)
                        {
                            v15 = br.trace.contents;
                            v16 = weapDef->damage;
                            v17 = (br.trace.surfaceFlags & 0x1F00000) >> 20;
                            v18 = traceHitEntityId;
                            v19 = attacker->nextState.number;
                            if (!sv_clientSideBullets->current.enabled || !IsEntityAPlayer(localClientNum, traceHitEntityId))
                                CG_BulletHitEvent(
                                    localClientNum,
                                    v19,
                                    v18,
                                    weaponIndex,
                                    bp->start,
                                    br.hitPos,
                                    br.trace.normal,
                                    v17,
                                    41,
                                    0,
                                    v16,
                                    v15);
                            DynEntCl_DynEntImpactEvent(
                                localClientNum,
                                attacker->nextState.number,
                                bp->start,
                                br.hitPos,
                                weapDef->damage,
                                0);
                        }
                    }
                }
                if (!traceHit)
                    return;
            }
        }
    }
    else if (drawTracer)
    {
        CG_SpawnTracer(localClientNum, tracerStart, bp->end);
    }
}

char __cdecl BulletTrace(
    int localClientNum,
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    BulletTraceResults *br,
    unsigned int lastSurfaceType)
{
    centity_s *Entity; // [esp+Ch] [ebp-10h]
    unsigned __int16 hitEntId; // [esp+18h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2301, 0, "%s", "bp");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2302, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2303, 0, "%s", "attacker");
    if (!br)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2304, 0, "%s", "br");
    if (lastSurfaceType >= 0x1D)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            2305,
            0,
            "lastSurfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            lastSurfaceType,
            29);
    Com_Memset((unsigned int *)br, 0, 68);
    CG_LocationalTrace(&br->trace, (float*)bp->start, (float*)bp->end, bp->ignoreEntIndex, 0x2806831);
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        return 0;
    hitEntId = Trace_GetEntityHitId(&br->trace);
    if (hitEntId == 1022)
        Entity = 0;
    else
        Entity = CG_GetEntity(localClientNum, hitEntId);
    Vec3Lerp(bp->start, bp->end, br->trace.fraction, br->hitPos);
    if (Entity)
    {
        if ((Entity->nextState.eType == 1 || Entity->nextState.eType == 2) && !br->trace.surfaceFlags)
            br->trace.surfaceFlags = 0x700000;
        br->ignoreHitEnt = ShouldIgnoreHitEntity(attacker->nextState.number, hitEntId);
    }
    br->depthSurfaceType = (br->trace.surfaceFlags & 0x1F00000) >> 20;
    if ((br->trace.surfaceFlags & 0x100) != 0)
    {
        br->depthSurfaceType = 0;
    }
    else if (!br->depthSurfaceType)
    {
        if (lastSurfaceType)
            br->depthSurfaceType = lastSurfaceType;
    }
    return 1;
}

bool __cdecl ShouldIgnoreHitEntity(int attackerNum, int hitEntNum)
{
    return hitEntNum == attackerNum;
}

bool __cdecl IsEntityAPlayer(int localClientNum, unsigned int entityNum)
{
    centity_s *cent; // [esp+4h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entityNum);
    if (!cent)
    {
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2350, 0, "%s", "cent");
        return 0;
    }
    return cent->nextState.eType == 1;
}

void __cdecl CG_BulletEndpos(
    int randSeed,
    float spread,
    const float *start,
    float *end,
    float *dir,
    const float *forwardDir,
    const float *rightDir,
    const float *upDir,
    float maxRange)
{
    float v9; // [esp+Ch] [ebp-54h]
    float v10; // [esp+10h] [ebp-50h]
    float right; // [esp+54h] [ebp-Ch] BYREF
    float aimOffset; // [esp+58h] [ebp-8h]
    float up; // [esp+5Ch] [ebp-4h] BYREF

    if ((LODWORD(spread) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2635, 0, "%s", "!IS_NAN(spread)");
    if (!end)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2637, 0, "%s", "end");
    v10 = spread * 0.01745329238474369;
    v9 = tan(v10);
    aimOffset = v9 * maxRange;
    if ((LODWORD(aimOffset) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2641, 0, "%s", "!IS_NAN(aimOffset)");
    RandomBulletDir(randSeed, &right, &up);
    right = right * aimOffset;
    up = up * aimOffset;
    if ((LODWORD(right) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2648, 0, "%s", "!IS_NAN(right)");
    if ((LODWORD(up) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2649, 0, "%s", "!IS_NAN(up)");
    Vec3Mad(start, maxRange, forwardDir, end);
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            2653,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    Vec3Mad(end, right, rightDir, end);
    Vec3Mad(end, up, upDir, end);
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            2658,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    if (dir)
    {
        Vec3Sub(end, start, dir);
        Vec3Normalize(dir);
        if ((COERCE_UNSIGNED_INT(*dir) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(dir[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(dir[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\cgame\\cg_weapons.cpp",
                2665,
                0,
                "%s",
                "!IS_NAN((dir)[0]) && !IS_NAN((dir)[1]) && !IS_NAN((dir)[2])");
        }
    }
}

void __cdecl RandomBulletDir(int randSeed, float *x, float *y)
{
    float v3; // [esp+8h] [ebp-14h]
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    if (!x)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2612, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 2613, 0, "%s", "y");
    theta = G_GoodRandomFloat(&randSeed) * 360.0;
    r = G_GoodRandomFloat(&randSeed);
    v3 = theta * 0.01745329238474369;
    cosT = cos(v3);
    sinT = sin(v3);
    *x = r * cosT;
    *y = r * sinT;
}

void __cdecl TakeClipOnlyWeaponIfEmpty(int localClientNum, playerState_s *ps)
{
    if (BG_WeaponIsClipOnly(ps->weapon)
        && !ps->ammoclip[BG_ClipForWeapon(ps->weapon)]
        && !ps->ammo[BG_AmmoForWeapon(ps->weapon)]
            && !BG_GetWeaponDef(ps->weapon)->hasDetonator)
    {
        BG_TakePlayerWeapon(ps, ps->weapon, 0);
        CG_OutOfAmmoChange(localClientNum);
    }
}

void __cdecl CG_SpawnTracer(int localClientNum, const float *pstart, const float *pend)
{
    int v3; // [esp+8h] [ebp-60h]
    float *trBase; // [esp+24h] [ebp-44h]
    float dir[3]; // [esp+34h] [ebp-34h] BYREF
    float dist; // [esp+40h] [ebp-28h]
    const cg_s *cgameGlob; // [esp+44h] [ebp-24h]
    float start[3]; // [esp+48h] [ebp-20h] BYREF
    float end[3]; // [esp+54h] [ebp-14h] BYREF
    int startTime; // [esp+60h] [ebp-8h]
    localEntity_s *le; // [esp+64h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    start[0] = *pstart;
    start[1] = pstart[1];
    start[2] = pstart[2];
    end[0] = *pend;
    end[1] = pend[1];
    end[2] = pend[2];
    Vec3Sub(end, start, dir);
    dist = Vec3Normalize(dir);
    le = CG_AllocLocalEntity(localClientNum);
    le->leType = LE_MOVING_TRACER;
    le->tracerClipDist = dist;
    if (cgameGlob->frametime)
        v3 = rand() % cgameGlob->frametime / 2;
    else
        v3 = 0;
    startTime = cgameGlob->time - v3;
    le->endTime = startTime + (int)(dist * 1000.0 / cg_tracerSpeed->current.value);
    le->pos.trType = TR_LINEAR;
    le->pos.trTime = startTime;
    trBase = le->pos.trBase;
    le->pos.trBase[0] = start[0];
    trBase[1] = start[1];
    trBase[2] = start[2];
    Vec3Scale(dir, cg_tracerSpeed->current.value, le->pos.trDelta);
    if ((COERCE_UNSIGNED_INT(le->pos.trBase[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(le->pos.trBase[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(le->pos.trBase[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            4064,
            0,
            "%s",
            "!IS_NAN((le->pos.trBase)[0]) && !IS_NAN((le->pos.trBase)[1]) && !IS_NAN((le->pos.trBase)[2])");
    }
    if ((COERCE_UNSIGNED_INT(le->pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(le->pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(le->pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            4065,
            0,
            "%s",
            "!IS_NAN((le->pos.trDelta)[0]) && !IS_NAN((le->pos.trDelta)[1]) && !IS_NAN((le->pos.trDelta)[2])");
    }
}

void __cdecl CG_DrawTracer(const float *start, const float *finish, const refdef_s *refdef)
{
    float v3; // [esp+0h] [ebp-54h]
    float diff[3]; // [esp+8h] [ebp-4Ch] BYREF
    float finishWidth; // [esp+14h] [ebp-40h] BYREF
    FxBeam beam; // [esp+18h] [ebp-3Ch] BYREF
    float startWidth; // [esp+50h] [ebp-4h] BYREF

    startWidth = cg_tracerWidth->current.value;
    finishWidth = cg_tracerWidth->current.value;
    ScaleTracer(start, finish, refdef->vieworg, &startWidth, &finishWidth);
    beam.begin[0] = *start;
    beam.begin[1] = start[1];
    beam.begin[2] = start[2];
    beam.end[0] = *finish;
    beam.end[1] = finish[1];
    beam.end[2] = finish[2];
    beam.beginColor.packed = -1;
    beam.endColor.packed = -1;
    beam.beginRadius = startWidth;
    beam.endRadius = finishWidth;
    beam.material = cgMedia.tracerMaterial;
    Vec3Sub(finish, start, diff);
    v3 = Vec3Length(diff);
    beam.segmentCount = (int)(v3 * 8.0 / cg_tracerScrewDist->current.value);
    beam.wiggleDist = cg_tracerScrewRadius->current.value;
    FX_Beam_Add(&beam);
}

void __cdecl ScaleTracer(
    const float *start,
    const float *finish,
    const float *viewOrg,
    float *startWidth,
    float *finishWidth)
{
    float v[4]; // [esp+28h] [ebp-30h] BYREF
    float diff[3]; // [esp+38h] [ebp-20h] BYREF
    float startDist; // [esp+44h] [ebp-14h]
    float finishDist; // [esp+48h] [ebp-10h]
    float tracerScale; // [esp+4Ch] [ebp-Ch]
    float tracerScaleDistRange; // [esp+50h] [ebp-8h]
    float tracerScaleMinDist; // [esp+54h] [ebp-4h]

    if (!startWidth)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4095, 0, "%s", "startWidth");
    if (!finishWidth)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4096, 0, "%s", "finishWidth");
    if (!cg_tracerScale)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4097, 0, "%s", "cg_tracerScale");
    if (!cg_tracerScaleMinDist)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4098, 0, "%s", "cg_tracerScaleMinDist");
    if (!cg_tracerScaleDistRange)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4099, 0, "%s", "cg_tracerScaleDistRange");
    tracerScale = cg_tracerScale->current.value;
    tracerScaleMinDist = cg_tracerScaleMinDist->current.value;
    tracerScaleDistRange = cg_tracerScaleDistRange->current.value;
    if (tracerScale != 1.0)
    {
        Vec3Sub(viewOrg, start, diff);
        startDist = Vec3Length(diff);
        Vec3Sub(viewOrg, finish, v);
        finishDist = Vec3Length(v);
        if (tracerScaleMinDist != 0.0)
        {
            startDist = startDist - tracerScaleMinDist;
            finishDist = finishDist - tracerScaleMinDist;
        }
        if (startDist > 0.0)
            *startWidth = CalcTracerFinalScale(tracerScaleDistRange, startDist, tracerScale) * *startWidth;
        if (finishDist > 0.0)
            *finishWidth = CalcTracerFinalScale(tracerScaleDistRange, finishDist, tracerScale) * *finishWidth;
    }
}

double __cdecl CalcTracerFinalScale(float tracerScaleDistRange, float dist, float tracerScale)
{
    float v4; // [esp+0h] [ebp-Ch]
    float lerp; // [esp+8h] [ebp-4h]

    if (tracerScaleDistRange <= 0.0)
    {
        return tracerScale;
    }
    else
    {
        lerp = dist / tracerScaleDistRange;
        v4 = tracerScale * lerp;
        if (tracerScale <= 1.0)
            MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 533, 0, "%s", "min < max");
        if (v4 >= 1.0)
        {
            if (tracerScale >= (double)v4)
                return (float)(tracerScale * lerp);
            else
                return tracerScale;
        }
        else
        {
            return (float)1.0;
        }
    }
}

cg_s *__cdecl CG_GetLocalClientGlobalsForEnt(int localClientNum, int entityNum)
{
    snapshot_s *nextSnap; // [esp+4h] [ebp-Ch]
    int clientIndex; // [esp+Ch] [ebp-4h]

    for (clientIndex = 0; clientIndex < 1; ++clientIndex)
    {
        if (CL_IsLocalClientActive(clientIndex))
        {
            if (clientIndex)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1071,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    clientIndex);
            nextSnap = cgArray[0].nextSnap;
            if ((nextSnap->ps.otherFlags & 6) != 0 && entityNum == nextSnap->ps.clientNum)
                return cgArray;
        }
    }
    return 0;
}

void __cdecl CG_GetViewDirection(int localClientNum, int entityNum, float *forward, float *right, float *up)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-10h]
    unsigned int clientNum; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobalsForEnt(localClientNum, entityNum);
    if (cgameGlob)
    {
        BG_GetPlayerViewDirection(&cgameGlob->predictedPlayerState, forward, right, up);
    }
    else
    {
        clientNum = CG_GetEntity(localClientNum, entityNum)->nextState.clientNum;
        if (clientNum >= 0x40)
            MyAssertHandler(
                ".\\cgame\\cg_weapons.cpp",
                4194,
                0,
                "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                clientNum,
                64);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (!cgArray[0].bgs.clientinfo[clientNum].infoValid)
            MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4197, 0, "%s", "ci->infoValid");
        AngleVectors(cgArray[0].bgs.clientinfo[clientNum].playerAngles, forward, right, up);
    }
}

void __cdecl CG_CalcEyePoint(int localClientNum, int entityNum, float *eyePos)
{
    const cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    centity_s *cent; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobalsForEnt(localClientNum, entityNum);
    if (cgameGlob)
    {
        *eyePos = cgameGlob->refdef.vieworg[0];
        eyePos[1] = cgameGlob->refdef.vieworg[1];
        eyePos[2] = cgameGlob->refdef.vieworg[2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, entityNum);
        *eyePos = cent->nextState.lerp.pos.trBase[0];
        eyePos[1] = cent->nextState.lerp.pos.trBase[1];
        eyePos[2] = cent->nextState.lerp.pos.trBase[2];
        if (entityNum < 64)
        {
            if ((cent->nextState.lerp.eFlags & 8) != 0)
            {
                eyePos[2] = eyePos[2] + 11.0;
            }
            else if ((cent->nextState.lerp.eFlags & 4) != 0)
            {
                eyePos[2] = eyePos[2] + 40.0;
            }
            else
            {
                eyePos[2] = eyePos[2] + 60.0;
            }
        }
    }
}

void __cdecl CG_RandomEffectAxis(const float *forward, float *left, float *up)
{
    float scale; // [esp+0h] [ebp-38h]
    float degrees; // [esp+Ch] [ebp-2Ch]
    float v5; // [esp+1Ch] [ebp-1Ch]
    float v6; // [esp+20h] [ebp-18h]
    float v7; // [esp+24h] [ebp-14h]
    float dot; // [esp+28h] [ebp-10h]
    float point[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if (!forward)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4339, 0, "%s", "forward");
    if (!left)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4340, 0, "%s", "left");
    if (!up)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4341, 0, "%s", "up");
    v5 = -forward[2];
    v6 = *forward;
    v7 = -forward[1];
    point[0] = v5;
    point[1] = v6;
    point[2] = v7;
    dot = Vec3Dot(point, forward);
    scale = -dot;
    Vec3Mad(point, scale, forward, point);
    degrees = random() * 360.0;
    RotatePointAroundVector(left, forward, point, degrees);
    Vec3Normalize(left);
    Vec3Cross(forward, left, up);
}

void __cdecl CG_ImpactEffectForWeapon(
    unsigned int weaponIndex,
    unsigned int surfType,
    char impactFlags,
    const FxEffectDef **outFx,
    snd_alias_list_t **outSnd)
{
    snd_alias_list_t *v5; // [esp+0h] [ebp-28h]
    snd_alias_list_t *v6; // [esp+4h] [ebp-24h]
    snd_alias_list_t *v7; // [esp+8h] [ebp-20h]
    snd_alias_list_t *v8; // [esp+Ch] [ebp-1Ch]
    snd_alias_list_t *v9; // [esp+10h] [ebp-18h]
    int fleshType; // [esp+18h] [ebp-10h]
    int fxType; // [esp+20h] [ebp-8h]
    const WeaponDef *weaponDef; // [esp+24h] [ebp-4h]

    weaponDef = BG_GetWeaponDef(weaponIndex);
    if (!weaponDef)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4363, 0, "%s", "weaponDef");
    if (surfType >= 0x1D)
        MyAssertHandler(
            ".\\cgame\\cg_weapons.cpp",
            4364,
            0,
            "surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            surfType,
            29);
    fxType = -1;
    *outSnd = 0;
    switch (weaponDef->impactType)
    {
    case IMPACT_TYPE_BULLET_SMALL:
        fxType = (impactFlags & 4) != 0;
        if ((impactFlags & 4) != 0)
            v9 = cgMedia.bulletExitSmallSound[surfType];
        else
            v9 = cgMedia.bulletHitSmallSound[surfType];
        *outSnd = v9;
        break;
    case IMPACT_TYPE_BULLET_LARGE:
        fxType = ((impactFlags & 4) != 0) + 2;
        if ((impactFlags & 4) != 0)
            v8 = cgMedia.bulletExitLargeSound[surfType];
        else
            v8 = cgMedia.bulletHitLargeSound[surfType];
        *outSnd = v8;
        break;
    case IMPACT_TYPE_BULLET_AP:
        fxType = ((impactFlags & 4) != 0) + 6;
        if ((impactFlags & 4) != 0)
            v7 = cgMedia.bulletExitAPSound[surfType];
        else
            v7 = cgMedia.bulletHitAPSound[surfType];
        *outSnd = v7;
        break;
    case IMPACT_TYPE_SHOTGUN:
        fxType = ((impactFlags & 4) != 0) + 4;
        if ((impactFlags & 4) != 0)
            v6 = cgMedia.shotgunExitSound[surfType];
        else
            v6 = cgMedia.shotgunHitSound[surfType];
        *outSnd = v6;
        break;
    case IMPACT_TYPE_GRENADE_BOUNCE:
        fxType = 8;
        if (weaponDef->bounceSound)
            v5 = weaponDef->bounceSound[surfType];
        else
            v5 = 0;
        *outSnd = v5;
        break;
    case IMPACT_TYPE_GRENADE_EXPLODE:
        fxType = 9;
        *outSnd = cgMedia.grenadeExplodeSound[surfType];
        break;
    case IMPACT_TYPE_ROCKET_EXPLODE:
        fxType = 10;
        *outSnd = cgMedia.rocketExplodeSound[surfType];
        break;
    case IMPACT_TYPE_PROJECTILE_DUD:
        fxType = 11;
        *outSnd = weaponDef->projDudSound;
        break;
    default:
        break;
    }
    if (fxType >= 0)
    {
        if (surfType == 7)
        {
            if ((impactFlags & 2) != 0)
                fleshType = (impactFlags & 1) != 0 ? 3 : 1;
            else
                fleshType = (impactFlags & 1) != 0 ? 2 : 0;
            *outFx = cgMedia.fx->table[fxType].flesh[fleshType];
        }
        else
        {
            *outFx = cgMedia.fx->table[fxType].nonflesh[surfType];
        }
    }
    else
    {
        *outFx = 0;
    }
}

void __cdecl CG_BulletHitEvent(
    int localClientNum,
    int sourceEntityNum,
    unsigned int targetEntityNum,
    unsigned int weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    unsigned int surfType,
    int event,
    unsigned __int8 eventParam,
    int damage,
    __int16 hitContents)
{
    char hasMuzzlePoint; // [esp+3h] [ebp-29h]
    float muzzle[3]; // [esp+10h] [ebp-1Ch] BYREF
    float exitDir[3]; // [esp+20h] [ebp-Ch] BYREF

    if (sv_clientSideBullets->current.enabled && IsEntityAPlayer(localClientNum, targetEntityNum))
    {
        iassert(!(eventParam & IMPACTEFFECT_EXIT));

        if (CalcMuzzlePoint(localClientNum, sourceEntityNum, muzzle, scr_const.tag_flash))
        {
            Vec3Sub(position, muzzle, exitDir);
            Vec3Normalize(exitDir);
            hasMuzzlePoint = 1;
        }
        else
        {
            hasMuzzlePoint = 0;
        }
        if (hasMuzzlePoint)
            CG_BulletHitEvent_Internal(
                localClientNum,
                sourceEntityNum,
                targetEntityNum,
                weaponIndex,
                startPos,
                position,
                exitDir,
                surfType,
                event,
                eventParam | 4,
                damage,
                hitContents);
    }
    CG_BulletHitEvent_Internal(
        localClientNum,
        sourceEntityNum,
        targetEntityNum,
        weaponIndex,
        startPos,
        position,
        normal,
        surfType,
        event,
        eventParam,
        damage,
        hitContents);
}

int __cdecl CalcMuzzlePoint(int localClientNum, int entityNum, float *muzzle, unsigned int flashTag)
{
    char *v5; // eax
    double v6; // st7
    DObj_s *obj; // [esp+8h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    centity_s *cent; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobalsForEnt(localClientNum, entityNum);
    if (cgameGlob)
    {
        *muzzle = cgameGlob->refdef.vieworg[0];
        muzzle[1] = cgameGlob->refdef.vieworg[1];
        muzzle[2] = cgameGlob->refdef.vieworg[2];
        return 1;
    }
    else
    {
        cent = CG_GetEntity(localClientNum, entityNum);
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj)
        {
            if (CG_DObjGetWorldTagPos(&cent->pose, obj, flashTag, muzzle))
            {
                return 1;
            }
            else
            {
                *muzzle = cent->nextState.lerp.pos.trBase[0];
                muzzle[1] = cent->nextState.lerp.pos.trBase[1];
                muzzle[2] = cent->nextState.lerp.pos.trBase[2];
                if (entityNum < 64)
                {
                    v5 = SL_ConvertToString(flashTag);
                    Com_DPrintf(17, "No %s in CalcMuzzlePoint on entity %d.\n", v5, entityNum);
                    if ((cent->nextState.lerp.eFlags & 8) != 0)
                    {
                        muzzle[2] = muzzle[2] + 11.0;
                    }
                    else
                    {
                        if ((cent->nextState.lerp.eFlags & 4) != 0)
                            v6 = muzzle[2] + 40.0;
                        else
                            v6 = muzzle[2] + 60.0;
                        muzzle[2] = v6;
                    }
                }
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl CG_BulletHitEvent_Internal(
    int localClientNum,
    int sourceEntityNum,
    unsigned int targetEntityNum,
    unsigned int weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    unsigned int surfType,
    int event,
    unsigned __int8 eventParam,
    int damage,
    __int16 hitContents)
{
    snapshot_s *nextSnap; // [esp+4h] [ebp-3Ch]
    snd_alias_list_t *hitSound; // [esp+Ch] [ebp-34h] BYREF
    cg_s *cgameGlob; // [esp+10h] [ebp-30h]
    int time; // [esp+14h] [ebp-2Ch]
    const FxEffectDef *fx; // [esp+18h] [ebp-28h] BYREF
    float axis[3][3]; // [esp+1Ch] [ebp-24h] BYREF

    iassert(sourceEntityNum >= 0);
    iassert(sourceEntityNum != ENTITYNUM_NONE);
    iassert(position);
    iassert(normal);
    iassert(surfType >= 0 && surfType < SURF_TYPECOUNT);
    iassert(damage >= 0);
    iassert(localClientNum == 0);

    cgameGlob = cgArray;
    CG_GetEntity(localClientNum, sourceEntityNum);
    fx = 0;
    hitSound = 0;

    if ((hitContents & 0x800) == 0)
        CG_ImpactEffectForWeapon(weaponIndex, surfType, eventParam, &fx, &hitSound);

    if (fx && !cg_blood->current.enabled && surfType == 7)
        fx = cgMedia.fxNoBloodFleshHit;

    if (hitSound)
        CG_PlaySoundAlias(localClientNum, 1022, position, hitSound);

    iassert(localClientNum == 0);

    time = cgArray[0].time;
    if (fx && (normal[0] != 0.0 || normal[1] != 0.0 || normal[2] != 0.0))
    {
        axis[0][0] = normal[0];
        axis[0][1] = normal[1];
        axis[0][2] = normal[2];
        CG_RandomEffectAxis(axis[0], axis[1], axis[2]);
        if (cg_marks_ents_player_only->current.enabled)
        {
            nextSnap = cgameGlob->nextSnap;
            if ((nextSnap->ps.otherFlags & 6) == 0 || sourceEntityNum != nextSnap->ps.clientNum)
                targetEntityNum = 1023;
        }
        FX_PlayOrientedEffectWithMarkEntity(localClientNum, fx, cgameGlob->time, position, axis, targetEntityNum);
    }
    BulletTrajectoryEffects(
        localClientNum,
        sourceEntityNum,
        startPos,
        position,
        surfType,
        scr_const.tag_flash,
        eventParam,
        damage);
}

void __cdecl BulletTrajectoryEffects(
    int localClientNum,
    int sourceEntityNum,
    float *startPos,
    float *position,
    int surfType,
    unsigned int flashTag,
    unsigned __int8 impactFlags,
    int damage)
{
    float muzzle[3]; // [esp+0h] [ebp-Ch] BYREF

    if (sourceEntityNum < 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4312, 0, "%s", "sourceEntityNum >= 0");
    if (damage < 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4313, 0, "%s\n\t(damage) = %i", "(damage >= 0)", damage);
    if (CalcMuzzlePoint(localClientNum, sourceEntityNum, muzzle, flashTag))
    {
        if (ShouldSpawnTracer(localClientNum, sourceEntityNum))
            CG_SpawnTracer(localClientNum, muzzle, position);
        WhizbySound(localClientNum, muzzle, position);
        if ((impactFlags & 4) == 0 && !sv_clientSideBullets->current.enabled)
            DynEntCl_DynEntImpactEvent(localClientNum, sourceEntityNum, startPos, position, damage, 0);
    }
}

void __cdecl WhizbySound(int localClientNum, const float *start, const float *end)
{
    float viewDelta[3]; // [esp+10h] [ebp-4Ch] BYREF
    float delta[3]; // [esp+1Ch] [ebp-40h] BYREF
    float dir[3]; // [esp+28h] [ebp-34h] BYREF
    float projPos[3]; // [esp+34h] [ebp-28h] BYREF
    float viewRadius; // [esp+40h] [ebp-1Ch]
    float dist; // [esp+44h] [ebp-18h]
    float viewDist; // [esp+48h] [ebp-14h]
    float minDist; // [esp+4Ch] [ebp-10h]
    float soundRadius; // [esp+50h] [ebp-Ch]
    const float *viewOrg; // [esp+54h] [ebp-8h]
    float maxDist; // [esp+58h] [ebp-4h]

    minDist = 64.0;
    maxDist = -64.0;
    soundRadius = 140.0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    viewOrg = cgArray[0].refdef.vieworg;
    Vec3Sub(end, start, delta);
    Vec3NormalizeTo(delta, dir);
    dist = Vec3Dot(delta, dir);
    Vec3Sub(viewOrg, start, viewDelta);
    viewDist = Vec3Dot(viewDelta, dir);
    if (minDist <= (double)viewDist && dist >= viewDist - maxDist)
    {
        Vec3Mad(start, viewDist, dir, projPos);
        Vec3Sub(projPos, viewOrg, delta);
        viewRadius = Vec3Length(delta);
        if (soundRadius >= (double)viewRadius)
        {
            Vec3Mad(projPos, -16.0, dir, projPos);
            CG_PlaySoundAlias(localClientNum, 1022, projPos, cgMedia.bulletWhizby);
        }
    }
}

bool __cdecl ShouldSpawnTracer(int localClientNum, int sourceEntityNum)
{
    snapshot_s *nextSnap; // [esp+8h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cg_tracerChance->current.value <= 0.0)
        return 0;
    nextSnap = cgArray[0].nextSnap;
    if ((nextSnap->ps.otherFlags & 6) != 0 && sourceEntityNum == nextSnap->ps.clientNum)
        return 0;
    if (CG_PlayerUsingScopedTurret(localClientNum)
        && cgArray[0].predictedPlayerState.viewlocked_entNum == sourceEntityNum)
    {
        return 0;
    }
    return cg_tracerChance->current.value * 32768.0 > (double)rand();
}

void __cdecl CG_BulletHitClientEvent(
    int localClientNum,
    int sourceEntityNum,
    float *startPos,
    float *position,
    unsigned int surfType,
    int event,
    int damage)
{
    const char *v7; // eax

    if (sourceEntityNum < 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4556, 0, "%s", "sourceEntityNum >= 0");
    if (sourceEntityNum == 1023)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4557, 0, "%s", "sourceEntityNum != ENTITYNUM_NONE");
    if (!position)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4558, 0, "%s", "position");
    if (surfType > 0x1C)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4559, 0, "%s", "surfType >= 0 && surfType < SURF_TYPECOUNT");
    if (damage < 0)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4560, 0, "%s\n\t(damage) = %i", "(damage >= 0)", damage);
    if (event == 42)
    {
        CG_PlaySoundAlias(localClientNum, 1022, position, cgMedia.bulletHitSmallSound[surfType]);
    LABEL_19:
        BulletTrajectoryEffects(
            localClientNum,
            sourceEntityNum,
            startPos,
            position,
            surfType,
            scr_const.tag_flash,
            0,
            damage);
        return;
    }
    if (event == 43)
    {
        CG_PlaySoundAlias(localClientNum, 1022, position, cgMedia.bulletHitLargeSound[surfType]);
        goto LABEL_19;
    }
    if (!alwaysfails)
    {
        v7 = va("CG_BulletHitClientEvent: Unknown event [%d]\n", event);
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4573, 0, v7);
    }
}

void __cdecl CG_MeleeBloodEvent(int localClientNum, const centity_s *cent)
{
    int weapon; // [esp+4h] [ebp-18h]
    snapshot_s *nextSnap; // [esp+8h] [ebp-14h]
    unsigned int dobjHandle; // [esp+10h] [ebp-Ch]

    if (!cent)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4589, 0, "%s", "cent");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    nextSnap = cgArray[0].nextSnap;
    if ((nextSnap->ps.otherFlags & 6) == 0 || cent->nextState.number != nextSnap->ps.clientNum)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4594, 0, "%s", "isPlayer");
    dobjHandle = CG_WeaponDObjHandle(cent->nextState.weapon);
    weapon = cent->nextState.weapon;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1095,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cg_weaponsArray[0][weapon].knifeModel)
    {
        if (cg_blood->current.enabled)
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxKnifeBlood, dobjHandle, scr_const.tag_knife_fx);
        else
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxKnifeNoBlood, dobjHandle, scr_const.tag_knife_fx);
    }
}

void __cdecl CG_SetupWeaponDef(int localClientNum)
{
    char v1; // [esp+3h] [ebp-2225h]
    _BYTE *v2; // [esp+8h] [ebp-2220h]
    char *v3; // [esp+Ch] [ebp-221Ch]
    _DWORD dst[129]; // [esp+10h] [ebp-2218h] BYREF
    char *ConfigString; // [esp+214h] [ebp-2014h]
    int iNumFiles; // [esp+218h] [ebp-2010h]
    _BYTE *v7; // [esp+21Ch] [ebp-200Ch]
    _BYTE v8[8196]; // [esp+220h] [ebp-2008h] BYREF

    memset((unsigned __int8 *)dst, 0, 0x1FCu);
    iNumFiles = 0;
    ConfigString = CL_GetConfigString(localClientNum, 0x8D2u);
    v3 = ConfigString;
    v2 = v8;
    do
    {
        v1 = *v3;
        *v2++ = *v3++;
    } while (v1);
    v7 = v8;
    dst[iNumFiles++] = (_DWORD)v8;
    while (*v7)
    {
        if (*v7 == 32)
        {
            *v7++ = 0;
            if (*v7 && *v7 != 32)
            {
                if (iNumFiles >= 127)
                    break;
                dst[iNumFiles++] = (_DWORD)v7;
            }
        }
        else
        {
            ++v7;
        }
    }
    ParseWeaponDefFiles((const char **)dst, iNumFiles);
}

void __cdecl ParseWeaponDefFiles(const char **ppszFiles, int iNumFiles)
{
    const char *name; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < iNumFiles; ++i)
    {
        name = ppszFiles[i];
        if (BG_GetWeaponIndexForName(name, 0) != i + 1)
            Com_Error(ERR_DROP, "Weapon index mismatch for '%s'", name);
    }
}

unsigned int __cdecl ValidLatestPrimaryWeapIdx(unsigned int weaponIndex)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]
    unsigned int weaponIndexa; // [esp+Ch] [ebp+8h]

    if (!weaponIndex)
        return 0;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->inventoryType == WEAPINVENTORY_PRIMARY)
        return weaponIndex;
    weaponIndexa = weapDef->altWeaponIndex;
    if (BG_GetWeaponDef(weaponIndexa)->inventoryType)
        return 0;
    else
        return weaponIndexa;
}

void __cdecl CG_SelectWeaponIndex(int localClientNum, unsigned int weaponIndex)
{
    BOOL v2; // [esp+0h] [ebp-10h]
    unsigned int validLatest; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].weaponSelectTime = cgArray[0].time;
    if (cgArray[0].weaponSelect != weaponIndex)
    {
        v2 = weaponIndex && weaponIndex == BG_GetWeaponDef(cgArray[0].weaponSelect)->altWeaponIndex;
        validLatest = ValidLatestPrimaryWeapIdx(weaponIndex);
        if (validLatest)
            cgArray[0].weaponLatestPrimaryIdx = validLatest;
        cgArray[0].weaponSelect = weaponIndex;
        CG_MenuShowNotify(localClientNum, 1);
        if (!v2)
            CL_SetADS(localClientNum, 0);
    }
}

char __cdecl CG_ScopeIsOverlayed(int localClientNum)
{
    float zoom; // [esp+8h] [ebp-4h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState < CA_ACTIVE)
        return 0;
    if (CG_PlayerUsingScopedTurret(localClientNum))
        return 1;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return CG_GetWeapReticleZoom(cgArray, &zoom);
}

int __cdecl CG_PlayerTurretWeaponIdx(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) == 0)
        return 0;
    if (cgArray[0].predictedPlayerState.viewlocked == PLAYERVIEWLOCK_NONE)
        return 0;
    if (cgArray[0].predictedPlayerState.viewlocked_entNum == 1023)
        MyAssertHandler(".\\cgame\\cg_weapons.cpp", 4788, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
    return CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum)->nextState.weapon;
}

bool __cdecl CG_PlayerUsingScopedTurret(int localClientNum)
{
    int weapIdxTurret; // [esp+4h] [ebp-4h]

    weapIdxTurret = CG_PlayerTurretWeaponIdx(localClientNum);
    return weapIdxTurret && BG_GetWeaponDef(weapIdxTurret)->overlayMaterial != 0;
}
