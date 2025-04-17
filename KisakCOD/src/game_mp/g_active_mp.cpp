#include "g_public_mp.h"

#include <game_mp/g_utils_mp.h>

#include <script/scr_const.h>
#include <script/scr_vm.h>

#include <server/sv_game.h>
#include <server/sv_world.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <xanim/xanim.h>
#include <universal/profile.h>
#include <qcommon/threads.h>
#include <client/client.h>

void __cdecl P_DamageFeedback(gentity_s *player)
{
    gclient_s *client; // [esp+8h] [ebp-40h]
    int damage; // [esp+Ch] [ebp-3Ch]
    int damagea; // [esp+Ch] [ebp-3Ch]
    float kick; // [esp+10h] [ebp-38h]
    float angles[3]; // [esp+14h] [ebp-34h] BYREF
    float viewaxis[3][3]; // [esp+20h] [ebp-28h] BYREF
    int DAMAGE_COUNT_DURATION; // [esp+44h] [ebp-4h]

    DAMAGE_COUNT_DURATION = 500;
    client = player->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 41, 0, "%s", "client");
    if (client->ps.pm_type < 7)
    {
        if (level.time - client->damageTime > 500)
            client->ps.damageCount = 0;
        damage = client->damage_blood;
        if (damage > 0 && client->sess.maxHealth > 0)
        {
            damagea = 100 * damage / client->sess.maxHealth;
            if (damagea > 127)
                damagea = 127;
            client->ps.aimSpreadScale = (double)damagea + client->ps.aimSpreadScale;
            if (client->ps.aimSpreadScale > 255.0)
                client->ps.aimSpreadScale = 255.0;
            kick = (double)damagea * bg_viewKickScale->current.value;
            if (bg_viewKickMin->current.value <= (double)kick)
            {
                if (bg_viewKickMax->current.value < (double)kick)
                    kick = bg_viewKickMax->current.value;
            }
            else
            {
                kick = bg_viewKickMin->current.value;
            }
            if (client->damage_fromWorld)
            {
                client->v_dmg_roll = 0.0;
                client->v_dmg_pitch = -kick;
                client->ps.damagePitch = 255;
                client->ps.damageYaw = 255;
                client->damage_fromWorld = 0;
            }
            else
            {
                vectoangles(client->damage_from, angles);
                AnglesToAxis(client->ps.viewangles, viewaxis);
                client->v_dmg_roll = Vec3Dot(client->damage_from, viewaxis[1]) * -kick;
                client->v_dmg_pitch = Vec3Dot(client->damage_from, viewaxis[0]) * kick;
                client->ps.damagePitch = (int)(angles[0] / 360.0 * 256.0);
                client->ps.damageYaw = (int)(angles[1] / 360.0 * 256.0);
            }
            ++client->ps.damageEvent;
            client->damageTime = level.time - 20;
            client->ps.damageCount = damagea;
            client->damage_blood = 0;
        }
    }
}

void __cdecl G_SetClientSound(gentity_s *ent)
{
    ent->s.loopSound = 0;
}

void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm)
{
    int j; // [esp+4h] [ebp-14h]
    void(__cdecl * entTouch)(gentity_s *, gentity_s *, int); // [esp+8h] [ebp-10h]
    gentity_s *other; // [esp+Ch] [ebp-Ch]
    void(__cdecl * otherTouch)(gentity_s *, gentity_s *, int); // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    entTouch = entityHandlers[ent->handler].touch;
    for (i = 0; i < pm->numtouch; ++i)
    {
        for (j = 0; j < i && pm->touchents[j] != pm->touchents[i]; ++j)
            ;
        if (j == i)
        {
            other = &g_entities[pm->touchents[i]];
            if (Scr_IsSystemActive())
            {
                Scr_AddEntity(other);
                Scr_Notify(ent, scr_const.touch, 1u);
                Scr_AddEntity(ent);
                Scr_Notify(other, scr_const.touch, 1u);
            }
            otherTouch = entityHandlers[other->handler].touch;
            if (otherTouch)
                otherTouch(other, ent, 1);
            if (entTouch)
                entTouch(ent, other, 1);
        }
    }
}

const float range[3] = { 20.0f, 20.0f, 20.0f };
void __cdecl G_TouchTriggers(gentity_s *ent)
{
    int entityList[1025]; // [esp+30h] [ebp-1030h] BYREF
    float diff[3]; // [esp+1034h] [ebp-2Ch] BYREF
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+1040h] [ebp-20h]
    entityState_s *item; // [esp+1044h] [ebp-1Ch]
    float sum[3]; // [esp+1048h] [ebp-18h] BYREF
    void(__cdecl * v6)(gentity_s *, gentity_s *, int); // [esp+1054h] [ebp-Ch]
    int v7; // [esp+1058h] [ebp-8h]
    int i; // [esp+105Ch] [ebp-4h]

    Profile_Begin(350);
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 191, 0, "%s", "ent->client");
    if (ent->client->ps.pm_type > 1)
    {
        Profile_EndInternal(0);
        return;
    }
    Vec3Sub(ent->r.absmin, range, diff);
    Vec3Add(ent->r.absmax, range, sum);
    v7 = CM_AreaEntities(diff, sum, entityList, 1024, 1079771144);
    Vec3Add(ent->client->ps.origin, ent->r.mins, diff);
    Vec3Add(ent->client->ps.origin, ent->r.maxs, sum);
    ExpandBoundsToWidth(diff, sum);
    touch = entityHandlers[ent->handler].touch;
    for (i = 0; i < v7; ++i)
    {
        item = &g_entities[entityList[i]].s;
        if ((LODWORD(item[1].lerp.pos.trDelta[2]) & 0x405C0008) == 0)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 215, 0, "%s", "hit->r.contents & MASK_TRIGGER");
        if (item->eType == 4)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 216, 0, "%s", "hit->s.eType != ET_MISSILE");
        v6 = entityHandlers[BYTE2(item[1].attackerEntityNum)].touch;
        if (v6 || touch)
        {
            if (item->eType == 3)
            {
                if (!BG_PlayerTouchesItem(&ent->client->ps, item, level.time))
                    continue;
            }
            else if (!SV_EntityContact(diff, sum, (const gentity_s *)item))
            {
                continue;
            }
            if (Scr_IsSystemActive())
            {
                Scr_AddEntity(ent);
                Scr_Notify((gentity_s *)item, scr_const.touch, 1u);
                Scr_AddEntity((gentity_s *)item);
                Scr_Notify(ent, scr_const.touch, 1u);
            }
            if (v6)
                v6((gentity_s *)item, ent, 1);
        }
    }
    Profile_EndInternal(0);
}

void __cdecl SpectatorThink(gentity_s *ent, usercmd_s *ucmd)
{
    gclient_s *client; // [esp+10h] [ebp-11Ch]
    pmove_t pm; // [esp+14h] [ebp-118h] BYREF

    client = ent->client;
    client->oldbuttons = client->buttons;
    client->buttons = client->sess.cmd.buttons;
    client->buttonsSinceLastFrame |= client->buttons & ~client->oldbuttons;
    if (client->sess.forceSpectatorClient < 0
        && G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS)
        && client->spectatorClient >= 0
        && (client->buttons & 4) != (client->oldbuttons & 4))
    {
        StopFollowing(ent);
    }
    if ((client->buttons & 1) == 0 || (client->oldbuttons & 1) != 0)
    {
        if ((client->buttons & 0x800) != 0 && (client->oldbuttons & 0x800) == 0)
            Cmd_FollowCycle_f(ent, -1);
    }
    else
    {
        Cmd_FollowCycle_f(ent, 1);
    }
    if ((client->ps.otherFlags & 2) == 0)
    {
        client->ps.pm_type = 4;
        if (G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS))
            client->ps.speed = 400;
        else
            client->ps.speed = 0;
        memset((unsigned __int8 *)&pm, 0, sizeof(pm));
        pm.ps = &client->ps;
        memcpy(&pm.cmd, ucmd, sizeof(pm.cmd));
        pm.tracemask = 0x800811;
        pm.handler = 1;
        Pmove(&pm);
        ent->r.currentOrigin[0] = client->ps.origin[0];
        ent->r.currentOrigin[1] = client->ps.origin[1];
        ent->r.currentOrigin[2] = client->ps.origin[2];
        SV_UnlinkEntity(ent);
    }
}

int __cdecl ClientInactivityTimer(gclient_s *client)
{
    const char *v2; // eax

    if (g_inactivity->current.integer)
    {
        if (client->sess.cmd.forwardmove || client->sess.cmd.rightmove || (client->sess.cmd.buttons & 0x401) != 0)
        {
            client->inactivityTime = level.time + 1000 * g_inactivity->current.integer;
            client->inactivityWarning = 0;
        }
        else if (!client->sess.localClient)
        {
            if (level.time > client->inactivityTime)
            {
                SV_GameDropClient(client - level.clients, "GAME_DROPPEDFORINACTIVITY");
                return 0;
            }
            if (level.time > client->inactivityTime - 10000 && !client->inactivityWarning)
            {
                client->inactivityWarning = 1;
                v2 = va("%c \"GAME_INACTIVEDROPWARNING\"", 99);
                SV_GameSendServerCommand(client - level.clients, SV_CMD_CAN_IGNORE, v2);
            }
        }
    }
    else
    {
        client->inactivityTime = level.time + 60000;
        client->inactivityWarning = 0;
    }
    return 1;
}

void __cdecl ClientIntermissionThink(gentity_s *ent)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    client = ent->client;
    client->oldbuttons = client->buttons;
    client->buttons = client->sess.cmd.buttons;
    client->buttonsSinceLastFrame |= client->buttons & ~client->oldbuttons;
}

void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 376, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 377, 0, "%s", "ent->client");
    if (!weaponIndex)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 378, 0, "%s", "weaponIndex != WP_NONE");
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (!weapDef)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 381, 0, "%s", "weapDef");
    Scr_AddString((char *)weapDef->szInternalName);
    Scr_Notify(ent, scr_const.grenade_pullback, 1u);
}

void __cdecl HandleClientEvent(gclient_s *client, gentity_s *ent, int event, int eventParm)
{
    gentity_s *attacker; // [esp+4h] [ebp-14h]
    float damage; // [esp+8h] [ebp-10h]
    float damagea; // [esp+8h] [ebp-10h]

    switch (event)
    {
    case 19:
        if (!ent->client)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 505, 0, "%s", "ent->client");
        Scr_Notify(ent, scr_const.reload_start, 0);
        break;
    case 20:
        if (!ent->client)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 510, 0, "%s", "ent->client");
        Scr_Notify(ent, scr_const.reload, 0);
        break;
    case 25:
    case 32:
        if (!ent->client)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 467, 0, "%s", "ent->client");
        NotifyGrenadePullback(ent, eventParm);
        break;
    case 26:
    case 27:
    case 38:
        if (g_antilag->current.enabled)
            FireWeapon(ent, client->lastServerTime);
        else
            FireWeapon(ent, level.time);
        break;
    case 31:
        if (g_antilag->current.enabled)
            FireWeaponMelee(ent, client->lastServerTime);
        else
            FireWeaponMelee(ent, level.time);
        break;
    case 33:
        G_UseOffHand(ent);
        break;
    case 34:
        if (ent->client->ps.cursorHintEntIndex && BG_GetWeaponDef(eventParm)->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
            AttemptLiveGrenadePickup(ent);
        break;
    case 62:
        if (ent->client && (ent->flags & 3) == 0)
        {
            ent->health = 0;
            ent->client->ps.stats[0] = 0;
            if (ent->client->ps.throwBackGrenadeOwner == 1023)
            {
                player_die(ent, ent, ent, 100000, 12, eventParm, 0, HITLOC_NONE, 0);
            }
            else
            {
                attacker = &g_entities[ent->client->ps.throwBackGrenadeOwner];
                player_die(ent, attacker, attacker, 100000, 3, eventParm, 0, HITLOC_NONE, 0);
            }
        }
        break;
    case 63:
        if (!ent->client)
            MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 515, 0, "%s", "ent->client");
        Scr_Notify(ent, scr_const.detonate, 0);
        break;
    default:
        if (event >= 106 && event <= 134 && ent->s.eType == 1)
        {
            damage = eventParm < 100 ? (double)eventParm * 0.009999999776482582 : 1.1;
            if (damage != 0.0)
            {
                damagea = (double)client->ps.stats[2] * damage;
                G_Damage(ent, 0, 0, 0, 0, (int)damagea, 0, 11, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
            }
        }
        break;
    }
}

void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt)
{
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+0h] [ebp-8h]
    gentity_s *grenadeEnt; // [esp+4h] [ebp-4h]

    if (!clientEnt)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 410, 0, "%s", "clientEnt");
    if (!clientEnt->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 411, 0, "%s", "clientEnt->client");
    if (clientEnt->client->ps.cursorHintEntIndex >= 0x400u)
        MyAssertHandler(
            ".\\game_mp\\g_active_mp.cpp",
            412,
            0,
            "%s",
            "(unsigned)clientEnt->client->ps.cursorHintEntIndex < MAX_GENTITIES");
    grenadeEnt = &g_entities[clientEnt->client->ps.cursorHintEntIndex];
    if (IsLiveGrenade(grenadeEnt))
    {
        if (clientEnt->client->ps.throwBackGrenadeTimeLeft)
        {
            touch = entityHandlers[grenadeEnt->handler].touch;
            if (touch)
            {
                if (EntHandle::isDefined(&grenadeEnt->parent))
                    clientEnt->client->ps.throwBackGrenadeOwner = EntHandle::entnum(&grenadeEnt->parent);
                else
                    clientEnt->client->ps.throwBackGrenadeOwner = 1022;
                clientEnt->client->ps.grenadeTimeLeft = clientEnt->client->ps.throwBackGrenadeTimeLeft;
                touch(grenadeEnt, clientEnt, 0);
                if (!clientEnt->client->ps.throwBackGrenadeTimeLeft)
                    MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 432, 0, "%s", "clientEnt->client->ps.throwBackGrenadeTimeLeft");
            }
        }
    }
}

bool __cdecl IsLiveGrenade(gentity_s *ent)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (ent->s.eType != 4)
        return 0;
    weapDef = BG_GetWeaponDef(ent->s.index.brushmodel % 128);
    if (!weapDef)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 396, 0, "%s", "weapDef");
    return weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE;
}

void __cdecl ClientEvents(gentity_s *ent, int oldEventSequence)
{
    gclient_s *client; // [esp+0h] [ebp-10h]
    int i; // [esp+Ch] [ebp-4h]

    client = ent->client;
    if (oldEventSequence < client->ps.eventSequence - 4)
        oldEventSequence = client->ps.eventSequence - 4;
    if (oldEventSequence > client->ps.eventSequence + 64)
        oldEventSequence -= 256;
    for (i = oldEventSequence; i < client->ps.eventSequence; ++i)
        HandleClientEvent(client, ent, client->ps.events[i & 3], client->ps.eventParms[i & 3]);
}

void __cdecl G_SetLastServerTime(int clientNum, int lastServerTime)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = &g_entities[clientNum];
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 579, 0, "%s", "ent->client");
    if (level.time - lastServerTime > 1000)
        lastServerTime = level.time - 1000;
    if (lastServerTime >= ent->client->lastServerTime || level.time <= lastServerTime)
        ent->client->lastServerTime = lastServerTime;
}

void __cdecl G_SetClientContents(gentity_s *pEnt)
{
    if (!pEnt->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 601, 0, "%s", "pEnt->client");
    if (pEnt->client->noclip)
    {
        pEnt->r.contents = 0;
    }
    else if (pEnt->client->ufo)
    {
        pEnt->r.contents = 0;
    }
    else if (pEnt->client->sess.sessionState == SESS_STATE_DEAD)
    {
        pEnt->r.contents = 0;
    }
    else
    {
        pEnt->r.contents = 0x2000000;
    }
}

void __cdecl ClientThink_real(gentity_s *ent, usercmd_s *ucmd)
{
    float *v2; // [esp+50h] [ebp-2FCh]
    float *vGunSpeed; // [esp+B4h] [ebp-298h]
    float *vGunOffset; // [esp+B8h] [ebp-294h]
    float *vLastMoveAng; // [esp+BCh] [ebp-290h]
    float *oldOrigin; // [esp+D0h] [ebp-27Ch]
    float *origin; // [esp+D4h] [ebp-278h]
    float loc2d; // [esp+100h] [ebp-24Ch]
    float loc2da; // [esp+100h] [ebp-24Ch]
    float loc2d_4; // [esp+104h] [ebp-248h]
    float loc2d_4a; // [esp+104h] [ebp-248h]
    float loc[3]; // [esp+108h] [ebp-244h] BYREF
    gclient_s *client; // [esp+114h] [ebp-238h]
    float vAxis3[3][3]; // [esp+118h] [ebp-234h] BYREF
    weaponState_t ws; // [esp+13Ch] [ebp-210h] BYREF
    float ssScale; // [esp+198h] [ebp-1B4h]
    int msec; // [esp+19Ch] [ebp-1B0h]
    int ssDT; // [esp+1A0h] [ebp-1ACh]
    int oldEventSequence; // [esp+1A4h] [ebp-1A8h]
    float vAxis2[3][3]; // [esp+1A8h] [ebp-1A4h] BYREF
    pmove_t pm; // [esp+1CCh] [ebp-180h] BYREF
    float angles[3]; // [esp+2E4h] [ebp-68h] BYREF
    float viewangles[3]; // [esp+2F0h] [ebp-5Ch] BYREF
    viewState_t vs; // [esp+2FCh] [ebp-50h] BYREF
    WeaponDef *weapDef; // [esp+320h] [ebp-2Ch]
    float vAxis[3][3]; // [esp+324h] [ebp-28h] BYREF
    float ssSwayScale; // [esp+348h] [ebp-4h]

    client = ent->client;
    if (client->sess.connected == CON_CONNECTED)
    {
        if (ucmd->serverTime > level.time + 200)
            ucmd->serverTime = level.time + 200;
        if (ucmd->serverTime < level.time - 1000)
            ucmd->serverTime = level.time - 1000;
        msec = ucmd->serverTime - client->ps.commandTime;
        if (msec >= 1 || client->ps.clientNum != ent - g_entities)
        {
            if (msec > 200)
                msec = 200;
            if (client->sess.sessionState == SESS_STATE_INTERMISSION)
            {
                //Profile_Begin(268);
                ClientIntermissionThink(ent);
                //Profile_EndInternal(0);
            }
            else if (client->sess.sessionState == SESS_STATE_SPECTATOR)
            {
                //Profile_Begin(269);
                SpectatorThink(ent, ucmd);
                //Profile_EndInternal(0);
            }
            else if (ClientInactivityTimer(client))
            {
                client->oldbuttons = client->buttons;
                if (!client->useButtonDone)
                    client->oldbuttons &= 0xFFFFFFD7;
                client->buttons = client->sess.cmd.buttons;
                if ((client->buttons & 0x28) == 0)
                    client->useButtonDone = 0;
                client->latched_buttons = client->buttons & ~client->oldbuttons;
                client->buttonsSinceLastFrame |= client->latched_buttons;
                if (client->ps.locationSelectionInfo)
                {
                    if ((client->buttonsSinceLastFrame & 0x10000) != 0)
                    {
                        loc2d = ((double)ucmd->selectedLocation[0] + 128.0) / 255.0;
                        loc2d_4 = ((double)ucmd->selectedLocation[1] + 128.0) / 255.0;
                        loc2da = loc2d * level.compassMapWorldSize[0];
                        loc2d_4a = loc2d_4 * level.compassMapWorldSize[1];
                        loc[0] = loc2da * level.compassNorth[1] + level.compassMapUpperLeft[0] - loc2d_4a * level.compassNorth[0];
                        loc[1] = level.compassMapUpperLeft[1] - loc2da * level.compassNorth[0] - loc2d_4a * level.compassNorth[1];
                        loc[2] = 0.0;
                        Scr_AddVector(loc);
                        Scr_Notify(ent, scr_const.confirm_location, 1u);
                    }
                    else if ((client->buttonsSinceLastFrame & 0x20000) != 0)
                    {
                        Scr_Notify(ent, scr_const.cancel_location, 0);
                    }
                    client->buttons &= 0x1300u;
                    client->latched_buttons &= 0x1300u;
                    client->buttonsSinceLastFrame &= 0x1300u;
                }
                oldEventSequence = client->ps.eventSequence;
                memset((unsigned __int8 *)&pm, 0, sizeof(pm));
                pm.ps = &client->ps;
                memcpy(&pm.cmd, ucmd, sizeof(pm.cmd));
                memcpy(&pm.oldcmd, &client->sess.oldcmd, sizeof(pm.oldcmd));
                if (client->ps.pm_type < 7)
                    pm.tracemask = 0x2810011;
                else
                    pm.tracemask = 0x810011;
                pm.handler = 1;
                oldOrigin = client->oldOrigin;
                origin = client->ps.origin;
                client->oldOrigin[0] = client->ps.origin[0];
                oldOrigin[1] = origin[1];
                oldOrigin[2] = origin[2];
                vs.ps = &client->ps;
                vs.damageTime = client->damageTime;
                vs.time = level.time;
                vs.v_dmg_pitch = client->v_dmg_pitch;
                vs.v_dmg_roll = client->v_dmg_roll;
                vs.xyspeed = BG_GetSpeed(&client->ps, level.time);
                vs.frametime = (double)msec * 0.001000000047497451;
                vs.fLastIdleFactor = client->fLastIdleFactor;
                vs.weapIdleTime = &client->weapIdleTime;
                client->ps.speed = g_speed->current.integer;
                BG_CalculateViewAngles(&vs, angles);
                Vec3Add(client->ps.viewangles, angles, viewangles);
                weapDef = BG_GetWeaponDef(client->ps.weapon);
                ssDT = client->ps.shellshockDuration + client->ps.shellshockTime - vs.time;
                if (ssDT <= 0)
                {
                    ssSwayScale = 1.0;
                    client->ps.pm_flags &= ~0x10000u;
                }
                else
                {
                    ssScale = 1.0;
                    if (ssDT < 3000)
                        ssScale = (double)ssDT / 3000.0;
                    ssScale = (3.0 - ssScale * 2.0) * ssScale * ssScale;
                    ssSwayScale = (weapDef->swayShellShockScale - 1.0) * ssScale + 1.0;
                }
                BG_CalculateWeaponPosition_Sway(
                    &client->ps,
                    client->swayViewAngles,
                    client->swayOffset,
                    client->swayAngles,
                    1.0,
                    msec);
                ws.ps = vs.ps;
                ws.xyspeed = vs.xyspeed;
                ws.frametime = (double)msec * 0.001000000047497451;
                ws.vLastMoveAng[0] = client->vLastMoveAng[0];
                ws.vLastMoveAng[1] = client->vLastMoveAng[1];
                ws.vLastMoveAng[2] = client->vLastMoveAng[2];
                ws.fLastIdleFactor = client->fLastIdleFactor;
                ws.time = vs.time;
                ws.damageTime = client->damageTime;
                ws.v_dmg_pitch = client->v_dmg_pitch;
                ws.v_dmg_roll = client->v_dmg_roll;
                ws.vGunOffset[0] = client->vGunOffset[0];
                ws.vGunOffset[1] = client->vGunOffset[1];
                ws.vGunOffset[2] = client->vGunOffset[2];
                ws.vGunSpeed[0] = client->vGunSpeed[0];
                ws.vGunSpeed[1] = client->vGunSpeed[1];
                ws.vGunSpeed[2] = client->vGunSpeed[2];
                ws.swayAngles[0] = client->swayAngles[0];
                ws.swayAngles[1] = client->swayAngles[1];
                ws.swayAngles[2] = client->swayAngles[2];
                ws.weapIdleTime = &client->weapIdleTime;
                BG_CalculateWeaponAngles(&ws, angles);
                if (BG_IsAimDownSightWeapon(ws.ps->weapon)
                    && ws.ps->fWeaponPosFrac != 0.0
                    && weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE)
                {
                    AnglesToAxis(angles, vAxis);
                    AnglesToAxis(viewangles, vAxis2);
                    MatrixMultiply(vAxis, vAxis2, vAxis3);
                    AxisToAngles(vAxis3, viewangles);
                }
                vLastMoveAng = client->vLastMoveAng;
                client->vLastMoveAng[0] = ws.vLastMoveAng[0];
                vLastMoveAng[1] = ws.vLastMoveAng[1];
                vLastMoveAng[2] = ws.vLastMoveAng[2];
                client->fLastIdleFactor = ws.fLastIdleFactor;
                vGunOffset = client->vGunOffset;
                client->vGunOffset[0] = ws.vGunOffset[0];
                vGunOffset[1] = ws.vGunOffset[1];
                vGunOffset[2] = ws.vGunOffset[2];
                vGunSpeed = client->vGunSpeed;
                client->vGunSpeed[0] = ws.vGunSpeed[0];
                vGunSpeed[1] = ws.vGunSpeed[1];
                vGunSpeed[2] = ws.vGunSpeed[2];
                if ((LODWORD(viewangles[0]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(viewangles[1]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(viewangles[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\game_mp\\g_active_mp.cpp",
                        895,
                        0,
                        "%s",
                        "!IS_NAN((viewangles)[0]) && !IS_NAN((viewangles)[1]) && !IS_NAN((viewangles)[2])");
                }
                client->fGunPitch = viewangles[0];
                client->fGunYaw = viewangles[1];
                if (pm.mantleStarted)
                    MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 899, 0, "%s", "!pm.mantleStarted");
                //Profile_Begin(27);
                Pmove(&pm);
                //Profile_EndInternal(0);
                if (pm.mantleStarted)
                {
                    if ((client->ps.pm_flags & 4) == 0)
                        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 907, 0, "%s", "client->ps.pm_flags & PMF_MANTLE");
                    G_AddPlayerMantleBlockage(pm.mantleEndPos, pm.mantleDuration, &pm);
                }
                if (client->ps.eventSequence != oldEventSequence)
                {
                    ent->eventTime = level.time;
                    ent->r.eventTime = level.time;
                }
                if (g_smoothClients->current.enabled)
                    G_PlayerStateToEntityStateExtrapolate(&client->ps, &ent->s, client->ps.commandTime, 1);
                else
                    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
                ent->r.currentOrigin[0] = ent->s.lerp.pos.trBase[0];
                ent->r.currentOrigin[1] = ent->s.lerp.pos.trBase[1];
                ent->r.currentOrigin[2] = ent->s.lerp.pos.trBase[2];
                ent->r.mins[0] = pm.mins[0];
                ent->r.mins[1] = pm.mins[1];
                ent->r.mins[2] = pm.mins[2];
                ent->r.maxs[0] = pm.maxs[0];
                ent->r.maxs[1] = pm.maxs[1];
                ent->r.maxs[2] = pm.maxs[2];
                //Profile_Begin(266);
                ClientEvents(ent, oldEventSequence);
                //Profile_EndInternal(0);
                SV_LinkEntity(ent);
                v2 = client->ps.origin;
                ent->r.currentOrigin[0] = client->ps.origin[0];
                ent->r.currentOrigin[1] = v2[1];
                ent->r.currentOrigin[2] = v2[2];
                ent->r.currentAngles[0] = 0.0;
                ent->r.currentAngles[1] = 0.0;
                ent->r.currentAngles[2] = 0.0;
                ent->r.currentAngles[1] = client->ps.viewangles[1];
                //Profile_Begin(267);
                ClientImpacts(ent, &pm);
                //Profile_EndInternal(0);
                if (client->ps.eventSequence != oldEventSequence)
                    ent->eventTime = level.time;
                Player_UpdateActivate(ent);
            }
        }
    }
}

void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap)
{
    s->lerp.pos.trDelta[0] = ps->velocity[0];
    s->lerp.pos.trDelta[1] = ps->velocity[1];
    s->lerp.pos.trDelta[2] = ps->velocity[2];
    if ((COERCE_UNSIGNED_INT(s->lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(s->lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(s->lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game_mp\\g_active_mp.cpp",
            626,
            0,
            "%s",
            "!IS_NAN((s->lerp.pos.trDelta)[0]) && !IS_NAN((s->lerp.pos.trDelta)[1]) && !IS_NAN((s->lerp.pos.trDelta)[2])");
    }
    s->lerp.pos.trTime = time;
    s->lerp.pos.trDuration = 50;
    BG_PlayerStateToEntityState(ps, s, snap, 1u);
    s->lerp.pos.trType = TR_LINEAR_STOP;
}

void __cdecl G_AddPlayerMantleBlockage(float *endPos, int duration, pmove_t *pm)
{
    gentity_s *owner; // [esp+10h] [ebp-Ch]
    gentity_s *ent; // [esp+18h] [ebp-4h]

    owner = &g_entities[pm->ps->clientNum];
    ent = G_Spawn();
    EntHandle::setEnt(&ent->parent, owner);
    EntHandle::setEnt(&ent->r.ownerNum, owner);
    ent->r.contents = 0x10000;
    ent->clipmask = 0x10000;
    ent->r.svFlags = 33;
    ent->s.eType = 5;
    ent->handler = 21;
    ent->r.mins[0] = owner->r.mins[0];
    ent->r.mins[1] = owner->r.mins[1];
    ent->r.mins[2] = owner->r.mins[2];
    ent->r.maxs[0] = owner->r.maxs[0];
    ent->r.maxs[1] = owner->r.maxs[1];
    ent->r.maxs[2] = owner->r.maxs[2];
    G_SetOrigin(ent, endPos);
    SV_LinkEntity(ent);
    ent->nextthink = g_mantleBlockTimeBuffer->current.integer + duration + level.time;
}

void __cdecl ClientThink(int clientNum)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 972, 0, "%s", "Sys_IsMainThread()");
    ent = &g_entities[clientNum];
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 976, 0, "%s", "ent->client");
    if (bgs)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 978, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
    bgs = &level_bgs;
    memcpy(&ent->client->sess.oldcmd, &ent->client->sess.cmd, sizeof(ent->client->sess.oldcmd));
    SV_GetUsercmd(clientNum, &ent->client->sess.cmd);
    ent->client->lastCmdTime = level.time;
    if (!g_synchronousClients->current.enabled)
        ClientThink_real(ent, &ent->client->sess.cmd);
    if (bgs != &level_bgs)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 993, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
    bgs = 0;
}

void __cdecl G_RunClient(gentity_s *ent)
{
    float *origin; // [esp+0h] [ebp-10h]

    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1000, 0, "%s", "ent->client");
    if (ent->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1001, 0, "%s", "ent->client->sess.connected != CON_DISCONNECTED");
    if (g_synchronousClients->current.enabled)
    {
        ent->client->sess.cmd.serverTime = level.time;
        ClientThink_real(ent, &ent->client->sess.cmd);
    }
    if (!ent->client->noclip)
    {
        if (ent->tagInfo)
        {
            ent->client->ps.pm_type = ent->client->sess.sessionState != SESS_STATE_DEAD ? 1 : 8;
            G_SetFixedLink(ent, 2);
            G_SetOrigin(ent, ent->r.currentOrigin);
            G_SetAngle(ent, ent->r.currentAngles);
            ent->s.lerp.pos.trType = TR_INTERPOLATE;
            ent->s.lerp.pos.trDuration = 0;
            ent->s.lerp.pos.trTime = 0;
            ent->s.lerp.pos.trDelta[0] = 0.0;
            ent->s.lerp.pos.trDelta[1] = 0.0;
            ent->s.lerp.pos.trDelta[2] = 0.0;
            ent->s.lerp.apos.trType = TR_INTERPOLATE;
            ent->s.lerp.apos.trDuration = 0;
            ent->s.lerp.apos.trTime = 0;
            ent->s.lerp.apos.trDelta[0] = 0.0;
            ent->s.lerp.apos.trDelta[1] = 0.0;
            ent->s.lerp.apos.trDelta[2] = 0.0;
            SV_LinkEntity(ent);
            origin = ent->client->ps.origin;
            *origin = ent->r.currentOrigin[0];
            origin[1] = ent->r.currentOrigin[1];
            origin[2] = ent->r.currentOrigin[2];
        }
        else if (ent->client->ps.pm_type == 1 || ent->client->ps.pm_type == 8)
        {
            --ent->client->ps.pm_type;
        }
    }
}

void __cdecl IntermissionClientEndFrame(gentity_s *ent)
{
    char *v1; // eax
    char *v2; // eax
    gclient_s *client; // [esp+0h] [ebp-4h]

    client = ent->client;
    ent->r.svFlags &= ~2u;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    ent->r.contents = 0;
    client->ps.otherFlags &= 0xFFFFFFE3;
    client->ps.pm_type = 5;
    client->ps.eFlags &= ~0x200000u;
    client->ps.eFlags &= ~0x40u;
    client->ps.viewmodelIndex = 0;
    ent->s.eType = 5;
    v1 = va("%i", level.teamScores[1]);
    SV_SetConfigstring(4, v1);
    v2 = va("%i", level.teamScores[2]);
    SV_SetConfigstring(5, v2);
}

void __cdecl SpectatorClientEndFrame(gentity_s *ent)
{
    unsigned int v1; // edx
    gclient_s *client; // [esp+0h] [ebp-2FE4h]
    clientState_s v3; // [esp+4h] [ebp-2FE0h] BYREF
    unsigned int v4; // [esp+6Ch] [ebp-2F78h]
    int clientNum; // [esp+70h] [ebp-2F74h]
    playerState_s ps; // [esp+74h] [ebp-2F70h] BYREF
    int pArchiveTime; // [esp+2FE0h] [ebp-4h] BYREF

    client = ent->client;
    ent->r.svFlags &= ~2u;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    ent->r.contents = 0;
    client->ps.otherFlags &= ~4u;
    ent->s.eType = 5;
    client->ps.viewmodelIndex = 0;
    client->fGunPitch = 0.0;
    client->fGunYaw = 0.0;
    if (client->sess.forceSpectatorClient >= 0)
    {
        clientNum = client->sess.forceSpectatorClient;
        client->spectatorClient = clientNum;
        while (1)
        {
            if (client->sess.archiveTime < 0)
                client->sess.archiveTime = 0;
            pArchiveTime = client->sess.archiveTime - client->sess.psOffsetTime;
            if (SV_GetArchivedClientInfo(client->sess.forceSpectatorClient, &pArchiveTime, &ps, &v3))
            {
                if (client->sess.killCamEntity == -1)
                    ps.killCamEntity = 1023;
                else
                    ps.killCamEntity = client->sess.killCamEntity;
                if ((ps.otherFlags & 4) == 0)
                    MyAssertHandler(
                        ".\\game_mp\\g_active_mp.cpp",
                        1119,
                        0,
                        "%s\n\t(ps.otherFlags) = %i",
                        "(ps.otherFlags & (1<<2))",
                        ps.otherFlags);
                if (G_ClientCanSpectateTeam(client, v3.team))
                    goto doFollow;
            }
            if (!client->sess.archiveTime)
            {
                client->sess.forceSpectatorClient = -1;
                client->sess.killCamEntity = -1;
                client->spectatorClient = -1;
                break;
            }
            client->sess.archiveTime -= 50;
        }
    }
    if (client->spectatorClient < 0 && !G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS))
        Cmd_FollowCycle_f(ent, 1);
    clientNum = client->spectatorClient;
    if (clientNum >= 0)
    {
        pArchiveTime = client->sess.psOffsetTime + client->sess.archiveTime;
        if (SV_GetArchivedClientInfo(clientNum, &pArchiveTime, &ps, &v3))
        {
            if ((ps.otherFlags & 4) == 0)
                MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1147, 0, "%s", "ps.otherFlags & POF_PLAYER");
            if (G_ClientCanSpectateTeam(client, v3.team))
            {
            doFollow:
                v4 = client->ps.eFlags & 0x100000 | ps.eFlags & 0xFFEFFFFF;
                memcpy((unsigned __int8 *)client, (unsigned __int8 *)&ps, 0x2F64u);
                HudElem_UpdateClient(client, ent->s.number, HUDELEM_UPDATE_CURRENT);
                client->ps.eFlags = v4;
                client->ps.otherFlags &= ~4u;
                client->ps.otherFlags |= 2u;
                if (client->sess.forceSpectatorClient < 0)
                {
                    client->ps.otherFlags |= 8u;
                    if (G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS))
                        v1 = client->ps.otherFlags | 0x10;
                    else
                        v1 = client->ps.otherFlags & 0xFFFFFFEF;
                    client->ps.otherFlags = v1;
                }
                else
                {
                    client->ps.otherFlags &= 0xFFFFFFE7;
                }
                return;
            }
        }
    }
    StopFollowing(ent);
    client->ps.otherFlags &= ~0x10u;
    if (G_ClientCanSpectateTeam(client, TEAM_ALLIES)
        || G_ClientCanSpectateTeam(client, TEAM_AXIS)
        || G_ClientCanSpectateTeam(client, TEAM_FREE))
    {
        client->ps.otherFlags |= 8u;
    }
    else
    {
        client->ps.otherFlags &= ~8u;
    }
}

bool __cdecl G_ClientCanSpectateTeam(gclient_s *client, team_t team)
{
    return (client->sess.noSpectate & (1 << team)) == 0;
}

hudelem_s g_dummyHudCurrent;
int __cdecl GetFollowPlayerState(int clientNum, playerState_s *ps)
{
    gclient_s *client; // [esp+8h] [ebp-8h]
    unsigned int index; // [esp+Ch] [ebp-4h]

    client = &level.clients[clientNum];
    if (!client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1221, 0, "%s", "client");
    if (client != g_entities[clientNum].client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1222, 0, "%s", "client == g_entities[clientNum].client");
    if ((client->ps.otherFlags & 4) != 0)
    {
        memcpy((unsigned __int8 *)ps, (unsigned __int8 *)client, sizeof(playerState_s));
        for (index = 0; index < 0x1F && ps->hud.current[index].type; ++index)
        {
            memset((unsigned __int8 *)&ps->hud.current[index], 0, sizeof(ps->hud.current[index]));
            if (ps->hud.current[index].type)
                MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1237, 0, "%s", "ps->hud.current[index].type == HE_TYPE_FREE");
        }
        while (index < 0x1F)
        {
            if (memcmp(&ps->hud.current[index], &g_dummyHudCurrent, 0xA0u))
                MyAssertHandler(
                    ".\\game_mp\\g_active_mp.cpp",
                    1244,
                    0,
                    "%s",
                    "!memcmp( &ps->hud.current[index], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )");
            ++index;
        }
        return 1;
    }
    else
    {
        memset((unsigned __int8 *)ps, 0, sizeof(playerState_s));
        return 0;
    }
}

int __cdecl StuckInClient(gentity_s *self)
{
    float v2; // [esp+0h] [ebp-50h]
    float integer; // [esp+4h] [ebp-4Ch]
    float v4; // [esp+8h] [ebp-48h]
    float *v5; // [esp+Ch] [ebp-44h]
    float v6; // [esp+10h] [ebp-40h]
    float *velocity; // [esp+14h] [ebp-3Ch]
    float fDist; // [esp+30h] [ebp-20h]
    gentity_s *hit; // [esp+34h] [ebp-1Ch]
    int i; // [esp+38h] [ebp-18h]
    float selfSpeed; // [esp+3Ch] [ebp-14h]
    float hitSpeed; // [esp+40h] [ebp-10h]
    float vDelta[3]; // [esp+44h] [ebp-Ch] BYREF

    LODWORD(vDelta[2]) = 300;
    if ((self->client->ps.otherFlags & 4) == 0)
        return 0;
    if (self->client->sess.sessionState)
        return 0;
    if (self->r.contents != 0x2000000 && (char *)self->r.contents != &svs.clients[29].netchanOutgoingBuffer[109728])
        return 0;
    hit = g_entities;
    for (i = 0; ; ++i)
    {
        if (i >= level.maxclients)
            return 0;
        if (hit->r.inuse
            && (hit->client->ps.otherFlags & 4) != 0
            && hit->client->sess.sessionState == SESS_STATE_PLAYING
            && hit != self
            && hit->client
            && hit->health > 0
            && (hit->r.contents == 0x2000000 || (char *)hit->r.contents == &svs.clients[29].netchanOutgoingBuffer[109728])
            && self->r.absmax[0] >= (double)hit->r.absmin[0]
            && self->r.absmin[0] <= (double)hit->r.absmax[0]
            && self->r.absmax[1] >= (double)hit->r.absmin[1]
            && self->r.absmin[1] <= (double)hit->r.absmax[1]
            && self->r.absmax[2] >= (double)hit->r.absmin[2]
            && self->r.absmin[2] <= (double)hit->r.absmax[2])
        {
            vDelta[0] = hit->r.currentOrigin[0] - self->r.currentOrigin[0];
            vDelta[1] = hit->r.currentOrigin[1] - self->r.currentOrigin[1];
            fDist = self->r.maxs[0] + hit->r.maxs[0];
            v4 = vDelta[1] * vDelta[1] + vDelta[0] * vDelta[0];
            if (v4 <= fDist * fDist)
                break;
        }
        ++hit;
    }
    vDelta[0] = hit->r.currentOrigin[0] - self->r.currentOrigin[0];
    vDelta[1] = hit->r.currentOrigin[1] - self->r.currentOrigin[1];
    vDelta[0] = crandom() + vDelta[0];
    vDelta[1] = crandom() + vDelta[1];
    Vec2Normalize(vDelta);
    if (Vec2Length(hit->client->ps.velocity) <= 0.0)
        integer = 0.0;
    else
        integer = (float)g_playerCollisionEjectSpeed->current.integer;
    hitSpeed = integer;
    if (Vec2Length(self->client->ps.velocity) <= 0.0)
        v2 = 0.0;
    else
        v2 = (float)g_playerCollisionEjectSpeed->current.integer;
    selfSpeed = v2;
    if (integer < 0.00009999999747378752 && v2 < 0.00009999999747378752)
    {
        hitSpeed = (float)hit->client->ps.speed;
        selfSpeed = (float)self->client->ps.speed;
    }
    velocity = hit->client->ps.velocity;
    *velocity = hitSpeed * vDelta[0];
    velocity[1] = hitSpeed * vDelta[1];
    hit->client->ps.pm_time = 300;
    hit->client->ps.pm_flags |= 0x80u;
    v5 = self->client->ps.velocity;
    v6 = -selfSpeed;
    *v5 = v6 * vDelta[0];
    v5[1] = v6 * vDelta[1];
    self->client->ps.pm_time = 300;
    self->client->ps.pm_flags |= 0x80u;
    return 1;
}

unsigned __int16 *controller_names[] =
{
    &scr_const.back_mid,
    &scr_const.back_up,
    &scr_const.neck,
    &scr_const.head,
    &scr_const.pelvis
};

void __cdecl G_PlayerController(const gentity_s *self, int *partBits)
{
    const DObj_s *obj; // [esp+0h] [ebp-1Ch]
    clientInfo_t *ci; // [esp+4h] [ebp-18h]
    int i; // [esp+8h] [ebp-14h]
    CEntPlayerInfo player; // [esp+Ch] [ebp-10h] BYREF

    SV_CheckThread();
    if (self->s.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\game_mp\\g_active_mp.cpp",
            1353,
            0,
            "self->s.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            self->s.clientNum,
            64);
    ci = &level_bgs.clientinfo[self->s.clientNum];
    if (!ci->infoValid)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1355, 0, "%s", "ci->infoValid");
    if (bgs != &level_bgs)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1357, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
    BG_Player_DoControllersSetup(&self->s, ci, level.frametime);
    obj = Com_GetServerDObj(self->s.number);
    for (i = 0; i < 6; ++i)
    {
        player.tag[i] = -2;
        DObjGetBoneIndex(obj, *controller_names[i], &player.tag[i]);
    }
    player.control = &ci->control;
    BG_Player_DoControllers(&player, obj, partBits);
}

void __cdecl ClientEndFrame(gentity_s *ent)
{
    int v1; // ecx
    int v2; // eax
    unsigned int v3; // edx
    const char *v4; // eax
    char *v5; // eax
    int v6; // [esp+24h] [ebp-144h]
    bool v7; // [esp+28h] [ebp-140h]
    float *playerAngles; // [esp+70h] [ebp-F8h]
    float *viewangles; // [esp+74h] [ebp-F4h]
    float v10; // [esp+DCh] [ebp-8Ch]
    int partBits[4]; // [esp+120h] [ebp-48h] BYREF
    int bChanged; // [esp+130h] [ebp-38h]
    gclient_s *client; // [esp+134h] [ebp-34h]
    DObj_s *obj; // [esp+138h] [ebp-30h]
    float vViewPos[3]; // [esp+13Ch] [ebp-2Ch] BYREF
    clientInfo_t *ci; // [esp+148h] [ebp-20h]
    int clientNum; // [esp+14Ch] [ebp-1Ch]
    float spawn_angles[3]; // [esp+150h] [ebp-18h] BYREF
    float spawn_origin[3]; // [esp+15Ch] [ebp-Ch] BYREF

    client = ent->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1489, 0, "%s", "client");
    if (client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1490, 0, "%s", "client->sess.connected != CON_DISCONNECTED");
    ent->handler = 11;
    client->ps.deltaTime = 0;
    client->ps.gravity = (int)g_gravity->current.value;
    client->ps.moveSpeedScaleMultiplier = ent->client->sess.moveSpeedScaleMultiplier;
    if (client->bFrozen)
        v1 = client->ps.pm_flags | 0x800;
    else
        v1 = client->ps.pm_flags & 0xFFFFF7FF;
    client->ps.pm_flags = v1;
    //Profile_Begin(270);
    bChanged = G_UpdateClientInfo(ent);
    //Profile_EndInternal(0);
    if (client->sess.connected == CON_CONNECTED)
    {
        if (client->sess.sessionState == SESS_STATE_INTERMISSION)
        {
            //Profile_Begin(273);
            IntermissionClientEndFrame(ent);
            //Profile_EndInternal(0);
            ent->client->buttonsSinceLastFrame = 0;
        }
        else if (client->sess.sessionState == SESS_STATE_SPECTATOR)
        {
            //Profile_Begin(274);
            SpectatorClientEndFrame(ent);
            //Profile_EndInternal(0);
            ent->client->buttonsSinceLastFrame = 0;
        }
        else if (client->ps.clientNum == ent->s.number)
        {
            ent->r.svFlags |= 2u;
            ent->r.svFlags &= ~1u;
            ent->takedamage = 1;
            client->ps.otherFlags |= 4u;
            client->ps.otherFlags &= 0xFFFFFFE7;
            client->ps.viewmodelIndex = client->sess.viewmodelIndex;
            G_SetClientContents(ent);
            client->dropWeaponTime = 0;
            if (client->compassPingTime <= level.time)
                client->ps.eFlags &= ~0x400000u;
            if (client->noclip)
            {
                client->ps.pm_type = 2;
            }
            else if (client->ufo)
            {
                client->ps.pm_type = 3;
            }
            else if (client->sess.sessionState == SESS_STATE_DEAD)
            {
                client->ps.pm_type = (ent->tagInfo != 0) + 7;
                ent->r.svFlags |= 1u;
                ent->r.svFlags &= ~2u;
                ent->takedamage = 0;
            }
            else if (client->lastStand)
            {
                client->ps.pm_type = 6;
            }
            else
            {
                client->ps.pm_type = ent->tagInfo != 0;
            }
            client->currentAimSpreadScale = client->ps.aimSpreadScale / 255.0;
            //Profile_Begin(276);
            Player_UpdateCursorHints(ent);
            //Profile_EndInternal(0);
            P_DamageFeedback(ent);
            if (level.time - client->lastCmdTime <= 1000)
                v2 = ent->s.lerp.eFlags & 0xFFFFFF7F;
            else
                v2 = ent->s.lerp.eFlags | 0x80;
            ent->s.lerp.eFlags = v2;
            client->ps.stats[0] = ent->health;
            G_SetClientSound(ent);
            v7 = level.teamHasRadar[client->sess.cs.team] || client->hasRadar;
            client->ps.radarEnabled = v7;
            if (g_smoothClients->current.enabled)
            {
                //Profile_Begin(277);
                G_PlayerStateToEntityStateExtrapolate(&client->ps, &ent->s, client->ps.commandTime, 1);
            }
            else
            {
                //Profile_Begin(278);
                BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
            }
            //Profile_EndInternal(0);
            if (ent->health > 0)
                StuckInClient(ent);
            G_GetPlayerViewOrigin(&ent->client->ps, vViewPos);
            //Profile_Begin(279);
            client->ps.iCompassPlayerInfo = G_GetNonPVSPlayerInfo(ent, vViewPos, client->iLastCompassPlayerInfoEnt);
            //Profile_EndInternal(0);
            if (client->ps.iCompassPlayerInfo)
            {
                client->iLastCompassPlayerInfoEnt = client->ps.iCompassPlayerInfo & 0x3F;
                if ((g_entities[client->iLastCompassPlayerInfoEnt].s.lerp.eFlags & 0x400000) != 0)
                    v3 = client->ps.eFlags | 0x800000;
                else
                    v3 = client->ps.eFlags & 0xFF7FFFFF;
                client->ps.eFlags = v3;
            }
            else
            {
                client->iLastCompassPlayerInfoEnt = 1023;
            }
            if (ent->s.eType == 1)
            {
                ent->handler = 2 * (ent->health <= 0) + 10;
                clientNum = ent->s.clientNum;
                if ((unsigned int)clientNum >= 0x40)
                    MyAssertHandler(
                        ".\\game_mp\\g_active_mp.cpp",
                        1669,
                        0,
                        "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        clientNum,
                        64);
                ci = &level_bgs.clientinfo[clientNum];
                if (!ci->infoValid)
                    MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1671, 0, "%s", "ci->infoValid");
                ci->lerpMoveDir = (float)ent->s.lerp.u.loopFx.period;
                ci->lerpLean = ent->s.lerp.u.turret.gunAngles[0];
                playerAngles = ci->playerAngles;
                viewangles = client->ps.viewangles;
                ci->playerAngles[0] = client->ps.viewangles[0];
                playerAngles[1] = viewangles[1];
                playerAngles[2] = viewangles[2];
                if (bChanged)
                    G_SafeDObjFree(ent);
                obj = Com_GetServerDObj(ent->s.number);
                //Profile_Begin(281);
                BG_UpdatePlayerDObj(-1, obj, &ent->s, ci, ent->attachIgnoreCollision);
                //Profile_EndInternal(0);
                //Profile_Begin(280);
                BG_PlayerAnimation(-1, &ent->s, ci);
                //Profile_EndInternal(0);
                if ((client->ps.otherFlags & 4) != 0 && (client->ps.eFlags & 0x300) != 0)
                {
                    if (client->ps.clientNum != ent->s.number)
                        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1691, 0, "%s", "client->ps.clientNum == ent->s.number");
                    if (client->ps.viewlocked_entNum == 1023)
                        MyAssertHandler(
                            ".\\game_mp\\g_active_mp.cpp",
                            1692,
                            0,
                            "%s",
                            "client->ps.viewlocked_entNum != ENTITYNUM_NONE");
                    if (!EntHandle::isDefined(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum)
                        || EntHandle::ent(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum) != ent)
                    {
                        if (EntHandle::isDefined(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum))
                        {
                            v6 = EntHandle::entnum(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum);
                            v4 = va(
                                "viewlocked_entNum is %i, ownerNum is %i, ent->s.number is %i",
                                client->ps.viewlocked_entNum,
                                v6,
                                ent->s.number);
                        }
                        else
                        {
                            v4 = va(
                                "viewlocked_entNum is %i, ownerNum is %i, ent->s.number is %i",
                                client->ps.viewlocked_entNum,
                                1023,
                                ent->s.number);
                        }
                        MyAssertHandler(
                            ".\\game_mp\\g_active_mp.cpp",
                            1693,
                            0,
                            "%s\n\t%s",
                            "level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined() && (level.gentities[client->ps.viewlo"
                            "cked_entNum].r.ownerNum.ent() == ent)",
                            v4);
                    }
                    turret_think_client(&level.gentities[client->ps.viewlocked_entNum]);
                }
                if (g_debugLocDamage->current.enabled && SV_DObjExists(ent))
                {
                    memset(partBits, 255, sizeof(partBits));
                    G_DObjCalcPose(ent, partBits);
                    SV_XModelDebugBoxes(ent);
                    v5 = va(
                        "[%i] %.3f, %.3f, %.3f\n",
                        clientNum,
                        level_bgs.clientinfo[clientNum].legs.yawAngle,
                        level_bgs.clientinfo[clientNum].torso.yawAngle,
                        level_bgs.clientinfo[clientNum].playerAngles[1]);
                    CL_AddDebugStarWithText(ent->r.currentOrigin, colorYellow, colorYellow, v5, 0.25, 1, 1);
                }
                ent->client->buttonsSinceLastFrame = 0;
            }
            else
            {
                ent->client->buttonsSinceLastFrame = 0;
            }
        }
        else
        {
            spawn_origin[0] = client->ps.origin[0];
            spawn_origin[1] = client->ps.origin[1];
            spawn_origin[2] = client->ps.origin[2];
            v10 = client->ps.viewangles[1];
            spawn_angles[0] = 0.0;
            spawn_angles[1] = v10;
            spawn_angles[2] = 0.0;
            //Profile_Begin(275);
            ClientSpawn(ent, spawn_origin, spawn_angles);
            //Profile_EndInternal(0);
            if (client->ps.clientNum != ent->s.number)
                MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1544, 0, "%s", "client->ps.clientNum == ent->s.number");
            ent->client->buttonsSinceLastFrame = 0;
        }
    }
    else
    {
        ent->client->buttonsSinceLastFrame = 0;
    }
}

int __cdecl G_UpdateClientInfo(gentity_s *ent)
{
    unsigned int v1; // eax
    unsigned int v2; // eax
    const char *v3; // eax
    gentity_s *v4; // eax
    int number; // [esp-4h] [ebp-5Ch]
    char *tagName; // [esp+3Ch] [ebp-1Ch]
    int bChanged; // [esp+40h] [ebp-18h]
    gclient_s *client; // [esp+44h] [ebp-14h]
    char *modelName; // [esp+48h] [ebp-10h]
    char *modelNamea; // [esp+48h] [ebp-10h]
    clientInfo_t *ci; // [esp+4Ch] [ebp-Ch]
    int i; // [esp+50h] [ebp-8h]
    unsigned int clientNum; // [esp+54h] [ebp-4h]

    client = ent->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1386, 0, "%s", "client");
    if (client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1387, 0, "%s", "client->sess.connected != CON_DISCONNECTED");
    clientNum = ent->s.clientNum;
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_active_mp.cpp",
            1390,
            0,
            "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            clientNum,
            64);
    ci = &level_bgs.clientinfo[clientNum];
    if (!ci->infoValid)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1392, 0, "%s", "ci->infoValid");
    bChanged = 0;
    v1 = G_ModelName(ent->model);
    modelName = SL_ConvertToString(v1);
    client->sess.cs.modelindex = ent->model;
    if (strcmp(ci->model, modelName))
    {
        bChanged = 1;
        I_strncpyz(ci->model, modelName, 64);
    }
    for (i = 0; i < 6; ++i)
    {
        if (ent->attachModelNames[i])
        {
            v2 = G_ModelName(ent->attachModelNames[i]);
            modelNamea = SL_ConvertToString(v2);
            client->sess.cs.attachModelIndex[i] = ent->attachModelNames[i];
            if (strcmp(ci->attachModelNames[i], modelNamea))
            {
                bChanged = 1;
                I_strncpyz(ci->attachModelNames[i], modelNamea, 64);
            }
            if (!ent->attachTagNames[i])
                MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1424, 0, "%s", "ent->attachTagNames[i]");
            tagName = SL_ConvertToString(ent->attachTagNames[i]);
            client->sess.cs.attachTagIndex[i] = G_TagIndex(tagName);
            if (strcmp(ci->attachTagNames[i], tagName))
            {
                bChanged = 1;
                I_strncpyz(ci->attachTagNames[i], tagName, 64);
            }
        }
        else
        {
            ci->attachModelNames[i][0] = 0;
            ci->attachTagNames[i][0] = 0;
            client->sess.cs.attachModelIndex[i] = 0;
            client->sess.cs.attachTagIndex[i] = 0;
        }
    }
    if ((client->ps.pm_flags & 0x100000) != 0)
    {
        v3 = va("pm_flags is %i", client->ps.pm_flags);
        MyAssertHandler(
            ".\\game_mp\\g_active_mp.cpp",
            1436,
            0,
            "%s\n\t%s",
            "!( client->ps.pm_flags & PMF_VEHICLE_ATTACHED )",
            v3);
    }
    if ((client->ps.pm_flags & 0x100000) != 0)
    {
        if (EntHandle::isDefined(&ent->r.ownerNum))
        {
            if (!EntHandle::isDefined(&ent->r.ownerNum))
                MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1449, 0, "%s", "ent->r.ownerNum.isDefined()");
            client->sess.cs.attachedVehEntNum = EntHandle::entnum(&ent->r.ownerNum);
            number = ent->s.number;
            v4 = EntHandle::ent(&ent->r.ownerNum);
            client->sess.cs.attachedVehSlotIndex = G_VehPlayerRideSlot(v4, number);
        }
        else
        {
            Com_PrintWarning(16, "G_UpdateClientInfo(): Veh attached, but no ownerNum\n");
            client->sess.cs.attachedVehEntNum = 1023;
            client->sess.cs.attachedVehSlotIndex = 0;
        }
    }
    else
    {
        client->sess.cs.attachedVehEntNum = 1023;
        client->sess.cs.attachedVehSlotIndex = 0;
    }
    return bChanged;
}

void __cdecl G_PlayerEvent(int clientNum, int event)
{
    gclient_s *client; // [esp+4h] [ebp-10h]
    float kickAVel[3]; // [esp+8h] [ebp-Ch] BYREF

    client = g_entities[clientNum].client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_active_mp.cpp", 1718, 0, "%s", "client");
    if (event >= 26 && (event <= 27 || event == 38))
        BG_WeaponFireRecoil(&client->ps, client->vGunSpeed, kickAVel);
}

