#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_save.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include "g_local.h"
#include "savememory.h"
#include "game_public.h"

#include <xanim/xanim.h>
#include "g_main.h"
#include <script/scr_readwrite.h>
#include <script/scr_animtree.h>
#include "actor_threat.h"
#include "actor_event_listeners.h"
#include <server/sv_public.h>
#include <server/sv_game.h>
#include <gfx_d3d/r_cinematic.h>
#include <script/scr_memorytree.h>
#include <aim_assist/aim_target.h>
#include <DynEntity/DynEntity_client.h>
#include <xanim/dobj_utils.h>
#include <cgame/cg_ents.h>
#include "actor_corpse.h"
#include <qcommon/cmd.h>
#include <script/scr_vm.h>
#include "g_vehicle_path.h"
#include <xanim/xanim_readwrite.h>
#include "savedevice.h"

bool g_useDevSaveArea;

gclient_s tempClient;

char g_pendingLoadName[64]{ 0 };

const char *monthStr[12] =
{
  "JAN",
  "FEB",
  "MAR",
  "APR",
  "MAY",
  "JUN",
  "JUL",
  "AUG",
  "SEP",
  "OCT",
  "NOV",
  "DEC"
};

const saveField_t tagInfoFields[4] =
{
  { offsetof(tagInfo_s, parent), SF_ENTITY },
  { offsetof(tagInfo_s, next),   SF_ENTITY },
  { offsetof(tagInfo_s, name),   SF_STRING },
  { 0, SF_NONE }
};

const saveField_t animscriptedFields[1] = { { 0, SF_NONE } };

const saveField_t gclientFields[5] =
{
  { offsetof(gclient_s, pHitHitEnt),    SF_ENTITY },
  { offsetof(gclient_s, pLookatEnt),    SF_ENTHANDLE },
  { offsetof(gclient_s, useHoldEntity), SF_ENTHANDLE },
  { offsetof(gclient_s, ps.viewmodelIndex), SF_MODELINT },
  { 0, SF_NONE }
};

const saveField_t badplaceFields[2] = { { 8, SF_STRING }, { 0, SF_NONE } };
const saveField_t badplaceBrushParmsFields[2] = { { 0, SF_ENTITY }, { 0, SF_NONE } };
const saveField_t badplaceDefaultParmsFields[1] = { { 0, SF_NONE } };

const saveField_t pathnodeFields[2] =
{
  { offsetof(pathnode_dynamic_t, pOwner), SF_SENTIENTHANDLE },
  { 0, SF_NONE }
};

const saveField_t turretFields[4] =
{
  { offsetof(TurretInfo, target),         SF_ENTHANDLE },
  { offsetof(TurretInfo, manualTarget),   SF_ENTHANDLE },
  { offsetof(TurretInfo, detachSentient), SF_SENTIENTHANDLE },
  { 0, SF_NONE }
};

const saveField_t vehicleFields[14] =
{
  { offsetof(scr_vehicle_s, pathPos.switchNode[0].name),               SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[0].target),             SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[0].script_linkname),    SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[0].script_noteworthy),  SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[1].name),               SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[1].target),             SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[1].script_linkname),    SF_STRING },
  { offsetof(scr_vehicle_s, pathPos.switchNode[1].script_noteworthy),  SF_STRING },
  { offsetof(scr_vehicle_s, lookAtText0),                              SF_STRING },
  { offsetof(scr_vehicle_s, lookAtText1),                              SF_STRING },
  { offsetof(scr_vehicle_s, idleSndEnt),                               SF_ENTHANDLE },
  { offsetof(scr_vehicle_s, engineSndEnt),                             SF_ENTHANDLE },
  { offsetof(scr_vehicle_s, lookAtEnt),                                SF_ENTHANDLE },
  { 0, SF_NONE }
};

const saveField_t threatGroupFields[17] =
{
  { 0,  SF_STRING },  { 2,  SF_STRING },  { 4,  SF_STRING },  { 6,  SF_STRING },
  { 8,  SF_STRING },  { 10, SF_STRING },  { 12, SF_STRING },  { 14, SF_STRING },
  { 16, SF_STRING },  { 18, SF_STRING },  { 20, SF_STRING },  { 22, SF_STRING },
  { 24, SF_STRING },  { 26, SF_STRING },  { 28, SF_STRING },  { 30, SF_STRING },
  { 0, SF_NONE }
};

const saveField_t gentityFields[86] =
{
  { offsetof(gentity_s, client),                  SF_CLIENT },
  { offsetof(gentity_s, actor),                   SF_ACTOR },
  { offsetof(gentity_s, sentient),                SF_SENTIENT },
  { offsetof(gentity_s, scr_vehicle),             SF_VEHICLE },
  { offsetof(gentity_s, pTurretInfo),             SF_TURRETINFO },
  { offsetof(gentity_s, classname),               SF_STRING },
  { offsetof(gentity_s, model),                   SF_MODELUSHORT },
  { offsetof(gentity_s, parent),                  SF_ENTHANDLE },
  { offsetof(gentity_s, target),                  SF_STRING },
  { offsetof(gentity_s, targetname),              SF_STRING },
  { offsetof(gentity_s, chain),                   SF_ENTITY },
  { offsetof(gentity_s, activator),               SF_ENTITY },
  { offsetof(gentity_s, script_linkName),         SF_STRING },
  { offsetof(gentity_s, script_noteworthy),       SF_STRING },
  { offsetof(gentity_s, attachModelNames[0]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[1]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[2]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[3]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[4]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[5]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[6]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[7]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[8]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[9]),     SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[10]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[11]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[12]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[13]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[14]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[15]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[16]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[17]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[18]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[19]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[20]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[21]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[22]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[23]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[24]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[25]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[26]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[27]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[28]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[29]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachModelNames[30]),    SF_MODELUSHORT },
  { offsetof(gentity_s, attachTagNames[0]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[1]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[2]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[3]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[4]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[5]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[6]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[7]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[8]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[9]),       SF_STRING },
  { offsetof(gentity_s, attachTagNames[10]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[11]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[12]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[13]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[14]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[15]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[16]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[17]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[18]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[19]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[20]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[21]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[22]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[23]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[24]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[25]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[26]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[27]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[28]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[29]),      SF_STRING },
  { offsetof(gentity_s, attachTagNames[30]),      SF_STRING },
  { offsetof(gentity_s, r.ownerNum),              SF_ENTHANDLE },
  { offsetof(gentity_s, missileTargetEnt),        SF_ENTHANDLE },
  { offsetof(gentity_s, pAnimTree),               SF_ANIMTREE },
  { offsetof(gentity_s, tagInfo),                 SF_TYPE_TAG_INFO },
  { offsetof(gentity_s, scripted),                SF_TYPE_SCRIPTED },
  { offsetof(gentity_s, tagChildren),             SF_ENTITY },
  { offsetof(gentity_s, snd_wait.notifyString),   SF_STRING },
  { offsetof(gentity_s, lookAtText0),             SF_STRING },
  { offsetof(gentity_s, lookAtText1),             SF_STRING },
  { 0, SF_NONE }
};

const saveField_t actorFields[77] =
{
  { offsetof(actor_s, ent),                                                  SF_ENTITY },
  { offsetof(actor_s, sentient),                                             SF_SENTIENT },
  { offsetof(actor_s, AnimScriptHandle),                                     SF_THREAD },
  { offsetof(actor_s, AnimScriptSpecific.name),                              SF_STRING },
  { offsetof(actor_s, pAnimScriptFunc),                                      SF_ANIMSCRIPT },
  { offsetof(actor_s, pAttackScriptFunc),                                    SF_ANIMSCRIPT },
  { offsetof(actor_s, pPotentialReacquireNode[0]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[1]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[2]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[3]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[4]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[5]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[6]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[7]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[8]),                           SF_PATHNODE },
  { offsetof(actor_s, pPotentialReacquireNode[9]),                           SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[0].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[1].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[2].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[3].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[4].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[5].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[6].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[7].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[8].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[9].pLastKnownNode),                       SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[10].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[11].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[12].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[13].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[14].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[15].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[16].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[17].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[18].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[19].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[20].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[21].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[22].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[23].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[24].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[25].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[26].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[27].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[28].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[29].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[30].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[31].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, sentientInfo[32].pLastKnownNode),                      SF_PATHNODE },
  { offsetof(actor_s, Suppressant[0].pSuppressor),                           SF_SENTIENT },
  { offsetof(actor_s, Suppressant[1].pSuppressor),                           SF_SENTIENT },
  { offsetof(actor_s, Suppressant[2].pSuppressor),                           SF_SENTIENT },
  { offsetof(actor_s, Suppressant[3].pSuppressor),                           SF_SENTIENT },
  { offsetof(actor_s, damageHitLoc),                                         SF_STRING },
  { offsetof(actor_s, damageWeapon),                                         SF_STRING },
  { offsetof(actor_s, properName),                                           SF_STRING },
  { offsetof(actor_s, weaponName),                                           SF_STRING },
  { offsetof(actor_s, pFavoriteEnemy),                                       SF_SENTIENTHANDLE },
  { offsetof(actor_s, pGrenade),                                             SF_ENTHANDLE },
  { offsetof(actor_s, GrenadeTossMethod),                                    SF_STRING },
  { offsetof(actor_s, pTurret),                                              SF_ENTITY },
  { offsetof(actor_s, scriptState),                                          SF_STRING },
  { offsetof(actor_s, lastScriptState),                                      SF_STRING },
  { offsetof(actor_s, stateChangeReason),                                    SF_STRING },
  { offsetof(actor_s, pCloseEnt),                                            SF_ENTHANDLE },
  { offsetof(actor_s, pPileUpActor),                                         SF_ACTOR },
  { offsetof(actor_s, pPileUpEnt),                                           SF_ENTITY },
  { offsetof(actor_s, anim_pose),                                            SF_STRING },
  { offsetof(actor_s, codeGoal.node),                                        SF_PATHNODE },
  { offsetof(actor_s, codeGoal.volume),                                      SF_ENTITY },
  { offsetof(actor_s, scriptGoalEnt),                                        SF_ENTHANDLE },
  { offsetof(actor_s, scriptGoal.node),                                      SF_PATHNODE },
  { offsetof(actor_s, scriptGoal.volume),                                    SF_ENTITY },
  { offsetof(actor_s, fixedNodeSafeVolume),                                  SF_ENTHANDLE },
  { offsetof(actor_s, pDesiredChainPos),                                     SF_PATHNODE },
  { offsetof(actor_s, faceLikelyEnemyPathNode),                              SF_PATHNODE },
  { 0, SF_NONE }
};

const saveField_t sentientFields[10] =
{
  { offsetof(sentient_s, ent),               SF_ENTITY },
  { offsetof(sentient_s, targetEnt),         SF_ENTHANDLE },
  { offsetof(sentient_s, scriptTargetEnt),   SF_ENTHANDLE },
  { offsetof(sentient_s, pClaimedNode),      SF_PATHNODE },
  { offsetof(sentient_s, pPrevClaimedNode),  SF_PATHNODE },
  { offsetof(sentient_s, pActualChainPos),   SF_PATHNODE },
  { offsetof(sentient_s, pNearestNode),      SF_PATHNODE },
  { offsetof(sentient_s, lastAttacker),      SF_ENTITY },
  { offsetof(sentient_s, syncedMeleeEnt),    SF_ENTHANDLE },
  { 0, SF_NONE }
};





void __cdecl TRACK_g_save()
{
    track_static_alloc_internal(&tempClient, sizeof(gclient_s), "tempClient", 9);
}

void __cdecl Scr_FreeFields(const saveField_t *fields, unsigned __int8 *base)
{
    const saveFieldtype_t *p_type; // r11
    const saveField_t *i; // r31
    saveFieldtype_t v5; // r11
    EntHandle *enthand;
    SentientHandle *senthand;

    p_type = &fields->type;
    for (i = fields; i->type; p_type = &i->type)
    {
        v5 = *p_type;
        switch (v5)
        {
        case SF_STRING:
            Scr_SetString((unsigned __int16 *)&base[i->ofs], 0);
            break;
        case SF_ENTHANDLE:
            enthand = (EntHandle *)&base[i->ofs];
            enthand->setEnt(NULL);
            break;
        case SF_SENTIENTHANDLE:
            senthand = (SentientHandle *)&base[i->ofs];
            senthand->setSentient(NULL);
            break;
        }
        ++i;
    }
}

void __cdecl Scr_FreeEntityFields(gentity_s *ent)
{
    Scr_FreeFields(gentityFields, (unsigned char*)&ent->s.eType);
}

void __cdecl Scr_FreeActorFields(actor_s *pActor)
{
    Scr_FreeFields(actorFields, (unsigned __int8 *)pActor);
}

void __cdecl Scr_FreeSentientFields(sentient_s *sentient)
{
    Scr_FreeFields(sentientFields, (unsigned __int8 *)sentient);
}

void G_SaveError(errorParm_t code, SaveErrorType errorType, const char *fmt, ...)
{
    char buf[544];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, 512, fmt, va);
    va_end(va);
    buf[511] = 0;

    const char *errMsg = buf;
    if (errorType)
    {
        if (errorType == SAVE_ERROR_CORRUPT_SAVE)
            errMsg = "PLATFORM_ERR_SAVEGAME_BAD";
    }
    else
    {
        errMsg = "PLATFORM_UNABLE_TO_READ_FROM_DEVICE";
    }

    Com_PrintError(10, "%s", buf);
    Com_Error(code, errMsg);
}

void __cdecl WriteCStyleString(const char *psz, int maxlen, SaveGame *save)
{
    const char *v6; // r11
    int v8; // r31
    int v9; // r4
    __int16 *v10; // r3
    char v11; // [sp+50h] [-40h] BYREF
    __int16 v12; // [sp+52h] [-3Eh] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 610, 0, "%s", "save");
    if (!psz)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 611, 0, "%s", "psz");
    if (maxlen > 0x10000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            612,
            0,
            "%s\n\t(maxlen) = %i",
            "(maxlen <= 65536)",
            maxlen);
    v6 = psz;
    while (*(unsigned __int8 *)v6++)
        ;
    v8 = v6 - psz - 1;
    if (v8 >= maxlen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", v8, maxlen);
    if (maxlen > 256)
    {
        v9 = 2;
        v12 = v8;
        v10 = &v12;
    }
    else
    {
        v9 = 1;
        v11 = v8;
        v10 = (__int16 *)&v11;
    }
    SaveMemory_SaveWrite(v10, v9, save);
    SaveMemory_SaveWrite(psz, v8, save);
}

void __cdecl ReadCStyleString(char *psz, int maxlen, SaveGame *save)
{
    int v6; // r31
    _BYTE v7[2]; // [sp+50h] [-30h] BYREF
    unsigned __int16 v8; // [sp+52h] [-2Eh] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    if (maxlen > 0x10000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            641,
            0,
            "%s\n\t(maxlen) = %i",
            "(maxlen <= 65536)",
            maxlen);
    if (maxlen > 256)
    {
        SaveMemory_LoadRead(&v8, 2, save);
        v6 = v8;
    }
    else
    {
        SaveMemory_LoadRead(v7, 1, save);
        v6 = v7[0];
    }
    if (v6 >= maxlen)
        Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
    SaveMemory_LoadRead(psz, v6, save);
    psz[v6] = 0;
}

void __cdecl WriteWeaponIndex(unsigned int weapon, SaveGame *save)
{
    WeaponDef *WeaponDef; // r29

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 670, 0, "%s", "save");
    if (weapon)
    {
        WeaponDef = BG_GetWeaponDef(weapon);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 674, 0, "%s", "weapDef");
        if (!WeaponDef->szInternalName)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 675, 0, "%s", "weapDef->szInternalName");
        WriteCStyleString(WeaponDef->szInternalName, 256, save);
    }
    else
    {
        WriteCStyleString("", 256, save);
    }
}

int __cdecl ReadWeaponIndex(SaveGame *save)
{
    int v2; // r31
    _BYTE v4[16]; // [sp+50h] [-140h] BYREF
    char v5[304]; // [sp+60h] [-130h] BYREF

    if (!save)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 694, 0, "%s", "save");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    }
    SaveMemory_LoadRead(v4, 1, save);
    v2 = v4[0];
    SaveMemory_LoadRead(v5, v4[0], save);
    v5[v2] = 0;
    if (v5[0])
        return G_GetWeaponIndexForName(v5);
    else
        return 0;
}

void __cdecl WriteItemIndex(int iIndex, SaveGame *save)
{
    gitem_s *v4; // r29
    char v5; // [sp+50h] [-40h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 714, 0, "%s", "save");
    if (iIndex)
    {
        v4 = &bg_itemlist[iIndex];
        if (!v4)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 718, 0, "%s", "pItem");
        if (v4->giType != IT_WEAPON)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 720, 0, "%s", "pItem->giType == IT_WEAPON");
        WriteWeaponIndex(iIndex % 128, save);
        v5 = iIndex / 128;
        SaveMemory_SaveWrite(&v5, 1, save);
    }
    else
    {
        WriteCStyleString("", 256, save);
    }
}

int __cdecl ReadItemIndex(SaveGame *save)
{
    int v2; // r31
    const gitem_s *Item; // r3
    unsigned __int8 v5; // [sp+50h] [-140h] BYREF
    _BYTE v6[15]; // [sp+51h] [-13Fh] BYREF
    char v7[304]; // [sp+60h] [-130h] BYREF

    iassert(save);

    SaveMemory_LoadRead(v6, 1, save);
    v2 = v6[0];
    SaveMemory_LoadRead(v7, v6[0], save);
    v7[v2] = 0;
    if (v7[0] && (SaveMemory_LoadRead(&v5, 1, save), (Item = G_FindItem(v7, v5)) != 0))
        return Item - bg_itemlist;
    else
        return 0;
}

void __cdecl WriteVehicleIndex(__int16 index, SaveGame *save)
{
    const char *VehicleInfoName; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 771, 0, "%s", "save");
    VehicleInfoName = G_GetVehicleInfoName(index);
    WriteCStyleString(VehicleInfoName, 256, save);
}

int __cdecl ReadVehicleIndex(SaveGame *save)
{
    int v2; // r31
    _BYTE v4[16]; // [sp+50h] [-140h] BYREF
    char v5[304]; // [sp+60h] [-130h] BYREF

    if (!save)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 788, 0, "%s", "save");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    }
    SaveMemory_LoadRead(v4, 1, save);
    v2 = v4[0];
    SaveMemory_LoadRead(v5, v4[0], save);
    v5[v2] = 0;
    return G_GetVehicleInfoIndex(v5);
}

void __cdecl WriteField1(const saveField_t *field, const unsigned __int8 *base, unsigned __int8 *original)
{
    EntHandle *v3; // r28
    unsigned int v4; // r31
    unsigned int v5; // r3
    int v6; // r31
    unsigned int v7; // r31
    unsigned int v8; // r31
    unsigned int v9; // r31
    unsigned int v10; // r31
    unsigned int v11; // r31
    unsigned int v12; // r31
    unsigned __int8 *v13; // r11
    int v14; // r11
    int v15; // r31
    const XAnim_s *anims; // r29
    int index; // r29

    EntHandle *enthand;
    SentientHandle *senthand;

    v3 = (EntHandle *)&base[field->ofs];

    switch (field->type)
    {
    case SF_STRING:
        if (v3->number)
            v3->number = 1;
        break;
    case SF_ENTITY:
        if (*(unsigned int *)v3)
        {
            v4 = (*(unsigned int *)v3 - (int)g_entities) / 628 + 1;
            if (v4 > 0x880)
                Com_Error(ERR_DROP, "WriteField1: entity out of range (%i)", v4);

            *(int *)v3 = v4;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_ENTHANDLE:
        enthand = (EntHandle *)&base[field->ofs];
        if (enthand->isDefined())
        {
            v5 = v3->entnum();
            v6 = v5 + 1;
            if ((int)(v5 + 1) > MAX_GENTITIES || v6 < 0)
                Com_Error(ERR_DROP, "WriteField1: entity out of range (%i)", v5 + 1);

            *(int *)v3 = v6;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_CLIENT:
        if (*(unsigned int *)v3)
        {
            v7 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.clients) / 46104 + 1;
            if (v7 >= 2)
                Com_Error(ERR_DROP, "WriteField1: client out of range (%i)", v7);
            *(int *)v3 = v7;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_ACTOR:
        if (*(unsigned int *)v3)
        {
            v8 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.actors) / 7824 + 1;
            if (v8 > 0x20)
                Com_Error(ERR_DROP, "WriteField1: actor out of range (%i)", v8);
            *(int *)v3 = v8;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_SENTIENT:
        if (*(unsigned int *)v3)
        {
            v9 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.sentients) / 116 + 1;
            if (v9 >= 0x22)
                Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", v9);
            *(int *)v3 = v9;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_SENTIENTHANDLE:
        senthand = (SentientHandle *)&base[field->ofs];
        if (senthand->isDefined())
        {
            //v10 = SentientHandle::sentient((SentientHandle *)v3) - level.sentients + 1;
            v10 = senthand->sentient() - level.sentients + 1;
            if (v10 >= 0x22)
                Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", v10);

            *(int *)v3 = v10;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_VEHICLE:
        if (*(unsigned int *)v3)
        {
            v11 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.vehicles) / 824 + 1;
            if (v11 > 0x40)
                Com_Error(ERR_DROP, "WriteField1: vehicle out of range (%i)", v11);
            *(int *)v3 = v11;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_TURRETINFO:
        if (*(unsigned int *)v3)
        {
            v12 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.turrets) / 188 + 1;
            if (v12 > 0x20)
                Com_Error(ERR_DROP, "WriteField1: turret out of range (%i)", v12);
            *(int *)v3 = v12;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_THREAD:
        v3->number = Scr_ConvertThreadToSave(v3->number);
        break;
    case SF_ANIMSCRIPT:
        v13 = (unsigned __int8 *)*(unsigned int *)v3;
        if (*(unsigned int *)v3)
        {
            if (v13 == original + 504)
            {

                *(int *)v3 = -1;
            }
            else
            {
                v14 = (v13 - (unsigned __int8 *)&g_scr_data.anim) >> 3;
                v15 = v14 + 1;
                if (v14 + 1 <= 0 || v15 > 298)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
                        908,
                        0,
                        "%s\n\t(index) = %i",
                        "(index > 0 && index <= (int)( sizeof( AnimScriptList ) * MAX_AI_SPECIES / sizeof( scr_animscript_t ) ))",
                        v14 + 1);
                *(int *)v3 = v15;
            }
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_PATHNODE:
        *(int *)v3 = Path_SaveIndex(*(const pathnode_t **)v3);
        break;
    case SF_ANIMTREE:
        if (*(unsigned int *)v3)
        {
            anims = XAnimGetAnims(*(const XAnimTree_s **)v3);
            iassert(anims);
            index = Scr_GetAnimsIndex(anims);
            iassert(index);
            *(int *)v3 = index;
        }
        else
        {
            *(unsigned int *)v3 = 0;
        }
        break;
    case SF_TYPE_TAG_INFO:
    case SF_TYPE_SCRIPTED:
        //*v3 = (EntHandle)((_cntlzw((unsigned int)*v3) & 0x20) == 0);
        *(int *)v3 = (*(unsigned int *)v3 != 0);
        break;
    case SF_MODELUSHORT:
    case SF_MODELINT:
        return;
    default:
        Com_Error(ERR_DROP, "WriteField1: unknown field type");
        break;
    }
}

void __cdecl WriteField2(const saveField_t *field, unsigned __int8 *base, SaveGame *save)
{
    saveFieldtype_t type; // r11
    int ofs; // r30
    const void *v8; // r4
    MemoryFile *v9; // r3
    unsigned int v10; // r3
    const void *v11; // r4
    MemoryFile *v12; // r3
    unsigned int v13; // r3
    MemoryFile *memFile; // r3
    unsigned int UsedSize; // r3
    const char *v16; // r30
    MemoryFile *v17; // r3
    MemoryFile *v18; // r3
    unsigned int v19; // r3
    unsigned __int8 v20[96]; // [sp+50h] [-F0h] BYREF
    unsigned __int8 v21[144]; // [sp+B0h] [-90h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 961, 0, "%s", "save");
    type = field->type;
    ofs = field->ofs;
    switch (type)
    {
    case SF_STRING:
        memFile = SaveMemory_GetMemoryFile(save);
        UsedSize = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("string", UsedSize);
        if (*(_WORD *)&base[ofs])
        {
            v16 = SL_ConvertToString(*(unsigned __int16 *)&base[ofs]);
            v17 = SaveMemory_GetMemoryFile(save);
            MemFile_WriteCString(v17, v16);
        }
        goto LABEL_12;
    case SF_TYPE_TAG_INFO:
        v11 = *(const void **)&base[ofs];
        if (!v11)
            return;
        memcpy(v21, v11, 0x70u);
        v12 = SaveMemory_GetMemoryFile(save);
        v13 = MemFile_GetUsedSize(v12);
        //ProfMem_Begin("tagInfo", v13);
        G_WriteStruct(tagInfoFields, *(unsigned __int8 **)&base[ofs], v21, 112, save);
        goto LABEL_12;
    case SF_TYPE_SCRIPTED:
        v8 = *(const void **)&base[ofs];
        if (v8)
        {
            memcpy(v20, v8, sizeof(v20));
            v9 = SaveMemory_GetMemoryFile(save);
            v10 = MemFile_GetUsedSize(v9);
            //ProfMem_Begin("animscripted", v10);
            G_WriteStruct(animscriptedFields, *(unsigned __int8 **)&base[ofs], v20, 96, save);
        LABEL_12:
            v18 = SaveMemory_GetMemoryFile(save);
            v19 = MemFile_GetUsedSize(v18);
            //ProfMem_End(v19);
        }
        break;
    }
}

void __cdecl ReadField(const saveField_t *field, unsigned __int8 *base, SaveGame *save)
{
    EntHandle *v7; // r31
    MemoryFile *MemoryFile; // r3
    const char *CString; // r3
    int v10; // r30
    bool v11; // cr58
    int v12; // r30
    int v13; // r30
    bool v14; // cr58
    int v15; // r30
    bool v16; // cr58
    int v17; // r30
    bool v18; // cr58
    int v19; // r30
    int v20; // r30
    bool v21; // cr58
    EntHandle v22; // r30
    int v23; // r30
    XAnim_s *anims; // r30
    unsigned __int8 *v25; // r3
    unsigned __int8 *v26; // r3

    iassert(save);

    v7 = (EntHandle *)&base[field->ofs];

    SentientHandle *senthand;

    switch (field->type)
    {
    case SF_STRING:
        if (v7->number)
        {
            MemoryFile = SaveMemory_GetMemoryFile(save);
            CString = MemFile_ReadCString(MemoryFile);
            v7->number = SL_GetString(CString, 0);
        }
        break;
    case SF_ENTITY:
        v10 = *(int*)v7;
        if (*(unsigned int *)v7 > MAX_GENTITIES || (v11 = v10 == 0, v10 < 0))
        {
            Com_Error(ERR_DROP, "ReadField: entity out of range (%i)", *v7);
            v11 = v10 == 0;
        }
        if (v11)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&g_entities[v10 - 1];
        break;
    case SF_ENTHANDLE:
        v12 = (int)*(int *)v7;
        if (*(unsigned int *)v7 > MAX_GENTITIES || v12 < 0)
            Com_Error(ERR_DROP, "ReadField: entity out of range (%i)", *v7);
        *(int*)v7 = 0;
        if (v12)
            v7->setEnt(&g_entities[v12 - 1]);
        break;
    case SF_CLIENT:
        v13 = (int)*(int*)v7;
        if (*(unsigned int *)v7 > 1 || (v14 = v13 == 0, v13 < 0))
        {
            Com_Error(ERR_DROP, "ReadField: client out of range (%i)", *v7);
            v14 = v13 == 0;
        }
        if (v14)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&level.clients[v13 - 1];
        break;
    case SF_ACTOR:
        v15 = (int)*(int*)v7;
        if (*(unsigned int *)v7 > 32 || (v16 = v15 == 0, v15 < 0))
        {
            Com_Error(ERR_DROP, "ReadField: actor out of range (%i)", *v7);
            v16 = v15 == 0;
        }
        if (v16)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&level.actors[v15 - 1];
        break;
    case SF_SENTIENT:
        v17 = (int)*(int*)v7;
        if (*(unsigned int *)v7 > 33 || (v18 = v17 == 0, v17 < 0))
        {
            Com_Error(ERR_DROP, "ReadField: sentient out of range (%i)", *v7);
            v18 = v17 == 0;
        }
        if (v18)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&level.sentients[v17 - 1];
        break;
    case SF_SENTIENTHANDLE:
        v19 = (int)*(int*)v7;
        if (*(unsigned int *)v7 > 33 || v19 < 0)
            Com_Error(ERR_DROP, "ReadField: sentient out of range (%i)", *v7);

        *(int*)v7 = 0;
        if (v19)
        {
            senthand = (SentientHandle *)v7;
            senthand->setSentient(&level.sentients[v19 - 1]);
            //SentientHandle::setSentient((SentientHandle *)v7, &level.sentients[v19 - 1]);
        }
        break;
    case SF_VEHICLE:
        v20 = (int)*(int*)v7;
        if (*(unsigned int *)v7 > 64 || (v21 = v20 == 0, v20 < 0))
        {
            Com_Error(ERR_DROP, "ReadField: vehicle out of range (%i)", *v7);
            v21 = v20 == 0;
        }
        if (v21)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&level.vehicles[v20 - 1];
        break;
    case SF_TURRETINFO:
        v22 = *v7;
        if (*(unsigned int *)v7 > 0x40u)
            Com_Error(ERR_DROP, "ReadField: turret out of range (%i)", *v7);
        if (!*(unsigned int *)&v22)
            goto LABEL_58;
        *(uintptr_t *)v7 = (uintptr_t)&level.turrets[*(unsigned int *)&v22 - 1];
        break;
    case SF_THREAD:
        v7->number = Scr_ConvertThreadFromLoad(v7->number);
        break;
    case SF_ANIMSCRIPT:
        v23 = (int)*(int*)v7;

        if (v23 > 298 || v23 < -1)
            Com_Error(ERR_DROP, "ReadField: animscript out of range (%i)", *v7);
        if (!v23)
            goto LABEL_58;
        if (v23 == -1)
            *(uintptr_t *)v7 = (uintptr_t)(base + 504);
        else
            *(uintptr_t *)v7 = (uintptr_t)(&g_scr_data.scripted_init + 2 * v23);
        break;
    case SF_PATHNODE:
        *(uintptr_t *)v7 = (uintptr_t)Path_LoadNode(*(unsigned int*)v7);
        break;
    case SF_ANIMTREE:
        if (*(unsigned int*)v7)
        {
            anims = Scr_GetAnims((unsigned int)*(unsigned int*)v7);
            iassert(anims);
            *(uintptr_t *)v7 = (uintptr_t)Com_XAnimCreateSmallTree(anims);
        }
        else
        {
        LABEL_58:
            *(unsigned int*)v7 = 0;
        }
        break;
    case SF_TYPE_TAG_INFO:
        if (*(unsigned int *)v7)
        {
            v25 = (unsigned __int8 *)MT_Alloc(112, 17);
            *(uintptr_t *)v7 = (uintptr_t)v25;
            G_ReadStruct(tagInfoFields, v25, 112, save);
        }
        break;
    case SF_TYPE_SCRIPTED:
        if (*(unsigned int *)v7)
        {
            v26 = (unsigned __int8 *)MT_Alloc(96, 17);
            *(uintptr_t *)v7 = (uintptr_t)v26;
            G_ReadStruct(animscriptedFields, v26, 96, save);
        }
        break;
    case SF_MODELUSHORT:
        v7->number = *(unsigned __int16 *)((char *)level.modelMap + __ROL4__(v7->number, 1));
        break;
    case SF_MODELINT:

        *(int *)v7 = level.modelMap[*(unsigned int *)v7];
        break;
    default:
        Com_Error(ERR_DROP, "ReadField: unknown field type");
        break;
    }
}

void __cdecl G_WriteStruct(
    const saveField_t *fields,
    unsigned __int8 *original,
    const unsigned __int8 *source,
    int sourcesize,
    SaveGame *save)
{
    const saveField_t *i; // r30
    unsigned int UsedSize; // r3
    unsigned int v14; // r3

    iassert(save);

    for (i = fields; i->type; ++i)
        WriteField1(i, source, original);

    //ProfMem_Begin("writestruct struct", UsedSize);
    SaveMemory_SaveWrite(source, sourcesize, save);
    //ProfMem_End(v14);

    for (; fields->type; ++fields)
        WriteField2(fields, original, save);
}

void __cdecl G_ReadStruct(const saveField_t *fields, unsigned __int8 *dest, int tempsize, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1214, 0, "%s", "save");
    SaveMemory_LoadRead(dest, tempsize, save);
    for (; fields->type; ++fields)
        ReadField(fields, dest, save);
}

void __cdecl WriteClient(gclient_s *cl, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1232, 0, "%s", "save");
    memcpy(&tempClient, cl, sizeof(tempClient));
    tempClient.ps.events[0] = 0;
    tempClient.ps.events[1] = 0;
    tempClient.ps.events[2] = 0;
    tempClient.ps.events[3] = 0;
    tempClient.ps.eventParms[0] = 0;
    tempClient.ps.eventParms[1] = 0;
    tempClient.ps.eventParms[2] = 0;
    tempClient.ps.eventParms[3] = 0;
    tempClient.ps.eventSequence = 0;
    tempClient.ps.oldEventSequence = 0;
    tempClient.ps.entityEventSequence = 0;
    G_WriteStruct(gclientFields, (unsigned __int8 *)cl, (const unsigned __int8 *)&tempClient, 46104, save);
    SaveMemory_SaveWrite(&cl->pers.cmd.buttons, 4, save);
    WriteWeaponIndex(cl->pers.cmd.weapon, save);
    WriteWeaponIndex(cl->pers.cmd.offHandIndex, save);
}

void __cdecl ReadClient(gclient_s *client, SaveGame *save)
{
    unsigned __int8 WeaponIndex; // r3
    int weapon; // r4
    int buttons; // r3
    unsigned __int8 v7; // r11
    int v8; // r5

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1262, 0, "%s", "save");
    G_ReadStruct(gclientFields, (unsigned __int8 *)client, 46104, save);
    SaveMemory_LoadRead(&client->pers.cmd.buttons, 4, save);
    client->pers.cmd.weapon = ReadWeaponIndex(save);
    WeaponIndex = ReadWeaponIndex(save);
    weapon = client->pers.cmd.weapon;
    v7 = WeaponIndex;
    v8 = WeaponIndex;
    buttons = client->pers.cmd.buttons;
    client->pers.cmd.offHandIndex = v7;
    SV_SetUsercmdButtonsWeapons(buttons, weapon, v8);
}

void WriteEntity(gentity_s *ent, SaveGame *save)
{
    unsigned int UsedSize; // r3
    unsigned int v7; // r3
    unsigned __int8 v8[632]; // [sp+50h] [-290h] BYREF

    iassert(save);
    memcpy(v8, ent, sizeof(gentity_s));
    //ProfMem_Begin("WriteStruct", UsedSize);
    G_WriteStruct(gentityFields, (unsigned char *)ent, v8, sizeof(gentity_s), save);
    //ProfMem_End(v7);
    if (ent->s.weapon)
    {
        WriteWeaponIndex(ent->s.weapon, save);
        SaveMemory_SaveWrite(&ent->s.weaponModel, 1, save);
    }
    WriteEntityDisconnectedLinks(ent, save);
}

void __cdecl ReadEntity(gentity_s *ent, SaveGame *save)
{
    _BYTE v4[8]; // [sp+50h] [-20h] BYREF

    iassert(save);
    G_ReadStruct(gentityFields, (unsigned char*)ent, sizeof(gentity_s), save);
    if (ent->s.weapon)
    {
        ent->s.weapon = ReadWeaponIndex(save);
        SaveMemory_LoadRead(v4, 1, save);
    }
    ReadEntityDisconnectedLinks(ent, save);
    if (ent->snd_wait.notifyString)
    {
        if (ent->snd_wait.duration < 0)
            G_AddEvent(ent, EV_SOUND_ALIAS_ADD_NOTIFY, ent->snd_wait.index);
    }
}

void __cdecl WriteActorPotentialCoverNodes(actor_s *pActor, SaveGame *save)
{
    int v4; // r30
    pathnode_t **pPotentialCoverNode; // r31
    int v6; // [sp+50h] [-30h] BYREF

    v4 = 0;
    if (pActor->iPotentialCoverNodeCount > 0)
    {
        pPotentialCoverNode = pActor->pPotentialCoverNode;
        do
        {
            v6 = Path_SaveIndex(*pPotentialCoverNode);
            SaveMemory_SaveWrite(&v6, 4, save);
            ++v4;
            ++pPotentialCoverNode;
        } while (v4 < pActor->iPotentialCoverNodeCount);
    }
}

void __cdecl ReadActorPotentialCoverNodes(actor_s *pActor, SaveGame *save)
{
    int v4; // r30
    pathnode_t **pPotentialCoverNode; // r31
    unsigned int v6; // [sp+50h] [-30h] BYREF

    v4 = 0;
    if (pActor->iPotentialCoverNodeCount > 0)
    {
        pPotentialCoverNode = pActor->pPotentialCoverNode;
        do
        {
            SaveMemory_LoadRead(&v6, 4, save);
            *pPotentialCoverNode = Path_LoadNode(v6);
            ++v4;
            ++pPotentialCoverNode;
        } while (v4 < pActor->iPotentialCoverNodeCount);
    }
}

void __cdecl WriteActor(actor_s *pActor, SaveGame *save)
{
    unsigned __int8 v4[3832]; // [sp+50h] [-F10h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1365, 0, "%s", "save");
    SaveMemory_SaveWrite(&pActor->inuse, 1, save);
    if (pActor->inuse)
    {
        memcpy(v4, pActor, 0xEECu);
        G_WriteStruct(actorFields, (unsigned __int8 *)pActor, v4, 3820, save);
        WriteActorPotentialCoverNodes(pActor, save);
    }
}

void __cdecl ReadActor(actor_s *pActor, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1385, 0, "%s", "save");
    SaveMemory_LoadRead(&pActor->inuse, 1, save);
    if (pActor->inuse)
    {
        G_ReadStruct(actorFields, (unsigned __int8 *)pActor, 3820, save);
        if (!pActor->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1394, 0, "%s", "pActor->inuse");
        ReadActorPotentialCoverNodes(pActor, save);
        pActor->pszDebugInfo = "";
    }
}

void __cdecl WriteSentient(sentient_s *sentient, SaveGame *save)
{
    unsigned __int8 v4[120]; // [sp+50h] [-90h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1420, 0, "%s", "save");
    SaveMemory_SaveWrite(&sentient->inuse, 1, save);
    if (sentient->inuse)
    {
        memcpy(v4, sentient, 0x74u);
        G_WriteStruct(sentientFields, (unsigned __int8 *)sentient, v4, 116, save);
    }
}

void __cdecl ReadSentient(sentient_s *sentient, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1437, 0, "%s", "save");
    SaveMemory_LoadRead(&sentient->inuse, 1, save);
    if (sentient->inuse)
    {
        G_ReadStruct(sentientFields, (unsigned __int8 *)sentient, 116, save);
        if (!sentient->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1446, 0, "%s", "sentient->inuse");
    }
}

void __cdecl WriteVehicle(scr_vehicle_s *pVehicle, SaveGame *save)
{
    unsigned __int8 v5[824]; // [sp+60h] [-350h] BYREF

    iassert(save);

    memcpy(v5, pVehicle, sizeof(v5));
    unsigned int v4 = (pVehicle->entNum != ENTITYNUM_NONE);
    SaveMemory_SaveWrite(&v4, 4, save);
    if (v4)
    {
        G_WriteStruct(vehicleFields, (unsigned __int8 *)pVehicle, v5, sizeof(scr_vehicle_s), save);
        WriteVehicleIndex(pVehicle->infoIdx, save);
    }
}

void __cdecl ReadVehicle(scr_vehicle_s *pVehicle, SaveGame *save)
{
    int v4; // [sp+50h] [-20h] BYREF

    v4 = 0;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1483, 0, "%s", "save");
    SaveMemory_LoadRead(&v4, 4, save);
    if (v4)
    {
        G_ReadStruct(vehicleFields, (unsigned __int8 *)pVehicle, 824, save);
        pVehicle->infoIdx = ReadVehicleIndex(save);
    }
}

void __cdecl WriteTurretInfo(TurretInfo *pTurretInfo, SaveGame *save)
{
    unsigned __int8 v5[200]; // [sp+60h] [-E0h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1508, 0, "%s", "save");
    memcpy(v5, pTurretInfo, 0xBCu);
    unsigned int v4 = pTurretInfo->inuse;
    SaveMemory_SaveWrite(&v4, 4, save);
    if (v4)
        G_WriteStruct(turretFields, (unsigned __int8 *)pTurretInfo, v5, 188, save);
}

void __cdecl ReadTurretInfo(TurretInfo *pTurretInfo, SaveGame *save)
{
    int v4; // [sp+50h] [-20h] BYREF

    v4 = 0;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1527, 0, "%s", "save");
    SaveMemory_LoadRead(&v4, 4, save);
    if (v4)
        G_ReadStruct(turretFields, (unsigned __int8 *)pTurretInfo, 188, save);
}

void __cdecl WritePathNodes(SaveGame *save)
{
    pathnode_t *i; // r29
    unsigned __int8 *v3; // r10
    pathnode_dynamic_t *p_dynamic; // r11
    int v5; // ctr
    int wLinkCount; // r11
    int v7; // r30
    int v8; // r31
    unsigned int v9[4]; // [sp+50h] [-60h] BYREF
    unsigned __int8 v10[80]; // [sp+60h] [-50h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1552, 0, "%s", "save");
    for (i = Path_FirstNode(-1); i; i = Path_NextNode(i, -1))
    {
        v3 = v10;
        p_dynamic = &i->dynamic;
        v5 = 8;
        do
        {
            *(SentientHandle *)v3 = p_dynamic->pOwner;
            p_dynamic = (pathnode_dynamic_t *)((char *)p_dynamic + 4);
            v3 += 4;
            --v5;
        } while (v5);
        G_WriteStruct(pathnodeFields, (unsigned __int8 *)&i->dynamic, v10, 32, save);
        wLinkCount = i->dynamic.wLinkCount;
        if (wLinkCount != i->constant.totalLinkCount)
        {
            v7 = 0;
            if (wLinkCount > 0)
            {
                v8 = 0;
                do
                {
                    v9[0] = i->constant.Links[v8].nodeNum;
                    SaveMemory_SaveWrite(v9, 4, save);
                    ++v7;
                    ++v8;
                } while (v7 < i->dynamic.wLinkCount);
            }
        }
    }
}

void __cdecl ReadPathNodes(SaveGame *save)
{
    pathnode_t *i; // r31
    int wLinkCount; // r11
    int v4; // r27
    int v5; // r29
    int v6; // r30
    int j; // r28
    pathlink_s *Links; // r11
    pathlink_s *v9; // r7
    float *p_fDist; // r11
    float fDist; // r6
    int v12; // r10
    int v13; // r9
    int v14; // r8
    unsigned int *v15; // r11
    int v16; // [sp+50h] [-60h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1584, 0, "%s", "save");
    for (i = Path_FirstNode(-1); i; i = Path_NextNode(i, -1))
    {
        G_ReadStruct(pathnodeFields, (unsigned __int8 *)&i->dynamic, 32, save);
        wLinkCount = i->dynamic.wLinkCount;
        if (wLinkCount != i->constant.totalLinkCount)
        {
            v4 = 0;
            if (wLinkCount > 0)
            {
                v5 = 0;
                do
                {
                    SaveMemory_LoadRead(&v16, 4, save);
                    v6 = i->constant.totalLinkCount - 1;
                    for (j = v6; ; --j)
                    {
                        if (v6 < 0)
                            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1597, 0, "%s", "j >= 0");
                        if (i->constant.Links[j].nodeNum == v16)
                            break;
                        --v6;
                    }
                    Links = i->constant.Links;
                    ++v4;
                    v9 = &Links[v5];
                    fDist = Links[v5].fDist;
                    p_fDist = &Links[v6].fDist;
                    v12 = *(unsigned int *)p_fDist;
                    v13 = *((unsigned int *)p_fDist + 1);
                    v14 = *((unsigned int *)p_fDist + 2);
                    *p_fDist = fDist;
                    p_fDist[1] = *(float *)&v9->nodeNum;
                    p_fDist[2] = *(float *)v9->ubBadPlaceCount;
                    v15 = (unsigned int *)&i->constant.Links[v5++].fDist;
                    *v15 = v12;
                    v15[1] = v13;
                    v15[2] = v14;
                } while (v4 < i->dynamic.wLinkCount);
            }
        }
    }
}

const saveField_t *__cdecl BadPlaceParmSaveFields(const badplace_t *badplace)
{
    if (!badplace)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
    if (badplace->type == 2)
        return badplaceBrushParmsFields;
    else
        return badplaceDefaultParmsFields;
}

void __cdecl WriteBadPlaces(SaveGame *save)
{
    int v2; // r27
    unsigned __int8 *v3; // r31
    unsigned __int8 *v4; // r10
    unsigned __int8 *v5; // r11
    int v6; // ctr
    unsigned __int8 *v7; // r10
    unsigned int *v8; // r11
    int v9; // ctr
    const saveField_t *v10; // r3
    unsigned __int8 v11[32]; // [sp+50h] [-90h] BYREF
    unsigned __int8 v12[112]; // [sp+70h] [-70h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1641, 0, "%s", "save");
    v2 = 32;
    v3 = (unsigned __int8 *)g_badplaces;
    do
    {
        v4 = v12;
        v5 = v3;
        v6 = 10;
        do
        {
            *(unsigned int *)v4 = *(unsigned int *)v5;
            v5 += 4;
            v4 += 4;
            --v6;
        } while (v6);
        G_WriteStruct(badplaceFields, v3, v12, 12, save);
        v7 = v11;
        v8 = (unsigned int*)(v3 + 12);
        v9 = 7;
        do
        {
            *(unsigned int *)v7 = *v8++;
            v7 += 4;
            --v9;
        } while (v9);
        if (!v3)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
        v10 = badplaceDefaultParmsFields;
        if (v3[10] == 2)
            v10 = badplaceBrushParmsFields;
        G_WriteStruct(v10, v3 + 12, v11, 28, save);
        --v2;
        v3 += 40;
    } while (v2);
}

void __cdecl ReadBadPlaces(SaveGame *save)
{
    int loops; // r27
    badplace_t *badplace; // r31
    const saveField_t *v4; // r3

    iassert(save);
    loops = ARRAY_COUNT(g_badplaces);
    badplace = g_badplaces;
    do
    {
        G_ReadStruct(badplaceFields, (unsigned __int8 *)badplace, 12, save);
        if (!badplace)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
        v4 = badplaceDefaultParmsFields;
        if (badplace->type == 2)
            v4 = badplaceBrushParmsFields;
        G_ReadStruct(v4, (unsigned __int8 *)&badplace->parms, 28, save);
        if (badplace->type)
            Path_UpdateBadPlaceCount(badplace, 1);
        --loops;
        ++badplace;
    } while (loops);
}

void __cdecl WriteThreatBiasGroups(SaveGame *save)
{
    unsigned __int8 v2[1064]; // [sp+50h] [-440h] BYREF

    memcpy(v2, &g_threatBias, 0x424u);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1686, 0, "%s", "save");
    G_WriteStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, v2, 1060, save);
}

void __cdecl ReadThreatBiasGroups(SaveGame *save)
{
    G_ReadStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, 1060, save);
}

void __cdecl WriteAIEventListeners(SaveGame *save)
{
    int v2[4]; // [sp+50h] [-20h] BYREF

    v2[0] = Actor_EventListener_GetCount();
    SaveMemory_SaveWrite(v2, 4, save);
    if (v2[0])
        SaveMemory_SaveWrite(g_AIEVlisteners, 8 * v2[0], save);
}

void __cdecl ReadAIEventListeners(SaveGame *save)
{
    int v2[4]; // [sp+50h] [-20h] BYREF

    if (Actor_EventListener_GetCount())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            1724,
            0,
            "%s",
            "Actor_EventListener_GetCount() == 0");
    SaveMemory_LoadRead(v2, 4, save);
    Actor_EventListener_SetCount(v2[0]);
    if (v2[0])
        SaveMemory_LoadRead(g_AIEVlisteners, 8 * v2[0], save);
}

char *__cdecl G_Save_DateStr()
{
    qtime_s v1; // [sp+50h] [-30h] BYREF

    Com_RealTime(&v1);
    return va("%s %i, %i", monthStr[v1.tm_mon], v1.tm_mday, v1.tm_year + 1900);
}

void __cdecl G_SaveConfigstrings(int iFirst, int iCount, SaveGame *save)
{
    int i; // r29
    char *v7; // r11
    int v9; // r11
    int v10; // r31
    _WORD v11[8]; // [sp+50h] [-460h] BYREF
    char v12[1104]; // [sp+60h] [-450h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1773, 0, "%s", "save");
    for (i = 0; i < iCount; ++i)
    {
        SV_GetConfigstring(i + iFirst, v12, 1024);
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 610, 0, "%s", "save");
        v7 = v12;
        while (*v7++)
            ;
        v9 = v7 - v12 - 1;
        v10 = v9;
        if (v9 >= 1024)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", v9, 1024);
        v11[0] = v10;
        SaveMemory_SaveWrite(v11, 2, save);
        SaveMemory_SaveWrite(v12, v10, save);
    }
}

void __cdecl G_LoadConfigstrings(int iFirst, int iCount, SaveGame *save)
{
    int i; // r31
    int v7; // r30
    _WORD v8[8]; // [sp+50h] [-470h] BYREF
    char v9[1120]; // [sp+60h] [-460h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1795, 0, "%s", "save");
    for (i = 0; i < iCount; ++i)
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v8, 2, save);
        v7 = v8[0];
        if (v8[0] >= 0x400u)
            Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
        SaveMemory_LoadRead(v9, v7, save);
        v9[v7] = 0;
        SV_SetConfigstring(i + iFirst, v9);
    }
}

void __cdecl G_LoadModelPrecacheList(SaveGame *save)
{
    unsigned __int16 *modelMap; // r30
    int v3; // r31
    unsigned __int16 v4; // r3
    _WORD v5[8]; // [sp+50h] [-460h] BYREF
    char v6[1104]; // [sp+60h] [-450h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1818, 0, "%s", "save");
    modelMap = level.modelMap;
    do
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v5, 2, save);
        v3 = v5[0];
        if (v5[0] >= 0x400u)
            Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
        SaveMemory_LoadRead(v6, v3, save);
        v6[v3] = 0;
        if (v6[0])
            v4 = G_ModelIndex(v6);
        else
            v4 = 0;
        *modelMap++ = v4;
    } while ((int)modelMap < (int)&level.priorityNodeBias);
}

void __cdecl G_ClearConfigstrings(int iFirst, int iCount)
{
    int i; // r31

    for (i = 0; i < iCount; ++i)
        SV_SetConfigstring(i + iFirst, "");
}

void __cdecl G_ClearAllConfigstrings()
{
    int i; // r30
    int j; // r30
    int k; // r30
    int m; // r30
    int n; // r30
    int ii; // r30
    int jj; // r30

    for (i = 0; i < 100; ++i)
        SV_SetConfigstring(i + CS_EFFECT_NAMES, "");
    for (j = 0; j < 256; ++j)
        SV_SetConfigstring(j + CS_EFFECT_TAGS, "");
    for (k = 0; k < 512; ++k)
        SV_SetConfigstring(k + CS_SOUNDALIASES, "");
    for (m = 0; m < 128; ++m)
        SV_SetConfigstring(m + CS_SERVER_MATERIALS, "");
    for (n = 0; n < 1023; ++n)
        SV_SetConfigstring(n + CS_LOCALIZED_STRINGS, "");

    SV_SetConfigstring(CS_AMBIENT, "");

    for (ii = 0; ii < 16; ++ii)
        SV_SetConfigstring(ii + CS_OBJECTIVES, "");
    SV_SetConfigstring(CS_CULLDIST, "");
    SV_SetConfigstring(CS_SUNLIGHT, "");
    SV_SetConfigstring(CS_SUNDIR, "");
    SV_SetConfigstring(CS_MINIMAP, "");
    SV_SetConfigstring(CS_NIGHTVISION, "");
    for (jj = 0; jj < 32; ++jj)
        SV_SetConfigstring(jj + CS_TARGETS, "");
}

void __cdecl G_SaveInitConfigstrings(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1871, 0, "%s", "save");
    G_SaveConfigstrings(1123, 512, save);    // CS_MODELS             (was Xbox 1155)
    G_SaveConfigstrings(2147, 100, save);    // CS_EFFECT_NAMES       (was Xbox 2179)
    G_SaveConfigstrings(2247, 256, save);    // CS_EFFECT_TAGS        (was Xbox 2279)
    G_SaveConfigstrings(1635, 512, save);    // CS_SOUNDALIASES
    G_SaveConfigstrings(2551, 128, save);    // CS_SERVER_MATERIALS   (was Xbox 2583)
    G_SaveConfigstrings(91, 1023, save);     // CS_LOCALIZED_STRINGS
    G_SaveConfigstrings(1114, 1, save);      // CS_AMBIENT
    G_SaveConfigstrings(6, 1, save);         // CS_CULLDIST
    G_SaveConfigstrings(7, 1, save);         // CS_SUNLIGHT
    G_SaveConfigstrings(8, 1, save);         // CS_SUNDIR
    G_SaveConfigstrings(1116, 1, save);      // CS_MINIMAP            (was Xbox 1148)
    G_SaveConfigstrings(1119, 1, save);      // CS_NIGHTVISION        (was Xbox 1151)
    G_SaveConfigstrings(1117, 1, save);      // CS_VISIONSET_NAKED    (was Xbox 1149)
    G_SaveConfigstrings(1118, 1, save);      // CS_VISIONSET_NIGHT    (was Xbox 1150)
}

void __cdecl G_LoadInitConfigstrings(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1896, 0, "%s", "save");
    G_LoadModelPrecacheList(save);
    G_LoadConfigstrings(2147, 100, save);    // CS_EFFECT_NAMES       (was Xbox 2179)
    G_LoadConfigstrings(2247, 256, save);    // CS_EFFECT_TAGS        (was Xbox 2279)
    G_LoadConfigstrings(1635, 512, save);    // CS_SOUNDALIASES
    G_LoadConfigstrings(2551, 128, save);    // CS_SERVER_MATERIALS   (was Xbox 2583)
    G_LoadConfigstrings(91, 1023, save);     // CS_LOCALIZED_STRINGS
    G_LoadConfigstrings(1114, 1, save);      // CS_AMBIENT
    G_LoadConfigstrings(6, 1, save);         // CS_CULLDIST
    G_LoadConfigstrings(7, 1, save);         // CS_SUNLIGHT
    G_LoadConfigstrings(8, 1, save);         // CS_SUNDIR
    G_LoadConfigstrings(1116, 1, save);      // CS_MINIMAP            (was Xbox 1148)
    G_LoadConfigstrings(1119, 1, save);      // CS_NIGHTVISION        (was Xbox 1151)
    G_LoadConfigstrings(1117, 1, save);      // CS_VISIONSET_NAKED    (was Xbox 1149)
    G_LoadConfigstrings(1118, 1, save);      // CS_VISIONSET_NIGHT    (was Xbox 1150)
}

void __cdecl G_SaveItems(SaveGame *save)
{
    int v2; // r31
    _BYTE v3[8]; // [sp+50h] [-20h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1927, 0, "%s", "save");
    v2 = 1;
    v3[0] = 0;
    do
    {
        if (IsItemRegistered(v2))
        {
            SaveMemory_SaveWrite(v3, 1, save);
            WriteItemIndex(v2, save);
        }
        ++v2;
    } while (v2 < 128);
    v3[0] = 1;
    SaveMemory_SaveWrite(v3, 1, save);
}

void __cdecl G_SaveWeaponCue(SaveGame *save)
{
    EntHandle *droppedWeaponCue; // r31
    int number; // r11
    unsigned int v4; // [sp+50h] [-50h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1956, 0, "%s", "save");
    droppedWeaponCue = level.droppedWeaponCue;
    do
    {
        number = droppedWeaponCue->number;
        if (droppedWeaponCue->number && !g_entities[number - 1].r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_public.h",
                336,
                0,
                "%s\n\t(number - 1) = %i",
                "(!number || g_entities[number - 1].r.inuse)",
                number - 1);
        if (droppedWeaponCue->number)
            v4 = droppedWeaponCue->entnum() + 1;
        else
            v4 = 0;
        SaveMemory_SaveWrite(&v4, 4, save);
        ++droppedWeaponCue;
    } while ((int)droppedWeaponCue < (int)&level.changelevel);
}

void __cdecl G_LoadWeaponCue(SaveGame *save)
{
    EntHandle *droppedWeaponCue; // r30
    int number; // r11
    int v4; // r5
    bool v5; // cr58
    int v6; // [sp+50h] [-70h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1979, 0, "%s", "save");
    droppedWeaponCue = level.droppedWeaponCue;
    do
    {
        SaveMemory_LoadRead(&v6, 4, save);
        number = droppedWeaponCue->number;
        if (droppedWeaponCue->number && !g_entities[number - 1].r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_public.h",
                336,
                0,
                "%s\n\t(number - 1) = %i",
                "(!number || g_entities[number - 1].r.inuse)",
                number - 1);
        if (droppedWeaponCue->number)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
                1984,
                0,
                "%s",
                "!level.droppedWeaponCue[i].isDefined()");
        v4 = v6;
        if (v6 > MAX_GENTITIES || (v5 = v6 == 0, v6 < 0))
        {
            Com_Error(ERR_DROP, "G_LoadWeaponCue: entity out of range (%i)", v6);
            v4 = v6;
            v5 = v6 == 0;
        }
        if (!v5)
            droppedWeaponCue->setEnt(&g_entities[v4 - 1]);
        ++droppedWeaponCue;
    } while ((int)droppedWeaponCue < (int)&level.changelevel);
}

void __cdecl G_SaveDvars(SaveGame *save)
{
    MemoryFile *MemoryFile; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2001, 0, "%s", "save");
    MemoryFile = SaveMemory_GetMemoryFile(save);
    Dvar_SaveDvars(MemoryFile, 0x1000u);
}

void __cdecl G_LoadDvars(SaveGame *save)
{
    MemoryFile *MemoryFile; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2013, 0, "%s", "save");
    MemoryFile = SaveMemory_GetMemoryFile(save);
    Dvar_LoadDvars(MemoryFile);
}

void __cdecl G_CheckEntityDefaultModel(gentity_s *e)
{
    int model; // r3
    int eType; // r11
    unsigned int v4; // r3
    const char *v5; // r3

    model = e->model;
    if (model)
    {
        if (G_XModelBad(model))
        {
            eType = e->s.eType;
            if (eType == ET_ACTOR || eType == ET_ACTOR_CORPSE)
            {
                v4 = G_ModelName(e->model);
                v5 = SL_ConvertToString(v4);
                Com_PrintWarning(10, "WARNING: actor model '%s' couldn't be found! switching to default actor model.\n", v5);
                G_OverrideModel(e->model, "defaultactor");
            }
        }
    }
}

void __cdecl G_UpdateAllEntities()
{
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (ent->r.inuse && ent->r.linked)
            SV_LinkEntity(ent);
    }
}

void G_CheckAllEntities()
{
    int v0; // r30
    gentity_s *v1; // r31
    int i; // r11

    v0 = 0;
    v1 = g_entities;
    for (i = level.num_entities; v0 < i; ++v1)
    {
        if (v1->r.inuse)
        {
            G_CheckDObjUpdate(v1);
            i = level.num_entities;
        }
        ++v0;
    }
}

void __cdecl G_SaveInitState(SaveGame *save)
{
    signed int i; // r28
    const char *psz; // r29
    const char *v4; // r11
    int len; // r11
    signed int NumWeapons; // [sp+54h] [-5Ch] BYREF

    iassert(save);

    SaveMemory_StartSegment(save, 1);
    NumWeapons = BG_GetNumWeapons();
    SaveMemory_SaveWrite(&NumWeapons, 4, save);

    for (i = 1; i < NumWeapons; ++i)
    {
        psz = BG_GetWeaponDef(i)->szInternalName;

        iassert(save);
        iassert(psz);

        v4 = psz;
        while (*(unsigned __int8 *)v4++)
            ;
        len = v4 - psz - 1;

        if (len >= 256)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", len, 256);

        uint8_t length = len;
        SaveMemory_SaveWrite(&length, 1, save);
        SaveMemory_SaveWrite(psz, length, save);
    }

    G_SaveItems(save);
    G_SaveInitConfigstrings(save);
}

void __cdecl G_SaveMainState(bool savegame, SaveGame *save)
{
    MemoryFile *memFile; // r3
    unsigned int UsedSize; // r3
    unsigned int v7; // r3
    unsigned int v11; // r3
    unsigned int v13; // r3
    unsigned int v15; // r3
    unsigned int v17; // r3
    unsigned int v19; // r3
    unsigned int v21; // r3
    int v22; // r11
    gentity_s *v23; // r29
    unsigned int v25; // r3
    unsigned int v27; // r3
    int v28; // r9
    gclient_s *v29; // r29
    unsigned int v36; // r3
    unsigned int v38; // r3
    unsigned int v40; // r3
    unsigned int v42; // r3
    int v43; // r3
    const DObj_s *ServerDObj; // r3
    const DObj_s *v45; // r30
    unsigned int *v46; // r29
    int v47; // r30
    unsigned int v49; // r3
    unsigned int v51; // r3
    unsigned int v53; // r3
    unsigned int v55; // r3
    unsigned int v57; // r3
    unsigned int v59; // r3
    int i; // [sp+50h] [-4B0h] BYREF
    unsigned int v62[4]; // [sp+60h] [-4A0h] BYREF
    unsigned __int8 v63[1168]; // [sp+70h] [-490h] BYREF

    memFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("SaveMainState", UsedSize);
    v7 = MemFile_GetUsedSize(SaveMemory_GetMemoryFile(save));
    //ProfMem_Begin("level state, dvars, hudelems", v7);
    iassert(save);
    SaveMemory_StartSegment(save, 2);
    SaveMemory_SaveWrite(&level.time, 4, save);
    SaveMemory_SaveWrite(&level.framenum, 4, save);
    SaveMemory_SaveWrite(&level.framePos, 4, save);
    SaveMemory_SaveWrite(&level.soundAliasFirst, 2, save);
    SaveMemory_SaveWrite(&level.soundAliasLast, 2, save);
    SaveMemory_SaveWrite(&level.changelevel, 4, save);
    SaveMemory_SaveWrite(&level.exitTime, 4, save);
    SaveMemory_SaveWrite(&level.savepersist, 4, save);
    SaveMemory_SaveWrite(&level.bMissionSuccess, 4, save);
    SaveMemory_SaveWrite(&level.bMissionFailed, 4, save);
    SaveMemory_SaveWrite(&level.scriptPrintChannel, 4, save);
    SaveMemory_SaveWrite(g_nextMap, 64, save);
    SaveMemory_SaveWrite(level.compassMapUpperLeft, 8, save);
    SaveMemory_SaveWrite(level.compassMapWorldSize, 8, save);
    SaveMemory_SaveWrite(level.compassNorth, 8, save);

    iassert(!level.bPlayerIgnoreRadiusDamage);
    SaveMemory_SaveWrite(&level.bPlayerIgnoreRadiusDamageLatched, 4, save);
    iassert(save);

    Dvar_SaveDvars(SaveMemory_GetMemoryFile(save), 0x1000u);
    SaveMemory_SaveWrite(g_hudelems, 44032, save);
    //ProfMem_End(v11);
    //ProfMem_Begin("misc", v13);
    SaveMemory_SaveWrite(&level.fFogOpaqueDist, 4, save);
    SaveMemory_SaveWrite(&level.fFogOpaqueDistSqrd, 4, save);
    SaveMemory_SaveWrite(&level.bDrawCompassFriendlies, 4, save);
    SaveMemory_SaveWrite(&sv_gameskill->current, 4, save);
    SaveMemory_SaveWrite(&g_player_maxhealth->current, 4, save);
    Sentient_WriteGlob(save);
    AimTarget_WriteSaveGame(save);
    Scr_SavePre(1);

    //ProfMem_End(v15);
    //ProfMem_Begin("path nodes", v17);

    SaveMemory_StartSegment(save, 3);
    Path_ValidateAllNodes();
    WritePathNodes(save);

    //ProfMem_End(v19);
    //ProfMem_Begin("entities", v21);

    SaveMemory_SaveWrite(&level.num_entities, 4, save);
    v22 = 0;
    i = 0;
    do
    {
        v23 = &g_entities[v22];
        if (v23->r.inuse)
        {
            SaveMemory_SaveWrite(&i, 4, save);
            WriteEntity(v23, save);
            v22 = i;
        }
        i = ++v22;
    } while (v22 < MAX_GENTITIES);
    i = -1;
    SaveMemory_SaveWrite(&i, 4, save);
    v25 = MemFile_GetUsedSize(SaveMemory_GetMemoryFile(save));
    //ProfMem_End(v25);
    v27 = MemFile_GetUsedSize(SaveMemory_GetMemoryFile(save));
    //ProfMem_Begin("misc: clients, actors, vehicles", v27);
    WriteBadPlaces(save);
    v28 = 0;
    i = 0;
    do
    {
        v29 = &level.clients[v28];
        if (v29->pers.connected == CON_CONNECTED)
        {
            SaveMemory_SaveWrite(&i, 4, save);
            WriteClient(v29, save);
            v28 = i;
        }
        i = ++v28;
    } while (v28 < 1);
    i = -1;
    SaveMemory_SaveWrite(&i, 4, save);

    for (i = 0; i < MAX_ACTORS; ++i)
    {
        WriteActor(&level.actors[i], save);
    }
    for (i = 0; i < MAX_SENTIENTS; ++i)
    {
        WriteSentient(&level.sentients[i], save);
    }
    for (i = 0; i < MAX_VEHICLES; ++i)
    {
        WriteVehicle(&level.vehicles[i], save);
    }

    G_SaveVehicleInfo(save);

    for (i = 0; i < 32; ++i)
    {
        WriteTurretInfo(&level.turrets[i], save);
    }
    for (i = 0; i < 16; ++i)
    {
        SaveMemory_SaveWrite(&g_scr_data.actorCorpseInfo[i].entnum, 4, save);
        if (g_scr_data.actorCorpseInfo[i].entnum != -1)
            SaveMemory_SaveWrite(&g_scr_data.actorCorpseInfo[i].proneInfo, 24, save);
    }

    DynEnt_SaveEntities(SaveMemory_GetMemoryFile(save));
    memcpy(v63, &g_threatBias, sizeof(threat_bias_t));

    iassert(save);

    G_WriteStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, v63, sizeof(threat_bias_t), save);

    int listenerCount = Actor_EventListener_GetCount();
    SaveMemory_SaveWrite(&listenerCount, 4, save);
    if (listenerCount)
        SaveMemory_SaveWrite(g_AIEVlisteners, sizeof(AIEventListener) * listenerCount, save);

    iassert(level.currentTriggerListSize == 0);
    SaveMemory_SaveWrite(&level.pendingTriggerListSize, 4, save);
    SaveMemory_SaveWrite(level.pendingTriggerList, 12 * level.pendingTriggerListSize, save);
    G_SaveWeaponCue(save);
    G_SaveConfigstrings(11, 16, save);
    G_SaveConfigstrings(27, 32, save);
    G_SaveConfigstrings(59, 32, save);
    Missile_SaveAttractors(SaveMemory_GetMemoryFile(save));
    Cmd_SaveNotifications(SaveMemory_GetMemoryFile(save));
    //ProfMem_End(v36);
    //ProfMem_Begin("Script", v38);
    SaveMemory_StartSegment(save, 4);
    Scr_SavePost(SaveMemory_GetMemoryFile(save));
    //ProfMem_End(v40);
    //ProfMem_Begin("Animtree", v42);
    SaveMemory_StartSegment(save, 5);
    v43 = 0;
    i = 0;
    do
    {
        if (g_entities[v43].r.inuse)
        {
            ServerDObj = Com_GetServerDObj(v43);
            v45 = ServerDObj;
            if (ServerDObj)
            {
                XAnimSaveAnimTree(ServerDObj, SaveMemory_GetMemoryFile(save));
                DObjGetHidePartBits(v45, v62);
                v46 = v62;
                v47 = 4;
                do
                {
                    int read = *v46;
                    MemFile_WriteData(SaveMemory_GetMemoryFile(save), 4, &read);
                    --v47;
                    ++v46;
                } while (v47);
            }
            v43 = i;
        }
        i = ++v43;
    } while (v43 < MAX_GENTITIES);
    //ProfMem_End(v49);
    Scr_SaveShutdown(savegame);
    G_CheckAllEntities();
    SV_BeginSaveGame();
    //ProfMem_Begin("client", v51);
    //ProfMem_Begin("clientState", v53);
    CG_SaveEntities(save);
    CL_ArchiveClientState(SaveMemory_GetMemoryFile(save), 6);
    //ProfMem_End(v55);
    CL_ArchiveServerCommands(SaveMemory_GetMemoryFile(save));
    SV_SaveServerCommands(save);
    CG_SaveViewModelAnimTrees(save);
    Phys_ArchiveState(SaveMemory_GetMemoryFile(save));
    SV_EndSaveGame();
    //ProfMem_End(v57);
    //ProfMem_End(v59);
}

void __cdecl G_SaveState(bool savegame, SaveGame *save)
{
    CM_ValidateWorld(); // LWSS: note stubbed for now!
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2398, 0, "%s", "save");
    G_SaveInitState(save);
    G_SaveMainState(savegame, save);
}

int __cdecl G_IsSavePossible(SaveType saveType)
{
    int result; // r3

    if (saveType == SAVE_TYPE_INTERNAL)
        return 1;
    result = 0;
    if (g_entities[0].health > 0)
        return 1;
    return result;
}

int __cdecl G_WriteGame(const PendingSave *pendingSave, int checksum, SaveGame *save)
{
    MemoryFile *memFile; // r3
    unsigned int UsedSize; // r3
    MemoryFile *v8; // r3
    MemoryFile *v9; // r3
    unsigned int v10; // r3
    MemoryFile *v12; // r3
    unsigned int v13; // r3
    SaveGame *v14; // [sp+8h] [-D8h]
    char username[128]; // [sp+60h] [-80h] BYREF

    //Profile_Begin(242);
    if (pendingSave == (const PendingSave *)-64)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2432, 0, "%s", "pendingSave->description");
    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2433, 0, "%s", "pendingSave->filename");
    if (pendingSave == (const PendingSave *)-320)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2434, 0, "%s", "pendingSave->screenShotName");
    Com_Printf(10, "G_WriteGame '%s' '%s'\n", pendingSave->filename, pendingSave->description);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2438, 0, "%s", "save");
    SaveMemory_InitializeGameSave(save);
    memFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("Game Save", UsedSize);
    v8 = SaveMemory_GetMemoryFile(save);
    Scr_SaveSource(v8);
    G_SaveState(1, save);
    SaveMemory_StartSegment(save, -1);
    if (SaveMemory_IsSuccessful(save) && BuildCleanSavePath(username, 0x40u, pendingSave->filename, pendingSave->saveType))
    {
        SaveMemory_CreateHeader(
            username,
            pendingSave->description,
            pendingSave->screenShotName,
            checksum,
            0,
            pendingSave->suppressPlayerNotify,
            pendingSave->saveType,
            pendingSave->saveId,
            save);
        SaveMemory_FinalizeSave(save);
        //Profile_EndInternal(0);
        v12 = SaveMemory_GetMemoryFile(save);
        v13 = MemFile_GetUsedSize(v12);
        //ProfMem_End(v13);
        //ProfMem_PrintTree();
        return 1;
    }
    else
    {
        SaveMemory_RollbackSave(save);
        //Profile_EndInternal(0);
        v9 = SaveMemory_GetMemoryFile(save);
        v10 = MemFile_GetUsedSize(v9);
        //ProfMem_End(v10);
        return 0;
        //ProfMem_PrintTree();
    }
}

void __cdecl G_WriteCurrentCommitToDevice()
{
    SaveGame *LastCommittedSave; // r3
    SaveGame *v1; // r31

    LastCommittedSave = SaveMemory_GetLastCommittedSave();
    v1 = LastCommittedSave;
    if (LastCommittedSave)
    {
        SaveMemory_GetHeader(LastCommittedSave);
        if (SaveMemory_WriteSaveToDevice(v1))
            SV_DisplaySaveErrorUI();
    }
}

void __cdecl G_PrepareSaveMemoryForWrite(char commitLevel)
{
    SaveGame *SaveHandle; // r30
    bool v3; // r31

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2509, 0, "%s", "save");
    v3 = g_entities[0].health <= 0;
    //Memcard_CheckOngoingTasks();
    if (SaveMemory_IsWaitingForCommit(SaveHandle) && !v3 && (commitLevel & 6) != 0)
        SaveMemory_ForceCommitSave(SaveHandle);
    if (SaveMemory_IsCurrentCommittedSaveValid() && (commitLevel & 4) != 0 && !SaveMemory_IsWrittenToDevice(SaveHandle))
        G_WriteCurrentCommitToDevice();
}

int __cdecl G_ProcessCommitActions(const PendingSave *pendingSave, SaveGame *save)
{
    int v4; // r31

    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2543, 0, "%s", "pendingSave");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2544, 0, "%s", "save");
    v4 = 0;
    if ((pendingSave->commitLevel & 2) != 0)
        SaveMemory_ForceCommitSave(save);
    if ((pendingSave->commitLevel & 4) != 0)
    {
        //if (pendingSave->saveType != SAVE_TYPE_AUTOSAVE)
        //    MemCard_SetUseDevDrive(1);
        v4 = SaveMemory_WriteSaveToDevice(save);
        //if (pendingSave->saveType != SAVE_TYPE_AUTOSAVE)
        //    MemCard_SetUseDevDrive(0);
    }
    return v4;
}

int __cdecl G_SaveGame(const PendingSave *pendingSave, int checksum)
{
    char v4; // r11
    SaveGame *SaveHandle; // r31

    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2581, 0, "%s", "pendingSave");
    if (pendingSave->saveType == SAVE_TYPE_INTERNAL || (v4 = 0, g_entities[0].health > 0))
        v4 = 1;
    if (!v4)
        return 0;
    G_PrepareSaveMemoryForWrite(pendingSave->commitLevel);
    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2588, 0, "%s", "save");
    if ((unsigned __int8)G_WriteGame(pendingSave, checksum, SaveHandle))
        return G_ProcessCommitActions(pendingSave, SaveHandle);
    else
        return 0;
}

bool __cdecl G_CommitSavedGame(int saveId)
{
    SaveGame *SaveHandle; // r31

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2603, 0, "%s", "save");
    return SaveMemory_CommitSave(SaveHandle, saveId);
}

void __cdecl G_LoadItems(SaveGame *save)
{
    _BYTE v2[16]; // [sp+50h] [-20h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2627, 0, "%s", "save");
    SaveMemory_LoadRead(v2, 1, save);
    while (!v2[0])
    {
        ReadItemIndex(save);
        SaveMemory_LoadRead(v2, 1, save);
    }
}

void __cdecl G_SetPendingLoadName(const char *filename)
{
    I_strncpyz(g_pendingLoadName, filename, 64);
}

void __cdecl G_PreLoadGame(int checksum, int *useLoadedSourceFiles, SaveGame **save)
{
    void *LoadFromDevice; // r28
    const SaveHeader *Header; // r26
    MemoryFile *memFile; // r3
    MemoryFile *v9; // r3
    MemoryFile *v10; // r3
    __int64 v11; // r10
    __int64 v12; // r8
    __int64 v13; // r6
    int v14; // [sp+8h] [-C8h]
    int v15; // [sp+Ch] [-C4h]
    int v16; // [sp+10h] [-C0h]
    int v17; // [sp+14h] [-BCh]
    int v18; // [sp+18h] [-B8h]
    int v19; // [sp+1Ch] [-B4h]
    int v20; // [sp+20h] [-B0h]
    int v21; // [sp+24h] [-ACh]
    unsigned int v22[24]; // [sp+70h] [-60h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2660, 0, "%s", "save");
    //Profile_Begin(244);
    //MemCard_SetUseDevDrive(g_useDevSaveArea);
    if (SaveMemory_IsCommittedSaveAvailable(g_pendingLoadName, checksum))
    {
        LoadFromDevice = 0;
        *save = SaveMemory_GetLastCommittedSave();
    }
    else
    {
        LoadFromDevice = SaveMemory_ReadLoadFromDevice(g_pendingLoadName, checksum, *useLoadedSourceFiles, save);
    }
    Header = SaveMemory_GetHeader(*save);
    if (!Header->demoPlayback)
        SV_SetLastSaveName(g_pendingLoadName);
    SaveMemory_InitializeLoad(*save, Header->bodySize);
    if (Header->demoPlayback)
    {
        memFile = SaveMemory_GetMemoryFile(*save);
        Dvar_LoadDvars(memFile);
    }
    if (*useLoadedSourceFiles
        && (!LoadFromDevice
            || (Scr_GetChecksum(v22), v22[0] == Header->scrCheckSum[0])
            && v22[1] == Header->scrCheckSum[1]
            && v22[2] == Header->scrCheckSum[2]))
    {
        v10 = SaveMemory_GetMemoryFile(*save);
        Scr_SkipSource(v10, LoadFromDevice);
    }
    else
    {
        if (!LoadFromDevice)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2704, 0, "%s", "fileHandle");
        if (*useLoadedSourceFiles)
        {
            *useLoadedSourceFiles = 0;
            G_ClearLowHunk();
        }
        v9 = SaveMemory_GetMemoryFile(*save);
        Scr_LoadSource(v9, LoadFromDevice);
    }
    SaveMemory_MoveToSegment(*save, -1);
    if (!SaveMemory_IsSuccessful(*save))
    {
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, "The save file has become corrupted.");
    }
    if (Header->demoPlayback)
    {
        if (!LoadFromDevice)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2725, 0, "%s", "fileHandle");
        SV_LoadDemo(*save, LoadFromDevice);
    }
    if (LoadFromDevice)
        CloseDevice(LoadFromDevice);
    //MemCard_SetUseDevDrive(0);
    //Profile_EndInternal(0);
}

int __cdecl G_LoadWeapons(SaveGame *save)
{
    int v2; // r29
    int v3; // r31
    _BYTE v5[4]; // [sp+50h] [-160h] BYREF
    int v6[3]; // [sp+54h] [-15Ch] BYREF
    char v7[336]; // [sp+60h] [-150h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2751, 0, "%s", "save");
    SaveMemory_LoadRead(v6, 4, save);
    v2 = 1;
    if (v6[0] <= 1)
        return 1;
    while (1)
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v5, 1, save);
        v3 = v5[0];
        SaveMemory_LoadRead(v7, v5[0], save);
        v7[v3] = 0;
        if (BG_GetWeaponIndexForName(v7, G_RegisterWeapon) != v2)
            break;
        if (++v2 >= v6[0])
            return 1;
    }
    Com_Printf(10, "Weapon index mismatch for '%s'\n", v7);
    return 0;
}

void __cdecl G_InitLoadGame(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2774, 0, "%s", "save");
    G_LoadInitConfigstrings(save);
}

void __cdecl G_LoadMainState(SaveGame *save)
{
    MemoryFile *memFile; // r21
    int i; // r5
    gentity_s *lastFreeEnt; // r10
    int v6; // r11
    int num_entities; // r9
    int v8; // r8
    int k; // r11
    gclient_s *cl; // r29
    TurretInfo *v15; // r29
    int v17; // r11
    gentity_s *v18; // r29
    int eType; // r11
    unsigned int v20; // r3
    const char *v21; // r3
    DObj_s *ServerDObj; // r3
    DObj_s *v23; // r27
    unsigned int *v24; // r28
    int v25; // r29
    int j; // [sp+50h] [-90h] BYREF
    unsigned int v28[32]; // [sp+60h] [-80h] BYREF

    iassert(save);

    G_FreeEntities();
    HudElem_DestroyAll();
    G_FreePathnodesScriptInfo();
    G_FreeVehiclePathsScriptInfo();
    Actor_ClearThreatBiasGroups();
    Cmd_UnregisterAllNotifications();
    Scr_ShutdownSystem(1, 1);
    Path_ValidateAllNodes();
    memFile = SaveMemory_GetMemoryFile(save);

    level.initializing = 1;

    SaveMemory_MoveToSegment(save, 2);
    SaveMemory_LoadRead(&level.time, 4, save);
    SaveMemory_LoadRead(&level.framenum, 4, save);
    SaveMemory_LoadRead(&level.framePos, 4, save);
    SaveMemory_LoadRead(&level.soundAliasFirst, 2, save);
    SaveMemory_LoadRead(&level.soundAliasLast, 2, save);
    SaveMemory_LoadRead(&level.changelevel, 4, save);
    SaveMemory_LoadRead(&level.exitTime, 4, save);
    SaveMemory_LoadRead(&level.savepersist, 4, save);
    SaveMemory_LoadRead(&level.bMissionSuccess, 4, save);
    SaveMemory_LoadRead(&level.bMissionFailed, 4, save);
    SaveMemory_LoadRead(&level.scriptPrintChannel, 4, save);
    SaveMemory_LoadRead(g_nextMap, 64, save);
    SaveMemory_LoadRead(level.compassMapUpperLeft, 8, save);
    SaveMemory_LoadRead(level.compassMapWorldSize, 8, save);
    SaveMemory_LoadRead(level.compassNorth, 8, save);

    iassert(!level.bPlayerIgnoreRadiusDamage);
    SaveMemory_LoadRead(&level.bPlayerIgnoreRadiusDamageLatched, 4, save);
    iassert(save);

    Dvar_LoadDvars(SaveMemory_GetMemoryFile(save));
    SaveMemory_LoadRead(g_hudelems, /*44032*/ sizeof(game_hudelem_s) * 256, save); // KISAKTODO: not the right array size
    SaveMemory_LoadRead(&level.fFogOpaqueDist, 4, save);
    SaveMemory_LoadRead(&level.fFogOpaqueDistSqrd, 4, save);
    SaveMemory_LoadRead(&level.bDrawCompassFriendlies, 4, save);
    SaveMemory_LoadRead(&j, 4, save);
    Dvar_SetInt(sv_gameskill, j);
    SaveMemory_LoadRead(&j, 4, save);
    Dvar_SetInt(g_player_maxhealth, j);
    Sentient_ReadGlob(save);
    AimTarget_ReadSaveGame(save);

    SaveMemory_MoveToSegment(save, 4);
    Scr_LoadPre(1, memFile);

    SaveMemory_MoveToSegment(save, 3);
    ReadPathNodes(save);

    iassert(!level.num_entities);
    SaveMemory_LoadRead(&level.num_entities, 4, save);

    SaveMemory_LoadRead(&j, 4, save);

    for (i = j; j >= 0; i = j)
    {
        if (i >= MAX_GENTITIES)
        {
            Com_Error(ERR_DROP, "G_LoadMainState: entitynum out of range (%i, MAX = %i)", i, MAX_GENTITIES);
            i = j;
        }
        ReadEntity(&g_entities[i], save);
        SaveMemory_LoadRead(&j, 4, save);
    }

    ReadBadPlaces(save);
    lastFreeEnt = 0;
    level.firstFreeEnt = 0;
    v6 = 1;
    level.lastFreeEnt = 0;
    num_entities = level.num_entities;
    for (j = 1; v6 < num_entities; j = v6)
    {
        v8 = v6;
        if (!g_entities[v6].r.inuse)
        {
            if (lastFreeEnt)
            {
                lastFreeEnt->nextFree = &g_entities[v8];
                v6 = j;
            }
            else
            {
                level.firstFreeEnt = &g_entities[v8];
            }
            level.lastFreeEnt = &g_entities[v6];
            level.lastFreeEnt->nextFree = 0;
            v6 = j;
            lastFreeEnt = level.lastFreeEnt;
            num_entities = level.num_entities;
        }
        ++v6;
    }

    Path_ValidateAllNodes();
    SaveMemory_LoadRead(&j, 4, save);

    for (k = j; j >= 0; k = j)
    {
        if (k > 1)
        {
            Com_Error(ERR_DROP, "G_LoadMainState: clientnum out of range");
            k = j;
        }
        cl = &level.clients[k];
        if (cl->pers.connected == CON_DISCONNECTED)
            Com_Error(ERR_DROP, "G_LoadMainState: client mis-match in savegame");
        ReadClient(cl, save);
        SaveMemory_LoadRead(&j, 4, save);
    }

    for (j = 0; j < MAX_ACTORS; ++j)
    {
        ReadActor(&level.actors[j], save);
    }
    for (j = 0; j < MAX_SENTIENTS; ++j)
    {
        ReadSentient(&level.sentients[j], save);
    }
    for (j = 0; j < MAX_VEHICLES; ++j)
    {
        ReadVehicle(&level.vehicles[j], save);
    }

    G_LoadVehicleInfo(save);

    for (j = 0; j < 32; ++j)
    {
        int read = 0;
        v15 = &level.turrets[j];
        iassert(save);
        SaveMemory_LoadRead(&read, 4, save);
        if (read)
            G_ReadStruct(turretFields, (unsigned __int8 *)v15, sizeof(TurretInfo), save);
    }

    for (j = 0; j < 16; ++j)
    {
        SaveMemory_LoadRead(&g_scr_data.actorCorpseInfo[j].entnum, 4, save);
        if (g_scr_data.actorCorpseInfo[j].entnum != -1)
            SaveMemory_LoadRead(&g_scr_data.actorCorpseInfo[j].proneInfo, 24, save);
    }

    level.actorCorpseCount = 16;

    DynEnt_LoadEntities(SaveMemory_GetMemoryFile(save));

    G_ReadStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, sizeof(threat_bias_t), save);

    iassert(Actor_EventListener_GetCount() == 0);
    int listenerCount;
    SaveMemory_LoadRead(&listenerCount, 4, save);
    Actor_EventListener_SetCount(listenerCount);
    if (listenerCount)
        SaveMemory_LoadRead(g_AIEVlisteners, sizeof(AIEventListener) * listenerCount, save);

    iassert(level.currentTriggerListSize == 0);
    SaveMemory_LoadRead(&level.pendingTriggerListSize, 4, save);
    SaveMemory_LoadRead(level.pendingTriggerList, 12 * level.pendingTriggerListSize, save);
    G_LoadWeaponCue(save);
    G_LoadConfigstrings(11, 16, save);
    G_LoadConfigstrings(27, 32, save);
    G_LoadConfigstrings(59, 32, save);
    G_LoadTargets();
    Missile_LoadAttractors(memFile);
    Cmd_LoadNotifications(memFile);
    SaveMemory_MoveToSegment(save, 5);

    v17 = 0;
    j = 0;
    do
    {
        v18 = &g_entities[v17];
        if (v18->r.inuse)
        {
            if (v18->model)
            {
                if (G_XModelBad(v18->model))
                {
                    eType = v18->s.eType;
                    if (eType == 14 || eType == 16)
                    {
                        v20 = G_ModelName(v18->model);
                        v21 = SL_ConvertToString(v20);
                        Com_PrintWarning(
                            10,
                            "WARNING: actor model '%s' couldn't be found! switching to default actor model.\n",
                            v21);
                        G_OverrideModel(v18->model, "defaultactor");
                    }
                }
            }
            G_DObjUpdate(v18);
            ServerDObj = Com_GetServerDObj(j);
            v23 = ServerDObj;
            if (ServerDObj)
            {
                XAnimLoadAnimTree(ServerDObj, memFile);
                v24 = v28;
                v25 = 4;
                do
                {
                    int read;
                    MemFile_ReadData(memFile, 4, (unsigned char*)&read);
                    --v25;
                    *v24++ = read;
                } while (v25);
                DObjSetHidePartBits(v23, v28);
            }
            v17 = j;
        }
        j = ++v17;
    } while (v17 < MAX_GENTITIES);
    SV_SendGameState();
    CG_LoadEntities(save);
    CL_ArchiveClientState(memFile, 6);
    CL_LoadServerCommands(save);
    SV_LoadServerCommands(save);
    CG_LoadViewModelAnimTrees(save, &level.clients->ps);
    Phys_ArchiveState(memFile);
    SaveMemory_MoveToSegment(save, -1);
    Scr_LoadShutdown();
    SV_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_s), &level.clients->ps, sizeof(gclient_s)); // KISAKTODO: pointer type on ps
    level.initializing = 0;
}

void __cdecl G_LoadGame(int /*checksum*/, SaveGame *save)
{
    const SaveHeader *header; // r31
    __int64 v4; // r10
    __int64 v5; // r8
    __int64 v6; // r6
    unsigned int RandomSeed; // [sp+70h] [-40h] BYREF
    unsigned int checksums[3]; // [sp+78h] [-38h] BYREF

    Com_Printf(10, "=== G_LoadGame ===\n");
    //Profile_Begin(244);
    R_Cinematic_UnsetNextPlayback();
    iassert(save);
    header = SaveMemory_GetHeader(save);
    iassert(header);

    if (header->isUsingScriptChecksum)
    {
        Scr_GetChecksum(checksums);
        if (checksums[0] != header->scrCheckSum[0] || checksums[1] != header->scrCheckSum[1] || checksums[2] != header->scrCheckSum[2])
            Com_Error(ERR_DROP, "G_LoadGame: savegame '%s' was saved with different script files %s", header->filename, header->buildNumber);
    }
    if (header->saveCheckSum != SaveMemory_CalculateChecksum(save))
    {
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, "The save file has become corrupted.");
    }
    G_LoadMainState(save);
    SaveMemory_FinalizeLoad(save);
    RandomSeed = G_GetRandomSeed();
    level.demoplaying = SV_InitDemo((int *)&RandomSeed);
    G_srand(RandomSeed);
    if (!SV_UsingDemoSave())
    {
        SV_GetUsercmd(0, &level.clients->pers.cmd);
        InitClientDeltaAngles(level.clients);
    }
    G_PruneLoadedCorpses();
    //Profile_EndInternal(0);
}

int __cdecl G_LoadErrorCleanup()
{
    SaveGame *SaveHandle; // r31
    SaveGame *v1; // r30

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    v1 = SaveMemory_GetSaveHandle(SAVE_DEMO_HANDLE);
    if (!SaveMemory_IsLoading(SaveHandle) && !SaveMemory_IsLoading(v1))
        return 0;
    Com_InitDObj();
    SaveMemory_CleanupSaveMemory();
    return 1;
}

