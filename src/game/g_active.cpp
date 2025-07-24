#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include "g_main.h"
#include <universal/com_math.h>
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include <server/sv_game.h>
#include "actor_events.h"
#include "player_use.h"
#include "turret.h"
#include <client/cl_input.h>

void __cdecl P_DamageFeedback(gentity_s *player)
{
    gclient_s *client; // r31
    const float *v3; // r4
    int damage_blood; // r10
    signed int maxHealth; // r11
    int v6; // r10
    int v7; // r30
    __int64 v8; // r11
    double v9; // fp0
    int damageEvent; // r11
    float v11; // [sp+58h] [-58h] BYREF
    float v12; // [sp+5Ch] [-54h]
    float v13; // [sp+60h] [-50h]
    float v14[4]; // [sp+68h] [-48h] BYREF
    float v15[14]; // [sp+78h] [-38h] BYREF

    client = player->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 29, 0, "%s", "client");
    if (client->ps.pm_type < 5)
    {
        if (player_debugHealth->current.enabled
            && client->invulnerableActivated
            && level.time < client->invulnerableExpireTime)
        {
            G_GetPlayerViewDirection(player, v14, 0, 0);
            G_GetPlayerViewOrigin(&client->ps, &v11);
            v11 = (float)(v14[0] * (float)30.0) + v11;
            v12 = (float)(v14[1] * (float)30.0) + v12;
            v13 = (float)(v14[2] * (float)30.0) + v13;
            G_DebugCircle(&v11, 10.0, v3, (int)colorYellow, 0, 0);
        }
        client->ps.damageCount = 0;
        damage_blood = client->damage_blood;
        if (damage_blood > 0)
        {
            maxHealth = client->pers.maxHealth;
            if (maxHealth > 0)
            {
                v6 = 100 * damage_blood;
                v7 = v6 / maxHealth;
                //__twllei(maxHealth, 0);
                //HIDWORD(v8) = __ROL4__(v6, 1) - 1;
                //__twlgei(maxHealth & ~HIDWORD(v8), 0xFFFFFFFF);

                if (maxHealth < 0) {
                    maxHealth = 0;
                }
                unsigned int alignUnit = ((unsigned int)v6 << 1) | ((unsigned int)v6 >> 31);
                unsigned int alignMask = alignUnit - 1;
                maxHealth = maxHealth & ~alignMask;

                if (v7 > 127)
                    v7 = 127;
                LODWORD(v8) = v7;
                v9 = (float)((float)v8 + client->ps.aimSpreadScale);
                client->ps.aimSpreadScale = (float)v8 + client->ps.aimSpreadScale;
                if (v9 > 255.0)
                    client->ps.aimSpreadScale = 255.0;
                if (client->damage_fromWorld)
                {
                    client->damage_fromWorld = 0;
                    client->ps.damagePitch = 255;
                    client->ps.damageYaw = 255;
                }
                else
                {
                    vectoangles(client->damage_from, v15);
                    client->ps.damagePitch = (int)(float)(v15[0] * (float)0.71111113);
                    client->ps.damageYaw = (int)(float)(v15[1] * (float)0.71111113);
                }
                damageEvent = client->ps.damageEvent;
                client->ps.damageCount = v7;
                client->damage_blood = 0;
                client->ps.damageEvent = damageEvent + 1;
            }
        }
    }
}

void __cdecl G_SetClientSound(gentity_s *ent)
{
    ent->s.loopSound = 0;
}

void __cdecl G_TouchEnts(gentity_s *ent, int numtouch, int *touchents)
{
    int v6; // r30
    int *v7; // r28
    int v8; // r10
    int *v9; // r11
    gentity_s *v10; // r31
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // r11
    void(__cdecl * v12)(gentity_s *, gentity_s *, int); // r11

    v6 = 0;
    if (numtouch > 0)
    {
        v7 = touchents;
        do
        {
            v8 = 0;
            if (v6 > 0)
            {
                v9 = touchents;
                do
                {
                    if (*v9 == *v7)
                        break;
                    ++v8;
                    ++v9;
                } while (v8 < v6);
            }
            if (v8 == v6)
            {
                v10 = &g_entities[*v7];
                if (Scr_IsSystemActive())
                {
                    Scr_AddEntity(v10);
                    Scr_Notify(ent, scr_const.touch, 1u);
                    Scr_AddEntity(ent);
                    Scr_Notify(v10, scr_const.touch, 1u);
                }
                touch = entityHandlers[ent->handler].touch;
                if (touch)
                    touch(ent, v10, 1);
                v12 = entityHandlers[v10->handler].touch;
                if (v12)
                    v12(v10, ent, 1);
            }
            ++v6;
            ++v7;
        } while (v6 < numtouch);
    }
}

void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm)
{
    G_TouchEnts(ent, pm->numtouch, pm->touchents);
}

void __cdecl G_DoTouchTriggers(gentity_s *ent)
{
    int v2; // r20
    sentient_s *sentient; // r11
    team_t eTeam; // r11
    double v5; // fp12
    double v6; // fp13
    double v7; // fp12
    double v8; // fp13
    int v9; // r30
    double v10; // fp13
    double v11; // fp0
    double v12; // fp7
    double v13; // fp11
    double v14; // fp12
    double v15; // fp9
    double v16; // fp6
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // r19
    int *v18; // r21
    int i; // r17
    gentity_s *v20; // r30
    void(__cdecl * v21)(gentity_s *, gentity_s *, int); // r29
    gclient_s *client; // r3
    float v23; // [sp+50h] [-22A0h] BYREF
    float v24; // [sp+54h] [-229Ch]
    float v25; // [sp+58h] [-2298h]
    float v26; // [sp+60h] [-2290h] BYREF
    float v27; // [sp+64h] [-228Ch]
    float v28; // [sp+68h] [-2288h]
    int v29[160]; // [sp+70h] [-2280h] BYREF

    //Profile_Begin(355);
    if (ent->scr_vehicle)
    {
        v2 = 8;
    }
    else
    {
        sentient = ent->sentient;
        if (sentient)
        {
            if (ent->client)
            {
                v2 = 0x40000000;
            }
            else
            {
                eTeam = sentient->eTeam;
                switch (eTeam)
                {
                case TEAM_AXIS:
                    v2 = 0x40000;
                    break;
                case TEAM_ALLIES:
                    v2 = 0x80000;
                    break;
                case TEAM_NEUTRAL:
                    v2 = 0x100000;
                    break;
                default:
                    goto LABEL_34;
                }
            }
        }
        else
        {
            v2 = 0x400000;
        }
    }
    v5 = (float)(ent->r.absmin[1] - (float)20.0);
    v23 = ent->r.absmin[0] - (float)20.0;
    v6 = (float)(ent->r.absmin[2] - (float)20.0);
    v24 = v5;
    v7 = (float)(ent->r.absmax[0] + (float)20.0);
    v25 = v6;
    v8 = (float)(ent->r.absmax[1] + (float)20.0);
    v26 = v7;
    v28 = ent->r.absmax[2] + (float)20.0;
    v27 = v8;
    v9 = CM_AreaEntities(&v23, &v26, v29, 2176, v2);
    v10 = ent->r.currentOrigin[1];
    v11 = (float)(ent->r.maxs[0] + ent->r.currentOrigin[0]);
    v12 = ent->r.maxs[1];
    v13 = (float)(ent->r.mins[1] + ent->r.currentOrigin[1]);
    v23 = ent->r.mins[0] + ent->r.currentOrigin[0];
    v26 = v11;
    v14 = ent->r.currentOrigin[2];
    v15 = ent->r.mins[2];
    v16 = ent->r.maxs[2];
    v24 = v13;
    v27 = (float)v12 + (float)v10;
    v25 = (float)v15 + (float)v14;
    v28 = (float)v16 + (float)v14;
    ExpandBoundsToWidth(&v23, &v26);
    touch = entityHandlers[ent->handler].touch;
    if (v9 > 0)
    {
        v18 = v29;
        for (i = v9; i; --i)
        {
            v20 = &g_entities[*v18];
            if ((v20->r.contents & v2) == 0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 232, 0, "%s", "hit->r.contents & contents");
            if (v20->s.eType == 3)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 233, 0, "%s", "hit->s.eType != ET_MISSILE");
            v21 = entityHandlers[v20->handler].touch;
            if (!v21 && !touch)
                goto LABEL_33;
            if (v20->s.eType == 2)
            {
                client = ent->client;
                if (client && BG_PlayerTouchesItem(&client->ps, &v20->s, level.time))
                {
                LABEL_26:
                    if (Scr_IsSystemActive())
                    {
                        Scr_AddEntity(ent);
                        Scr_Notify(v20, scr_const.touch, 1u);
                        Scr_AddEntity(v20);
                        Scr_Notify(ent, scr_const.touch, 1u);
                    }
                    if (v21)
                        v21(v20, ent, 1);
                    if (ent->actor && touch)
                        touch(ent, v20, 1);
                }
            }
            else if (SV_EntityContact(&v23, &v26, v20))
            {
                goto LABEL_26;
            }
        LABEL_33:
            ++v18;
        }
    }
LABEL_34:
    //Profile_EndInternal(0);
}

void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex)
{
    WeaponDef *WeaponDef; // r29

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 281, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 282, 0, "%s", "ent->client");
    if (!weaponIndex)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 283, 0, "%s", "weaponIndex != WP_NONE");
    WeaponDef = BG_GetWeaponDef(weaponIndex);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 286, 0, "%s", "weapDef");
    Scr_AddString(WeaponDef->szInternalName);
    Scr_Notify(ent, scr_const.grenade_pullback, 1u);
}

bool __cdecl IsLiveGrenade(gentity_s *ent)
{
    WeaponDef *WeaponDef; // r31

    if (ent->s.eType != 3)
        return 0;
    WeaponDef = BG_GetWeaponDef(*(unsigned __int16 *)ent->s.index - ((int)*(unsigned __int16 *)ent->s.index >> 7 << 7));
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 301, 0, "%s", "weapDef");
    return WeaponDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE;
}

void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt)
{
    gentity_s *v2; // r31
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // r29

    if (!clientEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 315, 0, "%s", "clientEnt");
    if (!clientEnt->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 316, 0, "%s", "clientEnt->client");
    if (clientEnt->client->ps.cursorHintEntIndex >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp",
            317,
            0,
            "%s",
            "(unsigned)clientEnt->client->ps.cursorHintEntIndex < MAX_GENTITIES");
    v2 = &g_entities[clientEnt->client->ps.cursorHintEntIndex];
    if (IsLiveGrenade(v2))
    {
        if (clientEnt->client->ps.throwBackGrenadeTimeLeft)
        {
            touch = entityHandlers[v2->handler].touch;
            if (touch)
            {
                if (v2->parent.isDefined())
                    clientEnt->client->ps.throwBackGrenadeOwner = v2->parent.entnum();
                else
                    clientEnt->client->ps.throwBackGrenadeOwner = 2174;
                clientEnt->client->ps.grenadeTimeLeft = clientEnt->client->ps.throwBackGrenadeTimeLeft;
                touch(v2, clientEnt, 0);
                if (!clientEnt->client->ps.throwBackGrenadeTimeLeft)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp",
                        337,
                        0,
                        "%s",
                        "clientEnt->client->ps.throwBackGrenadeTimeLeft");
            }
        }
    }
}

void __cdecl ClientEvents(gentity_s *ent, int oldEventSequence)
{
    gclient_s *client; // r23
    int eventSequence; // r10
    int v5; // r22
    __int64 v6; // r11
    int v7; // r30
    int v8; // r29
    double v9; // fp0
    __int64 v10; // r11
    gclient_s *v11; // r11
    gclient_s *v12; // r11
    sentient_s *sentient; // r11
    team_t v14; // r3
    int v15; // r29
    int v16; // r4
    int v17; // [sp+8h] [-118h]
    hitLocation_t v18; // [sp+Ch] [-114h]
    unsigned int v19; // [sp+10h] [-110h]
    unsigned int v20; // [sp+14h] [-10Ch]
    float v21[4]; // [sp+88h] [-98h] BYREF

    client = ent->client;
    eventSequence = client->ps.eventSequence;
    if (oldEventSequence < eventSequence - 4)
        oldEventSequence = eventSequence - 4;
    v5 = oldEventSequence;
    if (oldEventSequence < eventSequence)
    {
        while (1)
        {
            HIDWORD(v6) = 4 * ((v5 & 3) + 37);
            v7 = *(int *)((char *)&client->ps.commandTime + HIDWORD(v6));
            v8 = client->ps.eventParms[v5 & 3];
            if ((unsigned int)(v7 - 115) > 0x1C)
            {
                switch (v7)
                {
                case 1:
                    goto LABEL_46;
                case 19:
                    if (!ent->client)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 423, 0, "%s", "ent->client");
                    Scr_Notify(ent, scr_const.reload_start, 0);
                    break;
                case 20:
                    if (!ent->client)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 428, 0, "%s", "ent->client");
                    Scr_Notify(ent, scr_const.reload, 0);
                    break;
                case 25:
                case 32:
                    if (!ent->client)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 406, 0, "%s", "ent->client");
                    NotifyGrenadePullback(ent, v8);
                    break;
                case 26:
                case 27:
                case 38:
                case 39:
                case 40:
                    FireWeapon(ent, 0);
                    break;
                case 31:
                    FireWeaponMelee(ent, 0);
                    break;
                case 33:
                    G_UseOffHand(ent);
                    break;
                case 34:
                    if (ent->client->ps.cursorHintEntIndex
                        && BG_GetWeaponDef(client->ps.eventParms[v5 & 3])->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
                    {
                        AttemptLiveGrenadePickup(ent);
                    }
                    break;
                case 66:
                    v11 = ent->client;
                    if (v11 && (ent->flags & 3) == 0)
                    {
                        ent->health = 0;
                        v11->ps.stats[0] = 0;
                        player_die(ent, ent, ent, 100000, 12, v8, 0, HITLOC_NONE);
                    }
                    break;
                case 67:
                    if (!ent->client)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 448, 0, "%s", "ent->client");
                    Scr_Notify(ent, scr_const.detonate, 0);
                    break;
                default:
                    if (v7 == 81 || v7 == 82 || v7 == 83 || v7 == 85 || v7 >= 86 && v7 <= 114)
                    {
                        v12 = ent->client;
                        if ((v12->ps.pm_flags & 2) == 0 && (v12->pers.cmd.buttons & 0x800) == 0)
                        {
                            sentient = ent->sentient;
                            if (sentient)
                            {
                                v14 = Sentient_EnemyTeam(sentient->eTeam);
                                if (v14)
                                {
                                    v15 = 1 << v14;
                                    Sentient_GetOrigin(ent->sentient, v21);
                                    v16 = 3;
                                    if (v7 != 83)
                                        v16 = 2;
                                    Actor_BroadcastPointEvent(ent, v16, v15, v21, 0.0);
                                }
                            }
                        }
                    }
                    break;
                }
                goto LABEL_46;
            }
            if (ent->s.eType != 1)
                return;
            if (v8 >= 100)
                break;
            LODWORD(v6) = client->ps.eventParms[v5 & 3];
            v9 = (float)((float)v6 * (float)0.0099999998);
            if (v9 != 0.0)
                goto LABEL_9;
        LABEL_46:
            if (++v5 >= client->ps.eventSequence)
                return;
        }
        v9 = 1.1;
    LABEL_9:
        LODWORD(v10) = client->ps.stats[2];
        HIDWORD(v10) = 11;
        G_Damage(ent, 0, 0, 0, 0, (int)(float)((float)v10 * (float)v9), 0, 11, v17, v18, v19, v20);
        goto LABEL_46;
    }
}

void __cdecl Client_ClaimNode(gentity_s *ent)
{
    pathnode_t *v2; // r29
    pathnode_t *pClaimedNode; // r11
    const sentient_s *v4; // r3
    float v5[12]; // [sp+50h] [-30h] BYREF

    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 503, 0, "%s", "ent->client");
    if (!ent->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 504, 0, "%s", "ent->sentient");
    v2 = Sentient_NearestNode(ent->sentient);
    Sentient_GetOrigin(ent->sentient, v5);
    if (v2 && Vec2DistanceSq(v2->constant.vOrigin, v5) > 1024.0)
        v2 = 0;
    pClaimedNode = ent->sentient->pClaimedNode;
    if (v2 != pClaimedNode)
    {
        if (pClaimedNode)
        {
            if (ent->client->pLookatEnt.isDefined()
                && ent->client->pLookatEnt.ent()->actor
                && !ent->client->pLookatEnt.ent()->sentient->pClaimedNode)
            {
                Path_RelinquishNodeSoon(ent->sentient);
            }
            else
            {
                Path_RelinquishNodeNow(ent->sentient);
            }
        }
        if (v2)
        {
            if (!v2->dynamic.pOwner.isDefined()
                || v2->dynamic.pOwner.sentient() != ent->sentient
                && (v4 = v2->dynamic.pOwner.sentient(),
                    Sentient_GetOrigin(v4, v5),
                    Vec2DistanceSq(v5, v2->constant.vOrigin) >= 225.0))
            {
                Path_ForceClaimNode(v2, ent->sentient);
            }
        }
    }
}

void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap)
{
    int entityEventSequence; // r10
    int eventSequence; // r11
    int v7; // r11
    int oldEventSequence; // r4
    int v9; // r10
    int i; // r11
    unsigned int weapon; // r11
    int groundEntityNum; // r11

    s->lerp.pos.trType = TR_LINEAR_STOP;
    s->lerp.pos.trBase[0] = ps->origin[0];
    s->lerp.pos.trBase[1] = ps->origin[1];
    s->lerp.pos.trBase[2] = ps->origin[2];
    s->lerp.pos.trDelta[0] = ps->velocity[0];
    s->lerp.pos.trDelta[1] = ps->velocity[1];
    s->lerp.pos.trDelta[2] = ps->velocity[2];
    s->lerp.pos.trTime = time;
    s->lerp.pos.trDuration = 50;
    s->lerp.apos.trType = TR_INTERPOLATE;
    s->lerp.apos.trBase[0] = ps->viewangles[0];
    s->lerp.apos.trBase[1] = ps->viewangles[1];
    s->lerp.apos.trBase[2] = ps->viewangles[2];
    s->lerp.eFlags = ps->eFlags;
    entityEventSequence = ps->entityEventSequence;
    eventSequence = ps->eventSequence;
    if (entityEventSequence - eventSequence >= 0)
    {
        s->eventParm = 0;
    }
    else
    {
        if (eventSequence - entityEventSequence > 4)
            ps->entityEventSequence = eventSequence - 4;
        s->eventParm = ps->eventParms[ps->entityEventSequence++ & 3];
    }
    v7 = ps->eventSequence;
    if (ps->oldEventSequence - v7 > 0)
        ps->oldEventSequence = v7;
    oldEventSequence = ps->oldEventSequence;
    if (oldEventSequence != v7)
    {
        do
        {
            v9 = 0;
            for (i = 6; i > 0; i = singleClientEvents[v9])
            {
                if (i == (unsigned __int8)ps->events[oldEventSequence & 3])
                    break;
                ++v9;
            }
            if (singleClientEvents[v9] < 0)
            {
                s->events[s->eventSequence & 3] = ps->events[oldEventSequence & 3];
                s->eventParms[s->eventSequence++ & 3] = ps->eventParms[oldEventSequence & 3];
            }
            ++oldEventSequence;
        } while (oldEventSequence != ps->eventSequence);
    }
    weapon = ps->weapon;
    ps->oldEventSequence = ps->eventSequence;
    s->weapon = weapon;
    groundEntityNum = ps->groundEntityNum;
    s->eType = 1;
    s->groundEntityNum = groundEntityNum;
}

// local variable allocation has failed, the output may be wrong!
void __cdecl ClientThink_real(gentity_s *ent, long double a2)
{
    gclient_s *client; // r30
    usercmd_s *p_cmd; // r25
    unsigned int v5; // r11
    int v6; // r11
    long double v7; // fp2
    int eventSequence; // r28
    double v9; // fp0
    int integer; // r11
    int clipmask; // r11
    gclient_s *v12; // r3
    double v13; // fp13
    double v14; // fp12
    double v15; // fp11
    double v16; // fp10
    double v17; // fp9
    gclient_s *v18; // r11
    int numtouch; // r4
    int time; // r11
    int *p_buttons; // r27
    int *p_oldbuttons; // r11
    int buttons; // r10
    int useButtonDone; // r8
    int v25; // r10
    int *p_buttonsSinceLastFrame; // r28
    int v27; // r11
    int *p_latched_buttons; // r26
    __int64 v29; // r10 OVERLAPPED
    int v30; // r11
    unsigned int v31; // r5
    unsigned __int16 confirm_location; // r4
    int v33; // r9
    int v34; // r10
    int v35; // r11
    float v36[4]; // [sp+50h] [-1A0h] BYREF
    __int64 v37; // [sp+60h] [-190h]
    __int64 v38; // [sp+68h] [-188h]
    pmove_t v39; // [sp+70h] [-180h] BYREF

    client = ent->client;
    if (client->pers.connected != CON_CONNECTED)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp",
            644,
            0,
            "%s",
            "client->pers.connected == CON_CONNECTED");
    p_cmd = &ent->client->pers.cmd;
    if (client->bFrozen || client->linkAnglesLocked)
        v5 = client->ps.pm_flags | 0x800;
    else
        v5 = client->ps.pm_flags & 0xFFFFF7FF;
    client->ps.pm_flags = v5;
    if (client->noclip)
    {
        v6 = 2;
    }
    else if (client->ufo)
    {
        v6 = 3;
    }
    else if (level.mpviewer)
    {
        v6 = 4;
    }
    else
    {
        //v6 = (_cntlzw((unsigned int)ent->tagInfo) & 0x20) == 0;
        v6 = (ent->tagInfo != 0);
        if (client->ps.stats[0] <= 0)
            v6 += 5;
    }
    client->ps.pm_type = (pmtype_t)v6;
    *(double *)&a2 = (float)(g_gravity->current.value + (float)0.5);
    v7 = floor(a2);
    eventSequence = client->ps.eventSequence;
    v9 = (float)(client->ps.aimSpreadScale * (float)0.0039215689);
    client->ps.gravity = (int)(float)*(double *)&v7;
    integer = g_speed->current.integer;
    client->currentAimSpreadScale = v9;
    client->ps.speed = integer;
    memset(&v39, 0, sizeof(v39));
    v39.ps = &client->ps;
    memcpy(&v39.cmd, p_cmd, sizeof(v39.cmd));
    memcpy(&v39.oldcmd, &client->pers.oldcmd, sizeof(v39.oldcmd));
    if (client->ps.pm_type < 5)
        clipmask = ent->clipmask;
    else
        clipmask = 8454161;
    v39.tracemask = clipmask;
    v39.handler = 1;
    //Profile_Begin(27);
    Pmove(&v39);
    //Profile_EndInternal(0);
    v12 = ent->client;
    if (v12->ps.eventSequence != eventSequence)
        ent->r.eventTime = level.time;
    G_PlayerStateToEntityStateExtrapolate(&v12->ps, &ent->s, v12->ps.commandTime, 1);
    ent->r.currentOrigin[0] = ent->s.lerp.pos.trBase[0];
    ent->r.currentOrigin[1] = ent->s.lerp.pos.trBase[1];
    ent->r.currentOrigin[2] = ent->s.lerp.pos.trBase[2];
    v13 = v39.mins[1];
    v14 = v39.mins[2];
    v15 = v39.maxs[0];
    v16 = v39.maxs[1];
    v17 = v39.maxs[2];
    ent->r.mins[0] = v39.mins[0];
    ent->r.mins[1] = v13;
    ent->r.mins[2] = v14;
    ent->r.maxs[0] = v15;
    ent->r.maxs[1] = v16;
    ent->r.maxs[2] = v17;
    ClientEvents(ent, eventSequence);
    SV_LinkEntity(ent);
    v18 = ent->client;
    numtouch = v39.numtouch;
    ent->r.currentOrigin[0] = v18->ps.origin[0];
    ent->r.currentOrigin[1] = v18->ps.origin[1];
    ent->r.currentOrigin[2] = v18->ps.origin[2];
    ent->r.currentAngles[0] = 0.0;
    ent->r.currentAngles[1] = 0.0;
    ent->r.currentAngles[2] = 0.0;
    ent->r.currentAngles[1] = ent->client->ps.viewangles[1];
    G_TouchEnts(ent, numtouch, v39.touchents);
    time = level.time;
    if (level.time >= client->lastTouchTime + 500)
    {
        client->inControlTime = level.time;
        time = level.time;
    }
    if (ent->client->ps.eventSequence != eventSequence)
        ent->r.eventTime = time;
    p_buttons = &client->buttons;
    p_oldbuttons = &client->oldbuttons;
    buttons = client->buttons;
    useButtonDone = client->useButtonDone;
    client->oldbuttons = buttons;
    if (!useButtonDone)
        *p_oldbuttons = buttons & 0xFFFFFFD7;
    v25 = p_cmd->buttons;
    *p_buttons = v25;
    if ((v25 & 0x28) == 0)
        client->useButtonDone = 0;
    p_buttonsSinceLastFrame = &client->buttonsSinceLastFrame;
    HIDWORD(v29) = client->ps.locationSelectionInfo;
    v27 = *p_buttons & ~*p_oldbuttons;
    p_latched_buttons = &client->latched_buttons;
    LODWORD(v29) = client->buttonsSinceLastFrame | v27;
    client->latched_buttons = v27;
    client->buttonsSinceLastFrame = v29;
    if (HIDWORD(v29))
    {
        if ((v29 & 0x10000) != 0)
        {
            LOBYTE(v29) = p_cmd->selectedLocation[1];
            v30 = p_cmd->selectedLocation[0];
            v36[2] = 0.0;
            LODWORD(v29) = (char)v29;
            v38 = *(__int64 *)((char *)&v29 - 4);
            v37 = v29;
            v36[0] = -(float)((float)(level.compassNorth[0]
                * (float)(level.compassMapWorldSize[1]
                    * (float)((float)((float)v29 + (float)128.0) * (float)0.0039215689)))
                - (float)((float)(level.compassNorth[1]
                    * (float)(level.compassMapWorldSize[0]
                        * (float)((float)((float)*(__int64 *)((char *)&v29 - 4) + (float)128.0)
                            * (float)0.0039215689)))
                    + level.compassMapUpperLeft[0]));
            v36[1] = -(float)((float)(level.compassNorth[1]
                * (float)(level.compassMapWorldSize[1]
                    * (float)((float)((float)v29 + (float)128.0) * (float)0.0039215689)))
                - (float)-(float)((float)(level.compassNorth[0]
                    * (float)(level.compassMapWorldSize[0]
                        * (float)((float)((float)*(__int64 *)((char *)&v29 - 4)
                            + (float)128.0)
                            * (float)0.0039215689)))
                    - level.compassMapUpperLeft[1]));
            Scr_AddVector(v36);
            v31 = 1;
            confirm_location = scr_const.confirm_location;
        }
        else
        {
            if ((v29 & 0x20000) == 0)
            {
            LABEL_34:
                v33 = *p_buttonsSinceLastFrame;
                v34 = *p_latched_buttons & 0x1300;
                *p_buttons &= 0x1300u;
                *p_latched_buttons = v34;
                *p_buttonsSinceLastFrame = v33 & 0x1300;
                goto LABEL_35;
            }
            v31 = 0;
            confirm_location = scr_const.cancel_location;
        }
        Scr_Notify(ent, confirm_location, v31);
        goto LABEL_34;
    }
LABEL_35:
    client->fGunPitch = p_cmd->gunPitch;
    client->fGunYaw = p_cmd->gunYaw;
    client->fGunXOfs = p_cmd->gunXOfs;
    client->fGunYOfs = p_cmd->gunYOfs;
    client->fGunZOfs = p_cmd->gunZOfs;
    Player_UpdateActivate(ent);
    v35 = level.time;
    if (client->ps.shellshockDuration + client->ps.shellshockTime < level.time)
    {
        client->ps.pm_flags &= ~0x10000u;
        v35 = level.time;
    }
    if (client->ps.pm_type >= 5 && v35 > client->respawnTime)
        respawn(ent);
}

void __cdecl ClientThink(int clientNum)
{
    gentity_s *v2; // r31
    long double v3; // fp2

    v2 = &g_entities[clientNum];
    memcpy(&v2->client->pers.oldcmd, &v2->client->pers.cmd, sizeof(v2->client->pers.oldcmd));
    SV_GetUsercmd(clientNum, &v2->client->pers.cmd);
    ClientThink_real(v2, v3);
    if (v2->client->ps.leanf == 0.0)
        G_RemoveHeadHitEnt(v2);
    else
        G_UpdateHeadHitEnt(v2);
}

void __cdecl ClientEndFrame(gentity_s *ent)
{
    gclient_s *client; // r11
    gclient_s *v3; // r11
    gclient_s *v5; // r11
    gclient_s *v6; // r11
    int pm_type; // r10
    gclient_s *v8; // r3

    client = ent->client;
    if ((client->ps.eFlags & 0x300) != 0)
    {
        if (client->ps.viewlocked_entNum == 2175)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp",
                867,
                0,
                "%s",
                "ent->client->ps.viewlocked_entNum != ENTITYNUM_NONE");
        turret_think_client(&level.gentities[ent->client->ps.viewlocked_entNum]);
    }
    v3 = ent->client;
    if (!v3->noclip && !v3->ufo && !level.mpviewer)
    {
        if (ent->tagInfo)
        {
            v3->ps.pm_type = PM_DEAD_LINKED;
            if (v3->ps.stats[0] > 0)
            {
                v3->ps.pm_type = PM_NORMAL_LINKED;
            }
            G_SetPlayerFixedLink(ent);
            v5 = ent->client;
            v5->ps.origin[0] = ent->r.currentOrigin[0];
            v5->ps.origin[1] = ent->r.currentOrigin[1];
            v5->ps.origin[2] = ent->r.currentOrigin[2];
        }
        else
        {
            v3->prevLinkAnglesSet = 0;
            v6 = ent->client;
            pm_type = v6->ps.pm_type;
            
            if (pm_type == 1 || pm_type == 6)
            {
                --v6->ps.pm_type;
            }
        }
        G_UpdateGroundTilt(ent->client);
    }
    if (!ent->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 913, 0, "%s", "ent->sentient");
    Sentient_UpdateActualChainPos(ent->sentient);
    if (ent->client->ps.commandTime && level.time > 500)
    {
        Player_UpdateLookAtEntity(ent);
        Player_UpdateCursorHints(ent);
    }
    P_DamageFeedback(ent);
    ent->client->ps.moveSpeedScaleMultiplier = ent->client->pers.moveSpeedScaleMultiplier;
    ent->client->ps.stats[0] = ent->health;
    v8 = ent->client;
    ent->s.loopSound = 0;
    G_PlayerStateToEntityStateExtrapolate(&v8->ps, &ent->s, v8->ps.commandTime, 1);
    ent->client->buttonsSinceLastFrame = 0;
}

gentity_s *__cdecl G_GetPlayer()
{
    return level.gentities;
}

void __cdecl G_UpdatePlayer(gentity_s *ent)
{
    int v2; // r3

    if (ent->r.inuse)
    {
        v2 = ++level.framePos;
        if (level.demoplaying)
            level.demoplaying = SV_ReadPacket(v2);
        else
            CL_WritePacket();
        Sentient_UpdatePlayerTrail(ent);
    }
}

void __cdecl G_UpdatePlayerTriggers(gentity_s *ent)
{
    sentient_s *sentient; // r11
    int v3; // r10
    sentient_s *v4; // r11
    gclient_s *client; // r11

    if (ent->r.inuse)
    {
        if (!ent->client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_active.cpp", 1012, 0, "%s", "ent->client");
        ent->client->playerMoved = Vec2DistanceSq(ent->r.currentOrigin, ent->sentient->oldOrigin) >= 0.0099999998;
        sentient = ent->sentient;
        if (ent->r.currentOrigin[0] != sentient->oldOrigin[0]
            || ent->r.currentOrigin[1] != sentient->oldOrigin[1]
            || (v3 = 1, ent->r.currentOrigin[2] != sentient->oldOrigin[2]))
        {
            v3 = 0;
        }
        v4 = ent->sentient;
        v4->oldOrigin[0] = ent->r.currentOrigin[0];
        v4->oldOrigin[1] = ent->r.currentOrigin[1];
        v4->oldOrigin[2] = ent->r.currentOrigin[2];
        if (Sentient_NearestNodeDirty(ent->sentient, v3 == 0))
            Sentient_InvalidateNearestNode(ent->sentient);
        Client_ClaimNode(ent);
        Sentient_BanNearNodes(ent->sentient);
        client = ent->client;
        if (!client->noclip && !client->ufo && client->ps.pm_type < 5 && !level.mpviewer)
            G_DoTouchTriggers(ent);
    }
}

