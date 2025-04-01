#include "g_public_mp.h"


void __cdecl Player_UpdateActivate(gentity_s *ent)
{
    bool useSucceeded; // [esp+3h] [ebp-1h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 165, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 166, 0, "%s", "ent->client");
    ent->client->ps.weapFlags &= ~1u;
    useSucceeded = 0;
    if (EntHandle::isDefined(&ent->client->useHoldEntity)
        && (ent->client->oldbuttons & 0x20) != 0
        && (ent->client->buttons & 0x20) == 0)
    {
        ent->client->ps.weapFlags |= 1u;
    }
    else
    {
        if ((ent->client->latched_buttons & 0x28) != 0)
            useSucceeded = Player_ActivateCmd(ent);
        if (EntHandle::isDefined(&ent->client->useHoldEntity) || useSucceeded)
        {
            if ((ent->client->buttons & 0x28) != 0)
                Player_ActivateHoldCmd(ent);
            ent->client->useButtonDone = 1;
        }
        else if ((ent->client->latched_buttons & 0x20) != 0)
        {
            ent->client->ps.weapFlags |= 1u;
        }
    }
}

char __cdecl Player_ActivateCmd(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 82, 0, "%s", "ent");
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 83, 0, "%s", "ent->r.inuse");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 84, 0, "%s", "ent->client");
    if (!Scr_IsSystemActive())
        return 0;
    EntHandle::setEnt(&ent->client->useHoldEntity, 0);
    if (ent->active)
    {
        if ((ent->client->ps.eFlags & 0x300) != 0)
            ent->active = 2;
        else
            ent->active = 0;
        return 1;
    }
    else if ((ent->client->ps.pm_flags & 4) != 0)
    {
        return 1;
    }
    else if ((ent->client->ps.pm_flags & 0x8000) != 0)
    {
        return 1;
    }
    else if (ent->client->ps.weaponstate < 15 || ent->client->ps.weaponstate > 20)
    {
        if (ent->client->ps.cursorHint)
        {
            if (ent->client->ps.cursorHintEntIndex == 1023)
            {
                return 0;
            }
            else
            {
                if (!g_entities[ent->client->ps.cursorHintEntIndex].r.inuse)
                    MyAssertHandler(
                        ".\\game_mp\\player_use_mp.cpp",
                        117,
                        0,
                        "%s",
                        "g_entities[ ent->client->ps.cursorHintEntIndex ].r.inuse");
                EntHandle::setEnt(&ent->client->useHoldEntity, &g_entities[ent->client->ps.cursorHintEntIndex]);
                ent->client->useHoldTime = level.time;
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 1;
    }
}

void __cdecl Player_ActivateHoldCmd(gentity_s *ent)
{
    int v1; // eax
    gentity_s *useEnt; // [esp+0h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 133, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 134, 0, "%s", "ent->client");
    if (Scr_IsSystemActive()
        && EntHandle::isDefined(&ent->client->useHoldEntity)
        && level.time - ent->client->lastSpawnTime >= g_useholdspawndelay->current.integer
        && level.time - ent->client->useHoldTime >= g_useholdtime->current.integer)
    {
        useEnt = EntHandle::ent(&ent->client->useHoldEntity);
        if (useEnt->s.number != EntHandle::entnum(&ent->client->useHoldEntity))
        {
            v1 = EntHandle::entnum(&ent->client->useHoldEntity);
            MyAssertHandler(
                ".\\game_mp\\player_use_mp.cpp",
                151,
                0,
                "useEnt->s.number == (int)ent->client->useHoldEntity.entnum()\n\t%i, %i",
                useEnt->s.number,
                v1);
        }
        if (!useEnt->r.inuse)
            MyAssertHandler(
                ".\\game_mp\\player_use_mp.cpp",
                152,
                0,
                "%s\n\t(useEnt->s.number) = %i",
                "(useEnt->r.inuse)",
                useEnt->s.number);
        Player_UseEntity(ent, useEnt);
    }
}

void __cdecl Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt)
{
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+0h] [ebp-8h]
    void(__cdecl * use)(gentity_s *, gentity_s *, gentity_s *); // [esp+4h] [ebp-4h]

    if (!playerEnt)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 41, 0, "%s", "playerEnt");
    if (!playerEnt->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 42, 0, "%s", "playerEnt->client");
    if (!useEnt)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 43, 0, "%s", "useEnt");
    if (!useEnt->r.inuse)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 45, 0, "%s", "useEnt->r.inuse");
    if (useEnt->s.eType == 3)
    {
        Scr_AddEntity(playerEnt);
        Scr_Notify(useEnt, scr_const.touch, 1u);
        useEnt->active = 1;
        touch = (void(__cdecl *)(gentity_s *, gentity_s *, int))dword_946724[10 * useEnt->handler];
        if (touch)
            touch(useEnt, playerEnt, 0);
    }
    else if (useEnt->s.eType != 11 || G_IsTurretUsable(useEnt, playerEnt))
    {
        Scr_AddEntity(playerEnt);
        Scr_Notify(useEnt, scr_const.trigger, 1u);
        use = (void(__cdecl *)(gentity_s *, gentity_s *, gentity_s *))dword_946728[10 * useEnt->handler];
        if (use)
            use(useEnt, playerEnt, playerEnt);
    }
    EntHandle::setEnt(&playerEnt->client->useHoldEntity, 0);
}

void __cdecl Player_UpdateCursorHints(gentity_s *ent)
{
    int piIndex; // [esp+4h] [ebp-2020h] BYREF
    int scale; // [esp+8h] [ebp-201Ch]
    int ItemCursorHint; // [esp+Ch] [ebp-2018h]
    int hintString; // [esp+10h] [ebp-2014h]
    useList_t useList[1024]; // [esp+14h] [ebp-2010h] BYREF
    int v6; // [esp+2014h] [ebp-10h]
    int v7; // [esp+2018h] [ebp-Ch]
    playerState_s *ps; // [esp+201Ch] [ebp-8h]
    gentity_s *self; // [esp+2020h] [ebp-4h]

    self = 0;
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 525, 0, "%s", "ent->client");
    ps = &ent->client->ps;
    ps->cursorHint = 0;
    if (!BG_ThrowingBackGrenade(ps))
        ps->throwBackGrenadeTimeLeft = 0;
    if (ent->health > 0)
    {
        if (ent->active)
        {
            if ((ps->eFlags & 0x300) != 0)
                Player_SetTurretDropHint(ent);
        }
        else if ((ps->pm_flags & 0x100000) != 0)
        {
            Player_SetVehicleDropHint(ent);
        }
        else if ((ent->client->ps.pm_flags & 4) == 0
            && (ent->client->ps.pm_flags & 0x8000) == 0
            && (ps->weaponstate < 15 || ps->weaponstate > 19))
        {
            if (ps->pm_type == 6)
            {
                ps->cursorHintEntIndex = 1023;
            }
            else if (!ent->client->bFrozen)
            {
                v6 = Player_GetUseList(ent, useList, ps->cursorHintEntIndex);
                if (v6)
                {
                    hintString = 0;
                    scale = -1;
                    v7 = 0;
                    while (2)
                    {
                        if (v7 < v6)
                        {
                            self = useList[v7].ent;
                            if (!self)
                                MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 582, 0, "%s", "traceEnt");
                            if (!self->r.inuse)
                                MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 583, 0, "%s", "traceEnt->r.inuse");
                            switch (self->s.eType)
                            {
                            case 0:
                                if (self->classname != scr_const.trigger_use && self->classname != scr_const.trigger_use_touch)
                                    goto LABEL_49;
                                if (self->team && self->team != ent->client->sess.cs.team
                                    || self->item[1].ammoCount != 1023 && self->item[1].ammoCount != ent->client->ps.clientNum)
                                {
                                    goto LABEL_21;
                                }
                                hintString = self->s.un2.hintString;
                                if (self->s.un2.hintString && self->s.un1.scale != 255)
                                    scale = self->s.un1.scale;
                                goto LABEL_49;
                            case 3:
                                ItemCursorHint = Player_GetItemCursorHint(ent->client, self);
                                if (!ItemCursorHint)
                                    goto LABEL_21;
                                hintString = ItemCursorHint;
                                goto LABEL_49;
                            case 4:
                                hintString = self->s.index.brushmodel % 128 + 4;
                                ps->throwBackGrenadeTimeLeft = self->nextthink - level.time;
                                goto LABEL_49;
                            case 0xB:
                                if (!G_IsTurretUsable(self, ent))
                                    goto LABEL_21;
                                hintString = self->s.weapon + 4;
                                if (*BG_GetWeaponDef(self->s.weapon)->szUseHintString)
                                    scale = BG_GetWeaponDef(self->s.weapon)->iUseHintStringIndex;
                                goto LABEL_49;
                            case 0xE:
                                if (!G_VehUsable(self, ent))
                                    goto LABEL_21;
                                hintString = 1;
                                G_GetHintStringIndex(&piIndex, "MP_USEVEHICLE");
                                if (*BG_GetWeaponDef(self->s.weapon)->szUseHintString)
                                    scale = BG_GetWeaponDef(self->s.weapon)->iUseHintStringIndex;
                                else
                                    scale = piIndex;
                            LABEL_49:
                                if (hintString)
                                {
                                    ps->cursorHintEntIndex = self->s.number;
                                    self->flags |= 0x200000u;
                                    ps->cursorHint = hintString;
                                    ps->cursorHintString = scale;
                                }
                                else
                                {
                                    ps->cursorHintEntIndex = 1023;
                                }
                                if (ps->cursorHintEntIndex != 1023 && !g_entities[ps->cursorHintEntIndex].r.inuse)
                                    MyAssertHandler(
                                        ".\\game_mp\\player_use_mp.cpp",
                                        661,
                                        1,
                                        "%s\n\t(ps->cursorHintEntIndex) = %i",
                                        "(ps->cursorHintEntIndex == ((1<<10)-1) || g_entities[ps->cursorHintEntIndex].r.inuse)",
                                        ps->cursorHintEntIndex);
                                return;
                            default:
                            LABEL_21:
                                ++v7;
                                continue;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

int __cdecl Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex)
{
    double v3; // st7
    int eType; // [esp+8h] [ebp-10D4h]
    float v6; // [esp+Ch] [ebp-10D0h]
    float v7; // [esp+10h] [ebp-10CCh]
    float v8; // [esp+14h] [ebp-10C8h]
    float v9; // [esp+18h] [ebp-10C4h]
    float v10; // [esp+28h] [ebp-10B4h]
    float v11; // [esp+2Ch] [ebp-10B0h]
    float value; // [esp+30h] [ebp-10ACh]
    float v13; // [esp+34h] [ebp-10A8h]
    float v14; // [esp+3Ch] [ebp-10A0h]
    float a[3]; // [esp+40h] [ebp-109Ch] BYREF
    int entityList[1024]; // [esp+4Ch] [ebp-1090h] BYREF
    float origin[3]; // [esp+104Ch] [ebp-90h] BYREF
    float v18[3]; // [esp+1058h] [ebp-84h] BYREF
    float v19; // [esp+1064h] [ebp-78h]
    float diff[3]; // [esp+1068h] [ebp-74h] BYREF
    float v21; // [esp+1074h] [ebp-68h]
    float b[3]; // [esp+1078h] [ebp-64h] BYREF
    float v23; // [esp+1084h] [ebp-58h]
    gentity_s *gEnt; // [esp+1088h] [ebp-54h]
    float forward[3]; // [esp+108Ch] [ebp-50h] BYREF
    float v26; // [esp+1098h] [ebp-44h]
    unsigned int num; // [esp+109Ch] [ebp-40h]
    int v28; // [esp+10A0h] [ebp-3Ch]
    float maxs[3]; // [esp+10A4h] [ebp-38h] BYREF
    float v[3]; // [esp+10B0h] [ebp-2Ch] BYREF
    int v31; // [esp+10BCh] [ebp-20h]
    int v32; // [esp+10C0h] [ebp-1Ch]
    signed int i; // [esp+10C4h] [ebp-18h]
    playerState_s *ps; // [esp+10C8h] [ebp-14h]
    float v35; // [esp+10CCh] [ebp-10h]
    float sum[3]; // [esp+10D0h] [ebp-Ch] BYREF
    gentity_s *enta; // [esp+10E4h] [ebp+8h]

    v19 = 256.0;
    v35 = 0.75999999;
    v31 = 0;
    value = player_throwbackOuterRadius->current.value;
    v9 = 192.0 - value;
    if (v9 < 0.0)
        v10 = value;
    else
        v10 = 192.0;
    v11 = player_MGUseRadius->current.value;
    v8 = v10 - v11;
    if (v8 < 0.0)
        v7 = v11;
    else
        v7 = v10;
    b[1] = v7;
    b[0] = v7;
    b[2] = 96.0;
    ps = &ent->client->ps;
    G_GetPlayerViewOrigin(ps, origin);
    BG_GetPlayerViewDirection(ps, forward, 0, 0);
    Vec3Add(ps->origin, playerMins, sum);
    Vec3Add(ps->origin, playerMaxs, v18);
    Vec3Sub(origin, b, diff);
    Vec3Add(origin, b, maxs);
    v32 = CM_AreaEntities(diff, maxs, entityList, 1024, 0x200000);
    num = 0;
    for (i = 0; i < v32; ++i)
    {
        gEnt = &g_entities[entityList[i]];
        if (ent != gEnt && (gEnt->s.eType == 3 || (gEnt->r.contents & 0x200000) != 0))
        {
            if (gEnt->classname == scr_const.trigger_use_touch)
            {
                if (v18[0] >= (double)gEnt->r.absmin[0]
                    && sum[0] <= (double)gEnt->r.absmax[0]
                    && v18[1] >= (double)gEnt->r.absmin[1]
                    && sum[1] <= (double)gEnt->r.absmax[1]
                    && v18[2] >= (double)gEnt->r.absmin[2]
                    && sum[2] <= (double)gEnt->r.absmax[2])
                {
                    if (SV_EntityContact(sum, v18, gEnt))
                    {
                        useList[num].score = -256.0;
                        useList[num++].ent = gEnt;
                    }
                }
            }
            else if (gEnt->s.eType != 4
                || (prevHintEntIndex == gEnt->s.number
                    || (v13 = player_throwbackInnerRadius->current.value,
                        v13 * v13 >= Vec2DistanceSq(gEnt->r.currentOrigin, ent->r.currentOrigin)))
                && (v3 = Vec3LengthSq(gEnt->s.lerp.pos.trDelta),
                    v6 = bg_maxGrenadeIndicatorSpeed->current.value * bg_maxGrenadeIndicatorSpeed->current.value,
                    v6 >= v3))
            {
                eType = gEnt->s.eType;
                if (eType == 4)
                    v23 = player_throwbackOuterRadius->current.value;
                else
                    v23 = eType == 11 ? player_MGUseRadius->current.value : 128.0;
                Vec3Add(gEnt->r.absmin, gEnt->r.absmax, v);
                Vec3Scale(v, 0.5, v);
                Vec3Sub(v, origin, a);
                v21 = Vec3Normalize(a);
                if (v23 >= (double)v21)
                {
                    v14 = Vec3Dot(a, forward);
                    if (gEnt->classname != scr_const.trigger_use || !gEnt->trigger.requireLookAt || v35 <= (double)v14)
                    {
                        v26 = 1.0 - (v14 + 1.0) * 0.5;
                        useList[num].score = v26 * v19;
                        if (gEnt->s.eType == 4)
                            useList[num].score = useList[num].score - (v19 + v19);
                        if (gEnt->classname == scr_const.trigger_use)
                            useList[num].score = useList[num].score - v19;
                        if (gEnt->s.eType == 11)
                            useList[num].score = useList[num].score - v19 * 0.5;
                        if (gEnt->s.eType == 3 && !BG_CanItemBeGrabbed(&gEnt->s, &ent->client->ps, 0))
                        {
                            useList[num].score = useList[num].score + 10000.0;
                            ++v31;
                        }
                        useList[num].ent = gEnt;
                        useList[num].score = useList[num].score + v21;
                        ++num;
                    }
                }
            }
        }
    }
    qsort(useList, num, 8u, (int(__cdecl *)(const void *, const void *))compare_use);
    num -= v31;
    v28 = 0;
    for (i = 0; i < (int)num; ++i)
    {
        enta = useList[i].ent;
        if (enta->classname != scr_const.trigger_use_touch && enta->classname != scr_const.script_vehicle)
        {
            Vec3Add(enta->r.absmin, enta->r.absmax, v);
            Vec3Scale(v, 0.5, v);
            if (enta->s.eType == 11)
                G_DObjGetWorldTagPos(enta, scr_const.tag_aim, v);
            if (!G_TraceCapsuleComplete(origin, (float *)vec3_origin, (float *)vec3_origin, v, ps->clientNum, 17))
            {
                useList[i].score = useList[i].score + 10000.0;
                ++v28;
            }
        }
    }
    qsort(useList, num, 8u, (int(__cdecl *)(const void *, const void *))compare_use);
    return num - v28;
}

int __cdecl compare_use(float *pe1, float *pe2)
{
    return (int)(pe1[1] - pe2[1]);
}

int __cdecl Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt)
{
    WeaponDef *weapDefItem; // [esp+0h] [ebp-14h]
    WeaponDef *weapDefPlayer; // [esp+8h] [ebp-Ch]
    int index; // [esp+Ch] [ebp-8h]
    int weapIndex; // [esp+10h] [ebp-4h]

    if (!traceEnt)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 436, 0, "%s", "traceEnt");
    if (!client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 437, 0, "%s", "client");
    index = traceEnt->s.index.brushmodel;
    if ((unsigned int)index >= 0x800)
        MyAssertHandler(
            ".\\game_mp\\player_use_mp.cpp",
            441,
            0,
            "%s\n\t(index) = %i",
            "((0 <= index) && (index < (128 * NUM_WEAP_ALTMODELS )))",
            index);
    if (bg_itemlist[index] != 1)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 444, 0, "%s", "item->giType == IT_WEAPON");
    weapIndex = index % 128;
    weapDefItem = BG_GetWeaponDef(index % 128);
    weapDefPlayer = BG_GetWeaponDef(client->ps.weapon);
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, weapIndex, 16))
        return 0;
    if (weapDefPlayer->inventoryType == WEAPINVENTORY_PRIMARY
        || weapDefPlayer->inventoryType == WEAPINVENTORY_ALTMODE
        || weapDefItem->inventoryType && weapDefItem->inventoryType != WEAPINVENTORY_ALTMODE
        || BG_PlayerWeaponCountPrimaryTypes(&client->ps) < 2)
    {
        return weapIndex + 4;
    }
    return 0;
}

void __cdecl Player_SetTurretDropHint(gentity_s *ent)
{
    gentity_s *turret; // [esp+0h] [ebp-8h]
    gclient_s *ps; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 471, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 472, 0, "%s", "ent->client");
    if (!ent->active)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 473, 0, "%s", "ent->active");
    ps = ent->client;
    if ((ps->ps.eFlags & 0x300) == 0)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 477, 0, "%s", "ps->eFlags & EF_TURRET_ACTIVE");
    if (ps->ps.viewlocked_entNum == 1023)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 478, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
    turret = &level.gentities[ps->ps.viewlocked_entNum];
    if (turret->s.eType != 11)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 481, 0, "%s", "turret->s.eType == ET_MG42");
    if (*BG_GetWeaponDef(turret->s.weapon)->dropHintString)
    {
        ps->ps.cursorHintEntIndex = 1023;
        ps->ps.cursorHint = turret->s.weapon + 4;
        ps->ps.cursorHintString = BG_GetWeaponDef(turret->s.weapon)->dropHintStringIndex;
    }
}

void __cdecl Player_SetVehicleDropHint(gentity_s *ent)
{
    gentity_s *v1; // eax
    gclient_s *ps; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 497, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 498, 0, "%s", "ent->client");
    ps = ent->client;
    if ((ps->ps.pm_flags & 0x100000) == 0)
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 502, 0, "%s", "ps->pm_flags & PMF_VEHICLE_ATTACHED");
    if (!EntHandle::isDefined(&ent->r.ownerNum))
        MyAssertHandler(".\\game_mp\\player_use_mp.cpp", 503, 0, "%s", "ent->r.ownerNum.isDefined()");
    ps->ps.cursorHintEntIndex = EntHandle::entnum(&ent->r.ownerNum);
    v1 = EntHandle::ent(&ent->r.ownerNum);
    v1->flags |= 0x200000u;
}

