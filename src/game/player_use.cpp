#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "player_use.h"
#include "g_local.h"
#include <script/scr_const.h>
#include "g_main.h"
#include "sentient.h"
#include <script/scr_vm.h>
#include <server/sv_game.h>
#include <server/sv_public.h>
#include "actor_grenade.h"
#include "turret.h"
#include <aim_assist/aim_target.h>
#include "actor_events.h"
#include "bullet.h"


void __cdecl Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt)
{
    int eType; // r11
    int handler; // r10
    void(__cdecl * touch)(gentity_s *, gentity_s *, gentity_s *); // r11
    gentity_s *v7; // r5

    if (!playerEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 48, 0, "%s", "playerEnt");
    if (!playerEnt->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 49, 0, "%s", "playerEnt->client");
    if (!useEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 50, 0, "%s", "useEnt");
    if (!useEnt->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 52, 0, "%s", "useEnt->r.inuse");
    eType = useEnt->s.eType;
    if (eType == 2)
    {
        Scr_AddEntity(playerEnt);
        Scr_Notify(useEnt, scr_const.touch, 1u);
        handler = useEnt->handler;
        useEnt->active = 1;
        touch = (void(__cdecl *)(gentity_s *, gentity_s *, gentity_s *))entityHandlers[handler].touch;
        if (touch)
        {
            v7 = 0;
        LABEL_16:
            touch(useEnt, playerEnt, v7);
        }
    }
    else
    {
        if (eType == 14)
            Sentient_StealClaimNode(playerEnt->sentient, useEnt->sentient);
        Scr_AddEntity(playerEnt);
        Scr_Notify(useEnt, scr_const.trigger, 1u);
        touch = entityHandlers[useEnt->handler].use;
        if (touch)
        {
            v7 = playerEnt;
            goto LABEL_16;
        }
    }
    EntHandle::setEnt(&playerEnt->client->useHoldEntity, 0);
}

int __cdecl Player_ActivateCmd(gentity_s *ent)
{
    int result; // r3
    gentity_s *v3; // r4
    gclient_s *client; // r11
    int pm_flags; // r10
    int weaponstate; // r10
    int cursorHintEntIndex; // r11

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 90, 0, "%s", "ent");
    if (!ent->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 91, 0, "%s", "ent->r.inuse");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 92, 0, "%s", "ent->client");
    if (!Scr_IsSystemActive())
        return 0;
    EntHandle::setEnt(&ent->client->useHoldEntity, 0);
    v3 = G_IsVehicleUnusable(ent);
    if (v3)
    {
        Player_UseEntity(ent, v3);
        return 1;
    }
    client = ent->client;
    if (ent->active)
    {
        if ((client->ps.weapFlags & 0x800) == 0)
        {
            result = 1;
            ent->active = (client->ps.eFlags & 0x300) == 0 ? 0 : 2;
            return result;
        }
    }
    else
    {
        pm_flags = client->ps.pm_flags;
        if ((pm_flags & 4) == 0 && (pm_flags & 0x8000) == 0)
        {
            weaponstate = client->ps.weaponstate;
            if (weaponstate < 15 || weaponstate > 20)
            {
                cursorHintEntIndex = client->ps.cursorHintEntIndex;
                if (cursorHintEntIndex == 2175)
                    return 0;
                if (!g_entities[cursorHintEntIndex].r.inuse)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                        134,
                        0,
                        "%s",
                        "g_entities[ ent->client->ps.cursorHintEntIndex ].r.inuse");
                EntHandle::setEnt(&ent->client->useHoldEntity, &g_entities[ent->client->ps.cursorHintEntIndex]);
                ent->client->useHoldTime = level.time;
            }
        }
    }
    return 1;
}

void __cdecl Player_ActivateHoldCmd(gentity_s *ent)
{
    gclient_s *client; // r11
    gentity_s *v3; // r30
    unsigned int v4; // r3

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 150, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 151, 0, "%s", "ent->client");
    if (Scr_IsSystemActive())
    {
        if (EntHandle::isDefined(&ent->client->useHoldEntity))
        {
            client = ent->client;
            if (level.time - client->useHoldTime >= g_useholdtime->current.integer)
            {
                v3 = EntHandle::ent(&client->useHoldEntity);
                if (v3->s.number != EntHandle::entnum(&ent->client->useHoldEntity))
                {
                    v4 = EntHandle::entnum(&ent->client->useHoldEntity);
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                        164,
                        0,
                        "useEnt->s.number == ent->client->useHoldEntity.entnum()\n\t%i, %i",
                        v3->s.number,
                        v4);
                }
                if (!v3->r.inuse)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                        165,
                        0,
                        "%s\n\t(useEnt->s.number) = %i",
                        "(useEnt->r.inuse)",
                        v3->s.number);
                Player_UseEntity(ent, v3);
            }
        }
    }
}

void __cdecl Player_UpdateActivate(gentity_s *ent)
{
    char v2; // r28
    gclient_s *client; // r11

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 178, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 179, 0, "%s", "ent->client");
    v2 = 0;
    ent->client->ps.weapFlags &= ~1u;
    if (EntHandle::isDefined(&ent->client->useHoldEntity))
    {
        client = ent->client;
        if ((client->oldbuttons & 0x20) != 0 && (client->buttons & 0x20) == 0)
        {
        LABEL_13:
            ent->client->ps.weapFlags |= 1u;
            return;
        }
    }
    if ((ent->client->latched_buttons & 0x28) != 0)
        v2 = Player_ActivateCmd(ent);
    if (!EntHandle::isDefined(&ent->client->useHoldEntity) && !v2)
    {
        if ((ent->client->latched_buttons & 0x20) == 0)
            return;
        goto LABEL_13;
    }
    if ((ent->client->buttons & 0x28) != 0)
        Player_ActivateHoldCmd(ent);
    ent->client->useButtonDone = 1;
}

int __cdecl compare_use(float *pe1, float *pe2)
{
    return (int)(float)(pe1[1] - pe2[1]);
}

int __cdecl Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex)
{
    int v6; // r22
    gclient_s *client; // r29
    int v10; // r3
    unsigned int v11; // r27
    useList_t *v12; // r30
    int *v13; // r23
    int i; // r18
    gentity_s *v15; // r31
    int eType; // r10
    actor_s *actor; // r11
    double value; // fp31
    int v19; // r11
    double v20; // fp11
    double v21; // fp0
    double v22; // fp12
    double v23; // fp12
    double v24; // fp31
    bool v26; // mr_fpscr49
    double v28; // fp11
    double v29; // fp12
    double v30; // fp0
    unsigned int v31; // r30
    int v32; // r27
    bool v33; // zf
    float *p_score; // r31
    unsigned int v35; // r28
    const gentity_s *v36; // r3
    double v37; // fp0
    float v39; // [sp+50h] [-2340h] BYREF
    float v40; // [sp+54h] [-233Ch]
    float v41; // [sp+58h] [-2338h]
    float v42; // [sp+60h] [-2330h] BYREF
    float v43; // [sp+64h] [-232Ch]
    float v44; // [sp+68h] [-2328h]
    float v45; // [sp+70h] [-2320h] BYREF
    float v46; // [sp+74h] [-231Ch]
    float v47; // [sp+78h] [-2318h]
    float v48; // [sp+80h] [-2310h] BYREF
    float v49; // [sp+84h] [-230Ch]
    float v50; // [sp+88h] [-2308h]
    float v51[4]; // [sp+90h] [-2300h] BYREF
    float v52; // [sp+A0h] [-22F0h] BYREF
    float v53; // [sp+A4h] [-22ECh]
    float v54; // [sp+A8h] [-22E8h]
    float v55[4]; // [sp+B0h] [-22E0h] BYREF
    int v56[180]; // [sp+C0h] [-22D0h] BYREF

    v6 = 0;
    client = ent->client;
    _FP12 = (float)((float)192.0 - player_throwbackOuterRadius->current.value);
    __asm { fsel      f31, f12, f13, f0 }
    G_GetPlayerViewOrigin(&client->ps, &v42);
    G_GetPlayerViewDirection(ent, &v52, 0, 0);
    v45 = client->ps.origin[0] + (float)-15.0;
    v46 = client->ps.origin[1] + (float)-15.0;
    v47 = client->ps.origin[2] + (float)0.0;
    v48 = client->ps.origin[0] + (float)15.0;
    v49 = client->ps.origin[1] + (float)15.0;
    v50 = client->ps.origin[2] + (float)70.0;
    v55[0] = v42 - (float)_FP31;
    v55[1] = v43 - (float)_FP31;
    v51[0] = v42 + (float)_FP31;
    v55[2] = v44 - (float)96.0;
    v51[1] = v43 + (float)_FP31;
    v51[2] = v44 + (float)96.0;
    v10 = CM_AreaEntities(v55, v51, v56, 2176, 2113536);
    v11 = 0;
    if (v10 > 0)
    {
        v12 = useList;
        v13 = v56;
        for (i = v10; i; --i)
        {
            v15 = &g_entities[*v13];
            if (ent == v15)
                goto LABEL_41;
            eType = v15->s.eType;
            if (eType != 2 && (v15->r.contents & 0x200000) == 0)
            {
                actor = v15->actor;
                if (!actor || !actor->useable)
                    goto LABEL_41;
            }
            if (v15->classname == scr_const.trigger_use_touch)
            {
                if (v15->r.absmin[0] > (double)v48
                    || v15->r.absmax[0] < (double)v45
                    || v15->r.absmin[1] > (double)v49
                    || v15->r.absmax[1] < (double)v46
                    || v15->r.absmin[2] > (double)v50
                    || v15->r.absmax[2] < (double)v47
                    || !SV_EntityContact(&v45, &v48, v15))
                {
                    goto LABEL_41;
                }
                v12->score = -144.0;
            }
            else
            {
                if (eType == 3)
                {
                    if (prevHintEntIndex != v15->s.number)
                    {
                        value = player_throwbackInnerRadius->current.value;
                        if (Vec2DistanceSq(v15->r.currentOrigin, ent->r.currentOrigin) > (double)(float)((float)value * (float)value))
                            goto LABEL_41;
                    }
                    if ((float)((float)(v15->s.lerp.pos.trDelta[2] * v15->s.lerp.pos.trDelta[2])
                        + (float)((float)(v15->s.lerp.pos.trDelta[0] * v15->s.lerp.pos.trDelta[0])
                            + (float)(v15->s.lerp.pos.trDelta[1] * v15->s.lerp.pos.trDelta[1]))) > (double)(float)(bg_maxGrenadeIndicatorSpeed->current.value * bg_maxGrenadeIndicatorSpeed->current.value))
                        goto LABEL_41;
                }
                v19 = v15->s.eType;
                if (v19 == 3)
                    v20 = player_throwbackOuterRadius->current.value;
                else
                    v20 = v19 == 11 ? 128.0 : 72.0;
                v21 = (float)(v15->r.absmin[2] + v15->r.absmax[2]);
                v40 = (float)(v15->r.absmin[1] + v15->r.absmax[1]) * (float)0.5;
                v22 = (float)((float)(v15->r.absmin[0] + v15->r.absmax[0]) * (float)0.5);
                v39 = (float)(v15->r.absmin[0] + v15->r.absmax[0]) * (float)0.5;
                v41 = (float)v21 * (float)0.5;
                v23 = (float)((float)v22 - v42);
                v24 = __fsqrts((float)((float)((float)v23 * (float)v23)
                    + (float)((float)((float)(v41 - v44) * (float)(v41 - v44))
                        + (float)((float)(v40 - v43) * (float)(v40 - v43)))));
                _FP10 = -v24;
                v26 = v24 > v20;
                __asm { fsel      f11, f10, f29, f31 }
                v28 = (float)((float)1.0 / (float)_FP11);
                v29 = (float)((float)v28 * (float)v23);
                if (v26
                    || v15->classname == scr_const.trigger_use
                    && v15->trigger.requireLookAt
                    && (float)((float)(v52 * (float)v29)
                        + (float)((float)(v54 * (float)((float)(v41 - v44) * (float)v28))
                            + (float)(v53 * (float)((float)(v40 - v43) * (float)v28)))) < 0.75999999)
                {
                    goto LABEL_41;
                }
                v30 = (float)((float)-(float)((float)((float)((float)((float)(v52 * (float)v29)
                    + (float)((float)(v54
                        * (float)((float)(v41 - v44) * (float)v28))
                        + (float)(v53
                            * (float)((float)(v40 - v43) * (float)v28))))
                    + (float)1.0)
                    * (float)0.5)
                    - (float)1.0)
                    * (float)256.0);
                v12->score = (float)-(float)((float)((float)((float)((float)(v52 * (float)v29)
                    + (float)((float)(v54
                        * (float)((float)(v41 - v44) * (float)v28))
                        + (float)(v53
                            * (float)((float)(v40 - v43) * (float)v28))))
                    + (float)1.0)
                    * (float)0.5)
                    - (float)1.0)
                    * (float)256.0;
                if (v15->s.eType == 3)
                    v12->score = (float)v30 - (float)512.0;
                if (v15->classname == scr_const.trigger_use)
                    v12->score = v12->score - (float)256.0;
                if (v15->s.eType == 10)
                    v12->score = v12->score - (float)128.0;
                if (v15->s.eType == 2 && !BG_CanItemBeGrabbed(&v15->s, &ent->client->ps, 0))
                {
                    ++v6;
                    v12->score = v12->score + (float)10000.0;
                }
                v12->score = v12->score + (float)v24;
            }
            v12->ent = v15;
            ++v11;
            ++v12;
        LABEL_41:
            ++v13;
        }
    }
    qsort(useList, v11, 8u, (int(__cdecl *)(const void *, const void *))compare_use);
    v31 = v11 - v6;
    v33 = (int)(v11 - v6) <= 0;
    v32 = 0;
    if (!v33)
    {
        p_score = &useList->score;
        v35 = v31;
        do
        {
            v36 = (const gentity_s *)*((unsigned int *)p_score - 1);
            if (v36->classname != scr_const.trigger_use_touch)
            {
                v39 = v36->r.absmin[0] + v36->r.absmax[0];
                v40 = v36->r.absmin[1] + v36->r.absmax[1];
                v37 = (float)(v36->r.absmin[2] + v36->r.absmax[2]);
                v39 = v39 * (float)0.5;
                v40 = v40 * (float)0.5;
                v41 = (float)v37 * (float)0.5;
                if (v36->s.eType == 10)
                    G_DObjGetWorldTagPos_CheckTagExists(v36, scr_const.tag_aim, &v39);
                if (!G_TraceCapsuleComplete(&v42, vec3_origin, vec3_origin, &v39, client->ps.clientNum, 17))
                {
                    ++v32;
                    *p_score = *p_score + (float)10000.0;
                }
            }
            --v35;
            p_score += 2;
        } while (v35);
    }
    qsort(useList, v31, 8u, (int(__cdecl *)(const void *, const void *))compare_use);
    return v31 - v32;
}

void __cdecl G_UpdateFriendlyOverlay(gentity_s *ent)
{
    gentity_s *v2; // r30
    actor_s *actor; // r11
    const char *v4; // r3
    const char *v5; // r3
    const char *v6; // r3
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3
    const char *v9; // r4
    int v10; // r3
    scr_vehicle_s *scr_vehicle; // r11
    const char *v12; // r3
    const char *v13; // r3
    unsigned int lookAtText1; // r3
    const char *v15; // r3
    const char *v16; // r3

    if (!EntHandle::isDefined(&ent->client->pLookatEnt))
        goto LABEL_18;
    v2 = EntHandle::ent(&ent->client->pLookatEnt);
    actor = v2->actor;
    if (!actor || !actor->properName)
    {
        if (v2->s.eType == 11)
        {
            if (!v2->scr_vehicle)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 466, 0, "%s", "traceEnt->scr_vehicle");
            scr_vehicle = v2->scr_vehicle;
            if (scr_vehicle->lookAtText0)
            {
                v12 = SL_ConvertToString(scr_vehicle->lookAtText0);
                v13 = va("%s", v12);
                SV_SetConfigstring(9, v13);
                lookAtText1 = v2->scr_vehicle->lookAtText1;
                goto LABEL_12;
            }
            v9 = "none";
        LABEL_19:
            v10 = 9;
            goto LABEL_20;
        }
        if (v2->lookAtText0)
        {
            v15 = SL_ConvertToString(v2->lookAtText0);
            v16 = va("%s", v15);
            SV_SetConfigstring(9, v16);
            lookAtText1 = v2->lookAtText1;
        LABEL_12:
            if (lookAtText1)
            {
                v9 = SL_ConvertToString(lookAtText1);
                v10 = 10;
            }
            else
            {
                v10 = 10;
                v9 = "none";
            }
            goto LABEL_20;
        }
    LABEL_18:
        v9 = va("%s", "none");
        goto LABEL_19;
    }
    v4 = SL_ConvertToString(actor->properName);
    v5 = va("%s", v4);
    SV_SetConfigstring(9, v5);
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 455, 0, "%s", "ent->client");
    v6 = SL_ConvertToString(v2->actor->weaponName);
    WeaponIndexForName = G_GetWeaponIndexForName(v6);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    v9 = va("%s", WeaponDef->szOverlayName);
    v10 = 10;
LABEL_20:
    SV_SetConfigstring(v10, v9);
}

int __cdecl Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt)
{
    int v4; // r31
    unsigned int v5; // r30
    WeaponDef *WeaponDef; // r31
    WeaponDef *v7; // r3
    weapType_t weapType; // r11
    WeaponDef *v9; // r31
    int result; // r3
    weapInventoryType_t inventoryType; // r11
    bool v12; // zf

    if (!traceEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 508, 0, "%s", "traceEnt");
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 509, 0, "%s", "client");
    v4 = *(unsigned __int16 *)traceEnt->s.index;
    if (v4 >= 2048)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            513,
            0,
            "%s\n\t(index) = %i",
            "(index < (128 * NUM_WEAP_ALTMODELS ))",
            v4);
    if (bg_itemlist[v4].giType != IT_WEAPON)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 516, 0, "%s", "item->giType == IT_WEAPON");
    v5 = v4 % 128;
    WeaponDef = BG_GetWeaponDef(v4 % 128);
    v7 = BG_GetWeaponDef(client->ps.weapon);
    weapType = WeaponDef->weapType;
    v9 = v7;
    if (weapType == WEAPTYPE_GRENADE || BG_PlayerHasWeapon(&client->ps, v5))
        return 0;
    inventoryType = v9->inventoryType;
    if (inventoryType == WEAPINVENTORY_PRIMARY)
        return v5 + 4;
    if (inventoryType == WEAPINVENTORY_ALTMODE)
        return v5 + 4;
    v12 = BG_PlayerWeaponCountPrimaryTypes(&client->ps) >= 2;
    result = 0;
    if (!v12)
        return v5 + 4;
    return result;
}

void __cdecl Player_SetTurretDropHint(gentity_s *ent)
{
    gclient_s *client; // r28
    gentity_s *v3; // r29

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 543, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 544, 0, "%s", "ent->client");
    if (!ent->active)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 545, 0, "%s", "ent->active");
    client = ent->client;
    if ((client->ps.eFlags & 0x300) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            549,
            0,
            "%s",
            "ps->eFlags & EF_TURRET_ACTIVE");
    if (client->ps.viewlocked_entNum == 2175)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            550,
            0,
            "%s",
            "ps->viewlocked_entNum != ENTITYNUM_NONE");
    v3 = &level.gentities[client->ps.viewlocked_entNum];
    if (v3->s.eType != 10)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 553, 0, "%s", "turret->s.eType == ET_MG42");
    if (*BG_GetWeaponDef(v3->s.weapon)->dropHintString)
    {
        client->ps.cursorHintEntIndex = 2175;
        client->ps.cursorHint = v3->s.weapon + 4;
        client->ps.cursorHintString = BG_GetWeaponDef(v3->s.weapon)->dropHintStringIndex;
    }
}

void __cdecl Player_UpdateCursorHints(gentity_s *ent)
{
    gclient_s *client; // r23
    int v3; // r30
    int scale; // r21
    int cursorHintEntIndex; // r31
    gclient_s *v6; // r10
    int pm_flags; // r11
    int weaponstate; // r11
    int UseList; // r3
    int v10; // r27
    int v11; // r24
    useList_t *v12; // r28
    gentity_s *v13; // r31
    unsigned int weapon; // r3
    int ItemCursorHint; // r3
    int classname; // r11
    int v17; // r10
    bool v18; // cr34
    int v19; // r8
    useList_t v20[142]; // [sp+50h] [-4470h] BYREF

    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 579, 0, "%s", "ent->client");
    client = ent->client;
    v3 = 0;
    scale = -1;
    cursorHintEntIndex = client->ps.cursorHintEntIndex;
    client->ps.cursorHint = 0;
    client->ps.cursorHintString = -1;
    client->ps.cursorHintEntIndex = 2175;
    if (!BG_ThrowingBackGrenade(&client->ps))
        client->ps.throwBackGrenadeTimeLeft = 0;
    if (!g_reloading->current.integer && ent->health > 0)
    {
        G_UpdateFriendlyOverlay(ent);
        if (!G_IsVehicleUnusable(ent))
        {
            if (ent->active)
            {
                if ((client->ps.eFlags & 0x300) != 0 && (client->ps.weapFlags & 0x800) == 0)
                    Player_SetTurretDropHint(ent);
            }
            else
            {
                v6 = ent->client;
                pm_flags = v6->ps.pm_flags;
                if ((pm_flags & 4) == 0 && (pm_flags & 0x8000) == 0)
                {
                    weaponstate = client->ps.weaponstate;
                    if ((weaponstate < 15 || weaponstate > 19) && !v6->bFrozen)
                    {
                        UseList = Player_GetUseList(ent, v20, cursorHintEntIndex);
                        v10 = UseList;
                        if (UseList)
                        {
                            v11 = 0;
                            if (UseList > 0)
                            {
                                v12 = v20;
                                while (2)
                                {
                                    v13 = v12->ent;
                                    if (!v12->ent)
                                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 635, 0, "%s", "traceEnt");
                                    if (!v13->r.inuse)
                                        MyAssertHandler(
                                            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                            636,
                                            0,
                                            "%s",
                                            "traceEnt->r.inuse");
                                    switch (v13->s.eType)
                                    {
                                    case 0u:
                                        classname = v13->classname;
                                        if (classname == scr_const.trigger_use || classname == scr_const.trigger_use_touch)
                                        {
                                            v11 = *(unsigned int *)v13->s.un2;
                                            if (v11)
                                            {
                                                if (v13->s.un1.scale != 255)
                                                    scale = v13->s.un1.scale;
                                            }
                                        }
                                        goto LABEL_47;
                                    case 2u:
                                        ItemCursorHint = Player_GetItemCursorHint(ent->client, v13);
                                        if (!ItemCursorHint)
                                            goto LABEL_30;
                                        v11 = ItemCursorHint;
                                        goto LABEL_47;
                                    case 3u:
                                        if (!Actor_Grenade_InActorHands(v13))
                                        {
                                            v17 = *(unsigned __int16 *)v13->s.index;
                                            v18 = v13->nextthink == level.time;
                                            client->ps.throwBackGrenadeTimeLeft = v13->nextthink - level.time;
                                            v11 = v17 - (v17 >> 7 << 7) + 4;
                                            if (v18)
                                                MyAssertHandler(
                                                    "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                                    682,
                                                    0,
                                                    "%s",
                                                    "ps->throwBackGrenadeTimeLeft");
                                        }
                                        goto LABEL_47;
                                    case 0xAu:
                                        if (!G_IsTurretUsable(v13, ent))
                                            goto LABEL_30;
                                        weapon = v13->s.weapon;
                                        v11 = weapon + 4;
                                        goto LABEL_45;
                                    case 0xBu:
                                        if (!G_IsVehicleUsable(v13, ent))
                                            goto LABEL_30;
                                        weapon = v13->s.weapon;
                                        v11 = 1;
                                    LABEL_45:
                                        if (*BG_GetWeaponDef(weapon)->szUseHintString)
                                            scale = BG_GetWeaponDef(v13->s.weapon)->iUseHintStringIndex;
                                    LABEL_47:
                                        client->ps.cursorHintEntIndex = v13->s.number;
                                        v13->flags |= 0x400000u;
                                        client->ps.cursorHint = v11;
                                        client->ps.cursorHintString = scale;
                                        if (!v11)
                                            client->ps.cursorHintEntIndex = 2175;
                                        v19 = client->ps.cursorHintEntIndex;
                                        if (v19 != 2175 && !g_entities[v19].r.inuse)
                                            MyAssertHandler(
                                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                                710,
                                                1,
                                                "%s\n\t(ps->cursorHintEntIndex) = %i",
                                                "(ps->cursorHintEntIndex == ((2176)-1) || g_entities[ps->cursorHintEntIndex].r.inuse)",
                                                v19);
                                        return;
                                    case 0xEu:
                                        if (!v13->actor)
                                            MyAssertHandler(
                                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                                641,
                                                0,
                                                "%s",
                                                "traceEnt->actor");
                                        v11 = 1;
                                        if (v13->actor->iUseHintString >= 0)
                                            scale = v13->actor->iUseHintString;
                                        goto LABEL_47;
                                    default:
                                    LABEL_30:
                                        ++v3;
                                        ++v12;
                                        if (v3 >= v10)
                                            return;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

gentity_s *__cdecl Player_UpdateLookAtEntityTrace(
    trace_t *trace,
    const float *start,
    const float *end,
    int entNum,
    int contentMask,
    unsigned __int8 *priorityMap,
    float *forward)
{
    unsigned int EntityHitId; // r29
    double v16; // fp9
    double v17; // fp0
    double v18; // fp8
    double v19; // fp13
    float v20[24]; // [sp+50h] [-60h] BYREF

    if (!trace)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 726, 0, "%s", "trace");
    if (!priorityMap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 727, 0, "%s", "priorityMap");
    G_LocationalTrace(trace, start, end, entNum, contentMask, priorityMap);
    EntityHitId = Trace_GetEntityHitId(trace);
    if (EntityHitId >= 0x87E)
        return 0;
    v16 = forward[2];
    v17 = (float)(trace->fraction * (float)15000.0);
    v18 = start[2];
    v19 = (float)((float)(forward[1] * (float)(trace->fraction * (float)15000.0)) + start[1]);
    v20[0] = (float)(*forward * (float)(trace->fraction * (float)15000.0)) + *start;
    v20[1] = v19;
    v20[2] = (float)((float)v16 * (float)v17) + (float)v18;
    if (SV_FX_GetVisibility(start, v20) < 0.000099999997)
        return 0;
    else
        return &g_entities[EntityHitId];
}

int __cdecl Player_CheckAlmostStationary(gentity_s *ent, float *dir)
{
    gclient_s *client; // r29
    float *currentOrigin; // r30
    float *playerLOSCheckPos; // r31
    double v7; // fp1
    int result; // r3
    float v9; // [sp+50h] [-30h] BYREF
    float v10; // [sp+54h] [-2Ch]

    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 754, 0, "%s", "ent->client");
    v9 = *dir;
    client = ent->client;
    v10 = dir[1];
    Vec2Normalize(&v9);
    currentOrigin = ent->r.currentOrigin;
    v7 = Vec2DistanceSq(client->playerLOSCheckPos, ent->r.currentOrigin);
    playerLOSCheckPos = client->playerLOSCheckPos;
    if (v7 > ai_playerLOSHalfWidth->current.value
        || (float)((float)(client->playerLOSCheckDir[1] * v10) + (float)(client->playerLOSCheckDir[0] * v9)) < 0.98479998)
    {
        *playerLOSCheckPos = *currentOrigin;
        client->playerLOSCheckPos[1] = currentOrigin[1];
        client->playerLOSCheckDir[0] = v9;
        client->playerLOSCheckDir[1] = v10;
        client->playerLOSPosTime = level.time;
    }
    else
    {
        *playerLOSCheckPos = (float)((float)(*currentOrigin - *playerLOSCheckPos) * (float)0.25) + *playerLOSCheckPos;
        client->playerLOSCheckPos[1] = (float)((float)(currentOrigin[1] - client->playerLOSCheckPos[1]) * (float)0.25)
            + client->playerLOSCheckPos[1];
        client->playerLOSCheckDir[0] = (float)((float)(v9 - client->playerLOSCheckDir[0]) * (float)0.25)
            + client->playerLOSCheckDir[0];
        client->playerLOSCheckDir[1] = (float)((float)(v10 - client->playerLOSCheckDir[1]) * (float)0.25)
            + client->playerLOSCheckDir[1];
        if (ai_playerLOSMinTime->current.integer + client->playerLOSPosTime <= level.time)
        {
            result = 1;
            client->playerLOSPosTime = level.time;
            return result;
        }
    }
    return 0;
}

void __cdecl Player_DebugDrawLOS(const float *center, const float *dir, double dist2D, int debugDrawDuration)
{
    double v5; // fp1
    const float *v6; // r6
    float v7[4]; // [sp+50h] [-40h] BYREF
    float v8[12]; // [sp+60h] [-30h] BYREF

    v8[0] = (float)dist2D * (float)-0.5;
    v8[1] = -ai_playerLOSHalfWidth->current.value;
    v8[2] = -20.0;
    v7[0] = (float)dist2D * (float)0.5;
    v7[1] = ai_playerLOSHalfWidth->current.value;
    v7[2] = 20.0;
    v5 = vectoyaw(dir);
    G_DebugBox(center, v8, v7, v5, v6, (int)colorOrange, 0);
}

void __cdecl Player_BanNodesInFront(gentity_s *ent, double dist, const float *start, const float *dir, int a5)
{
    double v9; // fp0
    double v10; // fp31
    double v11; // fp11
    double v12; // fp13
    int v13; // r5
    pathsort_t *v14; // r4
    pathnode_tree_t *v15; // r30
    int v16; // r5
    int v17; // r19
    double v18; // fp31
    pathsort_t *v19; // r22
    pathnode_t *node; // r29
    sentient_s *v21; // r3
    sentient_s *v22; // r31
    actor_s *actor; // r27
    sentient_s *sentient; // r11
    float v25; // [sp+50h] [-E0h] BYREF
    float v26; // [sp+54h] [-DCh]
    float v27; // [sp+58h] [-D8h]
    float v28[4]; // [sp+60h] [-D0h] BYREF
    pathsort_t v29[4]; // [sp+70h] [-C0h] BYREF

    if (__fabs((float)((float)((float)(*(float *)(a5 + 8) * *(float *)(a5 + 8))
        + (float)((float)(*(float *)a5 * *(float *)a5)
            + (float)(*(float *)(a5 + 4) * *(float *)(a5 + 4))))
        - (float)1.0)) >= 0.0020000001)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 820, 0, "%s", "Vec3IsNormalized( dir )");
    if (ent->sentient->eTeam == TEAM_DEAD)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            821,
            0,
            "%s",
            "ent->sentient->eTeam != TEAM_DEAD");
    v9 = (float)((float)((float)dist * *(float *)(a5 + 4)) + dir[1]);
    v25 = (float)(*(float *)a5 * (float)dist) + *dir;
    v27 = (float)((float)dist * *(float *)(a5 + 8)) + dir[2];
    v26 = v9;
    v10 = Vec2Distance(dir, &v25);
    v11 = (float)(dir[2] + v27);
    v12 = (float)((float)(dir[1] + v26) * (float)0.5);
    v28[0] = (float)(*dir + v25) * (float)0.5;
    v28[1] = v12;
    v28[2] = (float)v11 * (float)0.5;
    v15 = Path_NodesInCylinder(v28, (float)((float)v10 * (float)0.5), 80.0, v14, v13, v29, 4, 270332);
    v17 = ai_playerLOSMinTime->current.integer / 50;
    if (ai_debugPlayerLOS->current.enabled)
        Player_DebugDrawLOS(v28, (const float *)a5, v10, v16);
    v18 = (float)(ai_playerLOSHalfWidth->current.value * ai_playerLOSHalfWidth->current.value);
    if ((int)v15 > 0)
    {
        v19 = v29;
        do
        {
            node = v19->node;
            if (PointToLineDistSq2D(v19->node->constant.vOrigin, dir, &v25) <= v18)
            {
                //if (SentientHandle::isDefined(&node->dynamic.pOwner))
                if (node->dynamic.pOwner.isDefined())
                {
                    //v21 = SentientHandle::sentient(&node->dynamic.pOwner);
                    v21 = node->dynamic.pOwner.sentient();
                    v22 = v21;
                    if (v21)
                    {
                        if (v21->pClaimedNode == node)
                        {
                            if (Actor_PointNearNode(v21->ent->r.currentOrigin, node))
                                goto LABEL_23;
                            actor = v22->ent->actor;
                            if (!actor)
                                goto LABEL_23;
                            sentient = ent->sentient;
                            if (v22->eTeam != sentient->eTeam)
                                goto LABEL_23;
                            if (v22 == sentient)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                    857,
                                    0,
                                    "%s",
                                    "nodeOwner != ent->sentient");
                            if ((unsigned __int8)Actor_KeepClaimedNode(actor))
                                goto LABEL_23;
                            if (actor->numCoverNodesInGoal > 1)
                            {
                                Path_RelinquishNodeSoon(v22);
                                //SentientHandle::setSentient(&node->dynamic.pOwner, 0);
                                node->dynamic.pOwner.setSentient(0);
                            }
                        }
                    }
                }
                node->dynamic.inPlayerLOSTime = ai_playerLOSMinTime->current.integer + level.time;
                if (ai_debugPlayerLOS->current.enabled)
                    Path_DrawDebugNoLinks(node, (const float (*)[4])colorOrange, v17);
            }
        LABEL_23:
            v15 = (pathnode_tree_t *)((char *)v15 - 1);
            ++v19;
        } while (v15);
    }
}

void __cdecl Player_BlockFriendliesInADS(gentity_s *ent, double dist, const float *start, const float *dir, int a5)
{
    int BestTarget; // r29
    gclient_s *client; // r10
    sentient_s *sentient; // r10
    double v12; // fp0
    float v13[6]; // [sp+50h] [-50h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 886, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 887, 0, "%s", "ent->client");
    BestTarget = AimTarget_GetBestTarget(dir, (const float *)a5);
    client = ent->client;
    if (BestTarget == 2175)
    {
        if (level.time - client->playerADSTargetTime > ai_playerADSTargetTime->current.integer)
            return;
    }
    else
    {
        client->playerADSTargetTime = level.time;
    }
    sentient = ent->sentient;
    v12 = (float)((float)(*(float *)(a5 + 4) * (float)dist) + dir[1]);
    v13[0] = (float)(*(float *)a5 * (float)dist) + *dir;
    v13[2] = (float)(*(float *)(a5 + 8) * (float)dist) + dir[2];
    v13[1] = v12;
    Actor_BroadcastLineEvent(ent, 14, 1 << sentient->eTeam, dir, v13, 0.0);
    if (ai_debugPlayerLOS->current.enabled)
        G_DebugLine(ent->r.currentOrigin, g_entities[BestTarget].r.currentOrigin, colorCyan, 0);
}

void __cdecl Player_GrenadeThrowBlockFriendlies(
    gentity_s *ent,
    double dist,
    const float *start,
    const float *dir,
    float *a5)
{
    sentient_s *sentient; // r10
    double v10; // fp0
    float v11[6]; // [sp+50h] [-50h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 920, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 921, 0, "%s", "ent->client");
    if ((ent->client->ps.weapFlags & 2) != 0)
    {
        sentient = ent->sentient;
        v10 = (float)((float)(a5[1] * (float)dist) + dir[1]);
        v11[0] = (float)(*a5 * (float)dist) + *dir;
        v11[2] = (float)(a5[2] * (float)dist) + dir[2];
        v11[1] = v10;
        Actor_BroadcastLineEvent(ent, 14, 1 << sentient->eTeam, dir, v11, 0.0);
    }
}

void __cdecl Player_UpdateLookAtEntity(gentity_s *ent)
{
    gclient_s *client; // r21
    unsigned int weapon; // r3
    WeaponDef *WeaponDef; // r3
    WeaponDef *v5; // r23
    unsigned __int8 *v6; // r27
    int number; // r6
    gentity_s *updated; // r31
    const float *v9; // r4
    gclient_s *v12; // r11
    double value; // fp1
    double v14; // fp30
    double v15; // fp29
    double v16; // fp28
    const dvar_s *v17; // r11
    double v18; // fp30
    const dvar_s *v19; // r11
    int v20; // r10
    actor_s *actor; // r11
    const dvar_s *v22; // r11
    double v23; // fp30
    double v24; // fp29
    double v25; // fp28
    double v26; // fp30
    char v27; // r3
    int weapFlags; // r11
    const dvar_s *v29; // r11
    double v30; // fp30
    const dvar_s *v31; // r11
    float v32; // [sp+58h] [-E8h] BYREF
    float v33; // [sp+5Ch] [-E4h]
    float v34; // [sp+60h] [-E0h]
    float v35; // [sp+68h] [-D8h] BYREF
    float v36; // [sp+6Ch] [-D4h]
    float v37; // [sp+70h] [-D0h]
    float v38[6]; // [sp+78h] [-C8h] BYREF
    trace_t v39; // [sp+90h] [-B0h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 946, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 947, 0, "%s", "ent->client");
    if (!g_friendlyNameDist)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 948, 0, "%s", "g_friendlyNameDist");
    if (!g_friendlyfireDist)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 949, 0, "%s", "g_friendlyfireDist");
    client = ent->client;
    client->ps.weapFlags &= 0xFFFFFDE7;
    EntHandle::setEnt(&ent->client->pLookatEnt, 0);
    G_GetPlayerViewOrigin(&client->ps, &v32);
    G_GetPlayerViewDirection(ent, &v35, 0, 0);
    if ((client->ps.eFlags & 0x20300) != 0)
    {
        if (client->ps.viewlocked_entNum == 2175)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                963,
                0,
                "%s",
                "ps->viewlocked_entNum != ENTITYNUM_NONE");
        weapon = g_entities[client->ps.viewlocked_entNum].s.weapon;
    }
    else
    {
        weapon = ent->client->ps.weapon;
    }
    WeaponDef = BG_GetWeaponDef(weapon);
    v5 = WeaponDef;
    if (ent->client->ps.weapon && WeaponDef->bRifleBullet)
        v6 = riflePriorityMap;
    else
        v6 = bulletPriorityMap;
    if ((LODWORD(v32) & 0x7F800000) == 0x7F800000
        || (LODWORD(v33) & 0x7F800000) == 0x7F800000
        || (LODWORD(v34) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            976,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((LODWORD(v35) & 0x7F800000) == 0x7F800000
        || (LODWORD(v36) & 0x7F800000) == 0x7F800000
        || (LODWORD(v37) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
            977,
            0,
            "%s",
            "!IS_NAN((forward)[0]) && !IS_NAN((forward)[1]) && !IS_NAN((forward)[2])");
    }
    number = ent->s.number;
    v38[0] = (float)(v35 * (float)15000.0) + v32;
    v38[1] = (float)(v36 * (float)15000.0) + v33;
    v38[2] = (float)(v37 * (float)15000.0) + v34;
    updated = Player_UpdateLookAtEntityTrace(&v39, &v32, v38, number, 578873345, v6, &v35);
    if ((unsigned __int8)Player_CheckAlmostStationary(ent, &v35))
    {
        _FP12 = (float)((float)(v39.fraction * (float)15000.0) - ai_playerLOSRange->current.value);
        __asm { fsel      f1, f12, f0, f13# dist }
        Player_BanNodesInFront(ent, _FP1, v9, &v32, (int)&v35);
    }
    v12 = ent->client;
    if ((v12->ps.pm_flags & 0x10) != 0 && v12->ps.fWeaponPosFrac == 1.0)
    {
        value = ai_playerADS_LOSRange->current.value;
        if (value != 0.0)
            Player_BlockFriendliesInADS(ent, value, v9, &v32, (int)&v35);
    }
    Player_GrenadeThrowBlockFriendlies(ent, ai_playerLOSRange->current.value, v9, &v32, &v35);
    if (updated)
    {
        if (updated->classname != scr_const.trigger_lookat
            || (EntHandle::setEnt(&ent->client->pLookatEnt, updated),
                G_Trigger(updated, ent),
                (updated = Player_UpdateLookAtEntityTrace(&v39, &v32, v38, ent->s.number, 42002433, v6, &v35)) != 0))
        {
            if ((updated->r.contents & 0x4000) != 0)
            {
                if ((v39.surfaceFlags & 0x10) == 0)
                {
                    if (!updated->sentient)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 1018, 0, "%s", "traceEnt->sentient");
                    v14 = (float)(updated->r.currentOrigin[0] - v32);
                    v15 = (float)(updated->r.currentOrigin[1] - v33);
                    v16 = (float)(updated->r.currentOrigin[2] - v34);
                    if (((1 << updated->sentient->eTeam) & ~(1 << Sentient_EnemyTeam(ent->sentient->eTeam))) != 0)
                    {
                        v17 = g_friendlyNameDist;
                        if (g_friendlyNameDist->current.value > 15000.0)
                        {
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                1024,
                                0,
                                "%s",
                                "g_friendlyNameDist->current.value <= MAX_FRIENDLY_DIST");
                            v17 = g_friendlyNameDist;
                        }
                        v18 = (float)((float)((float)v14 * (float)v14)
                            + (float)((float)((float)v16 * (float)v16) + (float)((float)v15 * (float)v15)));
                        if (v18 < (float)(v17->current.value * v17->current.value)
                            && !EntHandle::isDefined(&ent->client->pLookatEnt))
                        {
                            EntHandle::setEnt(&ent->client->pLookatEnt, updated);
                        }
                        v19 = g_friendlyfireDist;
                        if (g_friendlyfireDist->current.value > 15000.0)
                        {
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                1028,
                                0,
                                "%s",
                                "g_friendlyfireDist->current.value <= MAX_FRIENDLY_DIST");
                            v19 = g_friendlyfireDist;
                        }
                        if (v18 < (float)(v19->current.value * v19->current.value))
                        {
                            v20 = client->ps.weapFlags | 8;
                            client->ps.weapFlags = v20;
                            actor = updated->actor;
                            if (actor)
                            {
                                if (actor->bDontAvoidPlayer || (actor->Physics.iTraceMask & 0x2000000) == 0)
                                    client->ps.weapFlags = v20 | 0x200;
                            }
                        }
                    }
                    else
                    {
                        if (v5->enemyCrosshairRange > 15000.0)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                1039,
                                0,
                                "%s",
                                "weapDef->enemyCrosshairRange <= MAX_FRIENDLY_DIST");
                        if ((float)((float)((float)v14 * (float)v14)
                            + (float)((float)((float)v16 * (float)v16) + (float)((float)v15 * (float)v15))) < (double)(float)(v5->enemyCrosshairRange * v5->enemyCrosshairRange))
                        {
                            if (!EntHandle::isDefined(&ent->client->pLookatEnt))
                                EntHandle::setEnt(&ent->client->pLookatEnt, updated);
                            client->ps.weapFlags |= 0x10u;
                        }
                    }
                }
                return;
            }
            if (updated->s.eType != 11 || EntHandle::isDefined(&ent->client->pLookatEnt))
            {
                if (updated->lookAtText0 && !EntHandle::isDefined(&ent->client->pLookatEnt))
                {
                    v29 = g_friendlyNameDist;
                    v30 = (float)((float)((float)(updated->r.currentOrigin[0] - v32) * (float)(updated->r.currentOrigin[0] - v32))
                        + (float)((float)((float)(updated->r.currentOrigin[2] - v34)
                            * (float)(updated->r.currentOrigin[2] - v34))
                            + (float)((float)(updated->r.currentOrigin[1] - v33)
                                * (float)(updated->r.currentOrigin[1] - v33))));
                    if (g_friendlyNameDist->current.value > 15000.0)
                    {
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                            1083,
                            0,
                            "%s",
                            "g_friendlyNameDist->current.value <= MAX_FRIENDLY_DIST");
                        v29 = g_friendlyNameDist;
                    }
                    if (v30 < (float)(v29->current.value * v29->current.value))
                        EntHandle::setEnt(&ent->client->pLookatEnt, updated);
                    if (updated->s.eType == 5)
                    {
                        v31 = g_friendlyfireDist;
                        if (g_friendlyfireDist->current.value > 15000.0)
                        {
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                                1089,
                                0,
                                "%s",
                                "g_friendlyfireDist->current.value <= MAX_FRIENDLY_DIST");
                            v31 = g_friendlyfireDist;
                        }
                        if (v30 < (float)(v31->current.value * v31->current.value))
                        {
                            weapFlags = client->ps.weapFlags;
                        LABEL_83:
                            client->ps.weapFlags = weapFlags | 8;
                        }
                    }
                }
            }
            else
            {
                if (!updated->scr_vehicle)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp", 1054, 0, "%s", "traceEnt->scr_vehicle");
                v22 = g_friendlyNameDist;
                v23 = (float)(updated->r.currentOrigin[0] - v32);
                v24 = (float)(updated->r.currentOrigin[1] - v33);
                v25 = (float)(updated->r.currentOrigin[2] - v34);
                if (g_friendlyNameDist->current.value > 15000.0)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                        1057,
                        0,
                        "%s",
                        "g_friendlyNameDist->current.value <= MAX_FRIENDLY_DIST");
                    v22 = g_friendlyNameDist;
                }
                v26 = (float)((float)((float)v23 * (float)v23)
                    + (float)((float)((float)v25 * (float)v25) + (float)((float)v24 * (float)v24)));
                if (v26 < (float)(v22->current.value * v22->current.value))
                    EntHandle::setEnt(&ent->client->pLookatEnt, updated);
                if (v5->enemyCrosshairRange > 15000.0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\player_use.cpp",
                        1061,
                        0,
                        "%s",
                        "weapDef->enemyCrosshairRange <= MAX_FRIENDLY_DIST");
                if (v26 < (float)(v5->enemyCrosshairRange * v5->enemyCrosshairRange) && (client->ps.eFlags & 0x20000) != 0)
                {
                    v27 = Sentient_EnemyTeam(ent->sentient->eTeam);
                    weapFlags = client->ps.weapFlags;
                    if (((1 << v27) & (1 << updated->scr_vehicle->team)) != 0)
                    {
                        client->ps.weapFlags = weapFlags | 0x10;
                        return;
                    }
                    goto LABEL_83;
                }
            }
        }
    }
}

