#include "g_main_mp.h"

#include "g_public_mp.h"
#include "g_utils_mp.h"

#include <bgame/bg_local.h>

#include <client/client.h>

#include <database/database.h>

#include <game/game_public.h>

#include <server/sv_game.h>
#include <server/sv_world.h>

#include <script/scr_const.h>
#include <script/scr_main.h>
#include <script/scr_vm.h>

#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>

#include <universal/com_files.h>
#include <universal/com_sndalias.h>
#include <universal/profile.h>

//struct bgs_t level_bgs     82d8f0f8     g_main_mp.obj
//struct entityHandler_t *entityHandlers 827b5088     g_main_mp.obj
//struct gentity_s *g_entities 82cf2090     g_main_mp.obj
//struct level_locals_t level 82e7b988     g_main_mp.obj
// 

const dvar_t *pickupPrints;
const dvar_t *g_password;
const dvar_t *g_smoothClients;
const dvar_t *g_voteAbstainWeight;
const dvar_t *g_TeamColor_MyTeam;
const dvar_t *g_gravity ;
const dvar_t *g_filterBan;
const dvar_t *g_voiceChatsAllowed;
const dvar_t *g_deadChat;
const dvar_t *radius_damage_debug;
const dvar_t *g_dumpAnims;
const dvar_t *g_ScoresColor_Allies;
const dvar_t *g_friendlyfireDist;
const dvar_t *g_dropUpSpeedRand;
const dvar_t *g_maxclients;
const dvar_t *player_MGUseRadius;
const dvar_t *g_TeamName_Allies;
const dvar_t *g_debugBullets;
const dvar_t *g_synchronousClients;
const dvar_t *g_knockback;
const dvar_t *player_throwbackInnerRadius;
const dvar_t *g_ScoresColor_MyTeam;
const dvar_t *g_allowVote;
const dvar_t *anim_deltas_debug;
const dvar_t *g_dedicated;
const dvar_t *g_TeamColor_Allies;
const dvar_t *g_antilag ;
const dvar_t *g_TeamIcon_Allies;
const dvar_t *g_playerCollisionEjectSpeed;
const dvar_t *g_entinfo ;
const dvar_t *melee_debug;
const dvar_t *g_useholdspawndelay;
const dvar_t *g_TeamColor_Free;
const dvar_t *g_debugPlayerAnimScript;
const dvar_t *g_fogColorReadOnly;
const dvar_t *g_dropUpSpeedBase;
const dvar_t *g_listEntity;
const dvar_t *g_inactivity;
const dvar_t *g_TeamIcon_Spectator;
const dvar_t *g_redCrosshairs;
const dvar_t *g_cheats  ;
const dvar_t *g_TeamColor_Spectator;
const dvar_t *g_fogHalfDistReadOnly;
const dvar_t *g_maxDroppedWeapons;
const dvar_t *g_dropForwardSpeed;
const dvar_t *g_ScoresColor_Free;
const dvar_t *g_minGrenadeDamageSpeed;
const dvar_t *g_dropHorzSpeedRand;
const dvar_t *g_voiceChatTalkingDuration;
const dvar_t *g_ScoresColor_Spectator;
const dvar_t *g_useholdtime;
const dvar_t *g_ScoresColor_EnemyTeam;
const dvar_t *g_compassShowEnemies;
const dvar_t *g_speed   ;
const dvar_t *g_friendlyNameDist;
const dvar_t *g_log     ;
const dvar_t *g_TeamName_Axis;
const dvar_t *g_TeamIcon_Axis;
const dvar_t *bullet_penetrationEnabled;
const dvar_t *g_TeamColor_Axis;
const dvar_t *g_NoScriptSpam;
const dvar_t *g_banIPs  ;
const dvar_t *g_gametype;
const dvar_t *g_fogStartDistReadOnly;
const dvar_t *g_debugLocDamage;
const dvar_t *g_logSync ;
const dvar_t *g_mantleBlockTimeBuffer;
const dvar_t *player_throwbackOuterRadius;
const dvar_t *g_oldVoting;
const dvar_t *g_ScoresColor_Axis;
const dvar_t *g_TeamIcon_Free;
const dvar_t *g_motd    ;
const dvar_t *g_TeamColor_EnemyTeam;
const dvar_t *g_debugDamage;
const dvar_t *g_clonePlayerMaxVelocity;

const dvar_t *voice_global;
const dvar_t *voice_localEcho;
const dvar_t *voice_deadChat;

bgs_t level_bgs;
level_locals_t level;
gentity_s g_entities[1024];
gclient_s g_clients[64];
const char *g_entinfoNames[3] =
{
    "off",
    "all ents",
    NULL
};

void __cdecl TRACK_g_main()
{
    track_static_alloc_internal(&level, 11884, "level", 9);
    track_static_alloc_internal(g_entities, 643072, "g_entities", 9);
    track_static_alloc_internal(g_clients, 811264, "g_clients", 9);
    //track_static_alloc_internal(entityHandlers, 960, "entityHandlers", 9);
    track_static_alloc_internal(g_entinfoNames, 12, "g_entinfoNames", 0);
}

int __cdecl G_GetSavePersist()
{
    return level.savepersist;
}

void __cdecl G_SetSavePersist(int savepersist)
{
    level.savepersist = savepersist;
}

double __cdecl G_GetFogOpaqueDistSqrd()
{
    return level.fFogOpaqueDistSqrd;
}

int __cdecl G_GetClientScore(int clientNum)
{
    return level.clients[clientNum].sess.score;
}

int __cdecl G_GetClientArchiveTime(int clientNum)
{
    return level.clients[clientNum].sess.archiveTime;
}

void __cdecl G_SetClientArchiveTime(int clientNum, int time)
{
    level.clients[clientNum].sess.archiveTime = time;
}

clientState_s *__cdecl G_GetClientState(int clientNum)
{
    return &level.clients[clientNum].sess.cs;
}

gclient_s *__cdecl G_GetPlayerState(int clientNum)
{
    return &level.clients[clientNum];
}

int __cdecl G_GetClientSize()
{
    return 12676;
}

void __cdecl G_FreeEntities()
{
    gentity_s *e; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    e = g_entities;
    for (i = 0; i < level.num_entities; ++i)
    {
        if (e->r.inuse)
            G_FreeEntity(e);
        ++e;
    }
    if (g_entities[1022].r.inuse)
        G_FreeEntity(&g_entities[1022]);
    for (ia = 0; ia < 32; ++ia)
    {
        //EntHandle::setEnt((EntHandle *)(4 * ia + 23802164), 0);
        EntHandle::setEnt(&level.droppedWeaponCue[ia], 0);
    }
    if (g_entities[1023].r.inuse)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 826, 0, "%s", "!g_entities[ENTITYNUM_NONE].r.inuse");
    level.num_entities = 0;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
}

bool __cdecl G_ExitAfterConnectPaths()
{
    return 0;
}

int __cdecl G_IsServerGameSystem(int clientNum)
{
    if (!g_debugPlayerAnimScript)
        return 0;
    if (clientNum != g_debugPlayerAnimScript->current.integer)
        return 0;
    if (bgs != &level_bgs)
        return 0;
    Com_Printf(19, "(%i) ", level.time);
    return 1;
}

void __cdecl G_InitGame(int levelTime, int randomSeed, int restart, int savepersist)
{
    com_parse_mark_t *v4; // edx
    char serverinfo[1028]; // [esp+4h] [ebp-810h] BYREF
    int file; // [esp+408h] [ebp-40Ch]
    char buffer[1024]; // [esp+40Ch] [ebp-408h] BYREF
    int i; // [esp+810h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 992, 0, "%s", "Sys_IsMainThread()");
    Com_Printf(15, "------- Game Initialization -------\n");
    Com_Printf(15, "gamename: %s\n", "KisakCoD4");
    Com_Printf(15, "gamedate: %s\n", __DATE__);
    Swap_Init();
    EntHandle::Init();
    memset((unsigned __int8 *)&level, 0, sizeof(level));
    level.initializing = 1;
    level.time = levelTime;
    level.startTime = levelTime;
    level.currentEntityThink = -1;
    level.scriptPrintChannel = 24;
    srand(randomSeed);
    Rand_Init(randomSeed);
    GScr_LoadConsts();
    G_SetupWeaponDef();
    if (!restart || !savepersist)
        G_RegisterDvars();
    if (!Dvar_GetBool("sv_cheats") && !restart)
        Dvar_SetCheatState();
    G_ProcessIPBans();
    level_bgs.GetXModel = (XModel * (__cdecl *)(const char *))SV_XModelGet;
    level_bgs.CreateDObj = (void(__cdecl *)(DObjModel_s *, unsigned __int16, XAnimTree_s *, int, int, clientInfo_t *))G_CreateDObj;
    level_bgs.AttachWeapon = 0;
    level_bgs.GetDObj = G_GetDObj;
    level_bgs.SafeDObjFree = G_SafeDObjFree;
    level_bgs.AllocXAnim = (void *(__cdecl *)(int))Hunk_AllocXAnimServer;
    level_bgs.anim_user = 1;
    if (*(_BYTE *)g_log->current.integer)
    {
        if (g_logSync->current.enabled)
            FS_FOpenFileByMode((char *)g_log->current.integer, &level.logFile, FS_APPEND_SYNC);
        else
            FS_FOpenFileByMode((char *)g_log->current.integer, &level.logFile, FS_APPEND);
        if (level.logFile)
        {
            SV_GetServerinfo(serverinfo, 1024);
            G_LogPrintf("------------------------------------------------------------\n");
            G_LogPrintf("InitGame: %s\n", serverinfo);
        }
        else
        {
            Com_PrintWarning(15, "WARNING: Couldn't open logfile: %s\n", g_log->current.string);
        }
    }
    else
    {
        Com_Printf(15, "Not logging to disk.\n");
    }
    for (file = 0; file < 1; ++file)
    {
        level.openScriptIOFileHandles[file] = 0;
        level.openScriptIOFileBuffers[file] = 0;
        v4 = &level.currentScriptIOLineMark[file];
        v4->lines = 0;
        v4->text = 0;
        v4->ungetToken = 0;
        v4->backup_lines = 0;
        v4->backup_text = 0;
    }
    Mantle_CreateAnims((void *(__cdecl *)(int))Hunk_AllocXAnimServer);
    iassert(bgs == NULL);
    bgs = &level_bgs;
    if (!restart)
    {
        memset(&bgs->animScriptData, 0, sizeof(animScriptData_t));
#ifndef DEDICATED
        bgs->animScriptData.soundAlias = Com_FindSoundAlias;
#else
        bgs->animScriptData.soundAlias = Com_FindSoundAlias_DEDICATED;
#endif
        bgs->animScriptData.playSoundAlias = G_AnimScriptSound;
        GScr_LoadScripts();
        BG_LoadAnim();
        G_LoadAnimTreeInstances();
    }
    SV_GetConfigstring(0x13u, buffer, 1024);
    Info_SetValueForKey(buffer, "winner", "0");
    SV_SetConfigstring(19, buffer);
    memset(g_entities, 0, sizeof(g_entities));
    level.gentities = g_entities;
    level.maxclients = g_maxclients->current.integer;
    memset(g_clients, 0, sizeof(g_clients));
    level.clients = g_clients;
    for (i = 0; i < level.maxclients; ++i)
        g_entities[i].client = &level.clients[i];
    level.num_entities = 72;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
    SV_LocateGameData(level.gentities, 72, 628, &level.clients->ps, 12676);
    G_ParseHitLocDmgTable();
    BG_LoadPenetrationDepthTable();
    G_VehiclesInit(restart);
    G_InitTurrets();
    G_SpawnEntitiesFromString();
    G_VehiclesSetupSpawnedEnts();
    G_setfog("0");
    G_InitObjectives();
    Missile_InitAttractors();
    Scr_FreeEntityList();
    Com_Printf(15, "-----------------------------------\n");
    Scr_InitSystem(1);
    Scr_SetLoading(1);
    Scr_AllocGameVariable();
    G_LoadStructs();
    Scr_LoadGameType();
    Scr_LoadLevel();
    Scr_StartupGameType();

    for (i = 0; i < 8; ++i)
        g_scr_data.playerCorpseInfo[i].entnum = -1;

    if (useFastFile->current.enabled)
        G_PrintAllFastFileErrors();

    iassert(bgs == &level_bgs);

    bgs = 0;
    level.initializing = 0;
    SaveRegisteredWeapons();
    SaveRegisteredItems();
}

int MY_DEFAULT_USEHOLDSPAWNDELAY = 500;

const dvar_s *G_RegisterDvars()
{
    const dvar_s *result; // eax
    DvarLimits min; // [esp+8h] [ebp-14h]
    DvarLimits mina; // [esp+8h] [ebp-14h]
    DvarLimits minb; // [esp+8h] [ebp-14h]
    DvarLimits minc; // [esp+8h] [ebp-14h]
    DvarLimits mind; // [esp+8h] [ebp-14h]
    DvarLimits mine; // [esp+8h] [ebp-14h]
    DvarLimits minf; // [esp+8h] [ebp-14h]
    DvarLimits ming; // [esp+8h] [ebp-14h]
    DvarLimits minh; // [esp+8h] [ebp-14h]
    DvarLimits mini; // [esp+8h] [ebp-14h]
    DvarLimits minj; // [esp+8h] [ebp-14h]
    DvarLimits mink; // [esp+8h] [ebp-14h]
    DvarLimits minl; // [esp+8h] [ebp-14h]
    DvarLimits minm; // [esp+8h] [ebp-14h]
    DvarLimits minn; // [esp+8h] [ebp-14h]
    DvarLimits mino; // [esp+8h] [ebp-14h]
    DvarLimits minp; // [esp+8h] [ebp-14h]

    g_cheats = Dvar_RegisterBool("sv_cheats", 1, 0, "Enable cheats");
    Dvar_RegisterString("gamename", "KisakCoD4", 0x44u, "The name of the game");
    Dvar_RegisterString("gamedate", __DATE__, 0x40u, "The date compiled");
    Dvar_RegisterString("sv_mapname", (char *)"", 0x44u, "The current map name");
    g_gametype = Dvar_RegisterString("g_gametype", "war", 0x24u, "The current campaign");
    min.integer.max = Dvar_RegisterInt(
        "ui_maxclients",
        32,
        (DvarLimits)0x4000000001LL,
        0x25u,
        "The maximum number of clients that can connect to a server")->current.integer;
    min.enumeration.stringCount = 1;
    g_maxclients = Dvar_RegisterInt(
        "sv_maxclients",
        32,
        min,
        0x25u,
        "The maximum number of clients that can connect to a server");
    g_synchronousClients = Dvar_RegisterBool(
        "g_synchronousClients",
        0,
        8u,
        "Call 'client think' exactly once for each server frame to make smooth demos");
    g_log = Dvar_RegisterString("g_log", "games_mp.log", 1u, "Log file name");
    g_logSync = Dvar_RegisterBool("g_logSync", 0, 1u, "Enable synchronous logging");
    g_password = Dvar_RegisterString("g_password", (char *)"", 0, "Password");
    g_banIPs = Dvar_RegisterString("g_banIPs", (char *)"", 1u, "IP addresses to ban from playing");
#if defined(DEDICATED) || defined(KISAK_DEDICATED)
    g_dedicated = Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 2, 0x20u, "Dedicated server");
#else
    g_dedicated = Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x20u, "Dedicated server");
    if (g_dedicated->current.integer)
        Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 0x40u, "Dedicated server");
#endif

    g_speed = Dvar_RegisterInt("g_speed", 190, (DvarLimits)0x7FFFFFFF80000000LL, 0, "Player speed");
    mina.value.max = FLT_MAX;
    mina.value.min = 1.0;
    g_gravity = Dvar_RegisterFloat("g_gravity", 800.0, mina, 0, "Game gravity in inches per second per second");
    minb.value.max = FLT_MAX;
    minb.value.min = -FLT_MAX;
    g_knockback = Dvar_RegisterFloat("g_knockback", 1000.0, minb, 0, "Maximum knockback");
    g_maxDroppedWeapons = Dvar_RegisterInt(
        "g_maxDroppedWeapons",
        16,
        (DvarLimits)0x2000000002LL,
        0,
        "Maximum number of dropped weapons");
    g_inactivity = Dvar_RegisterInt(
        "g_inactivity",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0,
        "Time delay before player is kicked for inactivity");
    g_debugDamage = Dvar_RegisterBool("g_debugDamage", 0, 0x80u, "Show debug information for damage");
    g_debugBullets = Dvar_RegisterInt(
        "g_debugBullets",
        0,
        (DvarLimits)0x6FFFFFFFDLL,
        0x80u,
        "Show debug information for bullets");
    bullet_penetrationEnabled = Dvar_RegisterBool(
        "bullet_penetrationEnabled",
        1,
        0x80u,
        "Enable/Disable bullet penetration.");
    g_entinfo = Dvar_RegisterEnum("g_entinfo", g_entinfoNames, 0, 0x80u, "Display entity information");
    g_debugPlayerAnimScript = Dvar_RegisterInt(
        "g_debugPlayerAnimScript",
        -1,
        (DvarLimits)0x40FFFFFFFFLL,
        0,
        "Show debug information for playeranim.script");
    g_motd = Dvar_RegisterString("g_motd", (char *)"", 0, "The message of the day");
    g_playerCollisionEjectSpeed = Dvar_RegisterInt(
        "g_playerCollisionEjectSpeed",
        25,
        (DvarLimits)0x7D0000000000LL,
        1u,
        "Speed at which to push intersecting players away from each other");
    minc.value.max = 1000.0;
    minc.value.min = 0.0;
    g_dropForwardSpeed = Dvar_RegisterFloat("g_dropForwardSpeed", 10.0, minc, 1u, "Forward speed of a dropped item");
    mind.value.max = 1000.0;
    mind.value.min = 0.0;
    g_dropUpSpeedBase = Dvar_RegisterFloat(
        "g_dropUpSpeedBase",
        10.0,
        mind,
        1u,
        "Base component of the initial vertical speed of a dropped item");
    mine.value.max = 1000.0;
    mine.value.min = 0.0;
    g_dropUpSpeedRand = Dvar_RegisterFloat(
        "g_dropUpSpeedRand",
        5.0,
        mine,
        1u,
        "Random component of the initial vertical speed of a dropped item");
    minf.value.max = 1000.0;
    minf.value.min = 0.0;
    g_dropHorzSpeedRand = Dvar_RegisterFloat(
        "g_dropHorzSpeedRand",
        100.0,
        minf,
        1u,
        "Random component of the initial horizontal speed of a dropped item");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0;
    g_clonePlayerMaxVelocity = Dvar_RegisterFloat(
        "g_clonePlayerMaxVelocity",
        80.0,
        ming,
        1u,
        "Maximum velocity in each axis of a cloned player\n(for death animations)");
    voice_global = Dvar_RegisterBool("voice_global", 0, 1u, "Send voice messages to everybody");
    voice_localEcho = Dvar_RegisterBool("voice_localEcho", 0, 1u, "Echo voice chat back to the player");
    voice_deadChat = Dvar_RegisterBool("voice_deadChat", 0, 1u, "Allow dead players to talk to living players");
    g_allowVote = Dvar_RegisterBool("g_allowVote", 1, 0, "Enable voting on this server");
    g_listEntity = Dvar_RegisterBool("g_listEntity", 0, 0, "List the entities");
    g_deadChat = Dvar_RegisterBool("g_deadChat", 0, 1u, "Allow dead players to chat with living players");
    g_voiceChatTalkingDuration = Dvar_RegisterInt(
        "g_voiceChatTalkingDuration",
        500,
        (DvarLimits)0x271000000000LL,
        1u,
        "Time after the last talk packet was received that the player is considered by the\n"
        "server to still be talking in milliseconds");
    g_TeamIcon_Allies = Dvar_RegisterString(
        "g_TeamIcon_Allies",
        "faction_128_usmc",
        0x100u,
        "Shader name for the allied scores banner");
    g_TeamIcon_Axis = Dvar_RegisterString(
        "g_TeamIcon_Axis",
        "faction_128_arab",
        0x100u,
        "Shader name for the axis scores banner");
    g_TeamIcon_Free = Dvar_RegisterString(
        "g_TeamIcon_Free",
        (char *)"",
        0x100u,
        "Shader name for the scores of players with no team");
    g_TeamIcon_Spectator = Dvar_RegisterString(
        "g_TeamIcon_Spectator",
        (char *)"",
        0x100u,
        "Shader name for the scores of players who are spectators");
    g_ScoresColor_MyTeam = Dvar_RegisterColor(
        "g_ScoresColor_MyTeam",
        0.25,
        0.72000003,
        0.25,
        1.0,
        0x100u,
        "Player team color on scoreboard");
    g_ScoresColor_EnemyTeam = Dvar_RegisterColor(
        "g_ScoresColor_EnemyTeam",
        0.69,
        0.07,
        0.050000001,
        1.0,
        0x100u,
        "Enemy team color on scoreboard");
    g_ScoresColor_Spectator = Dvar_RegisterColor(
        "g_ScoresColor_Spectator",
        0.25,
        0.25,
        0.25,
        1.0,
        0x100u,
        "Spectator team color on scoreboard");
    g_ScoresColor_Free = Dvar_RegisterColor(
        "g_ScoresColor_Free",
        0.75999999,
        0.77999997,
        0.1,
        1.0,
        0x100u,
        "Free Team color on scoreboard");
    g_ScoresColor_Allies = Dvar_RegisterColor(
        "g_ScoresColor_Allies",
        0.090000004,
        0.46000001,
        0.07,
        1.0,
        0x100u,
        "Allies team color on scoreboard");
    g_ScoresColor_Axis = Dvar_RegisterColor(
        "g_ScoresColor_Axis",
        0.69,
        0.07,
        0.050000001,
        1.0,
        0x100u,
        "Axis team color on scoreboard");
    g_TeamName_Allies = Dvar_RegisterString("g_TeamName_Allies", "GAME_ALLIES", 0x100u, "Allied team name");
    g_TeamName_Axis = Dvar_RegisterString("g_TeamName_Axis", "GAME_AXIS", 0x100u, "Axis team name");
    g_TeamColor_Allies = Dvar_RegisterColor(
        "g_TeamColor_Allies",
        0.60000002,
        0.63999999,
        0.69,
        1.0,
        0x100u,
        "Allies team color");
    g_TeamColor_Axis = Dvar_RegisterColor(
        "g_TeamColor_Axis",
        0.64999998,
        0.56999999,
        0.41,
        1.0,
        0x100u,
        "Axis team color");
    g_TeamColor_MyTeam = Dvar_RegisterColor(
        "g_TeamColor_MyTeam",
        0.40000001,
        0.60000002,
        0.85000002,
        1.0,
        0x100u,
        "Player team color");
    g_TeamColor_EnemyTeam = Dvar_RegisterColor("g_TeamColor_EnemyTeam", 0.75, 0.25, 0.25, 1.0, 0x100u, "Enemy team color");
    g_TeamColor_Spectator = Dvar_RegisterColor(
        "g_TeamColor_Spectator",
        0.25,
        0.25,
        0.25,
        1.0,
        0x100u,
        "Spectator team color");
    g_TeamColor_Free = Dvar_RegisterColor("g_TeamColor_Free", 0.75, 0.25, 0.25, 1.0, 0x100u, "Free Team color");
    g_smoothClients = Dvar_RegisterBool("g_smoothClients", 1, 0, "Enable extrapolation between client states");
    g_antilag = Dvar_RegisterBool("g_antilag", 1, 5u, "Turn on antilag checks for weapon hits");
    g_oldVoting = Dvar_RegisterBool("g_oldVoting", 1, 1u, "Use old voting method");
    minh.value.max = 1.0;
    minh.value.min = 0.0;
    g_voteAbstainWeight = Dvar_RegisterFloat(
        "g_voteAbstainWeight",
        0.5,
        minh,
        1u,
        "How much an abstained vote counts as a 'no' vote");
    g_NoScriptSpam = Dvar_RegisterBool("g_no_script_spam", 0, 0, "Turn off script debugging info");
    g_debugLocDamage = Dvar_RegisterBool(
        "g_debugLocDamage",
        0,
        0x80u,
        "Turn on debugging information for locational damage");
    mini.value.max = 15000.0;
    mini.value.min = 0.0;
    g_friendlyfireDist = Dvar_RegisterFloat(
        "g_friendlyfireDist",
        256.0,
        mini,
        0x80u,
        "Maximum range for disabling fire at a friendly");
    minj.value.max = 15000.0;
    minj.value.min = 0.0;
    g_friendlyNameDist = Dvar_RegisterFloat(
        "g_friendlyNameDist",
        15000.0,
        minj,
        0x80u,
        "Maximum range for seeing a friendly's name");
    melee_debug = Dvar_RegisterBool("melee_debug", 0, 0x80u, "Turn on debug lines for melee traces");
    radius_damage_debug = Dvar_RegisterBool(
        "radius_damage_debug",
        0,
        0x80u,
        "Turn on debug lines for radius damage traces");
    mink.value.max = FLT_MAX;
    mink.value.min = 0.0;
    player_throwbackInnerRadius = Dvar_RegisterFloat(
        "player_throwbackInnerRadius",
        90.0,
        mink,
        0x80u,
        "The radius to a live grenade player must be within initially to do a throwback");
    minl.value.max = FLT_MAX;
    minl.value.min = 0.0;
    player_throwbackOuterRadius = Dvar_RegisterFloat(
        "player_throwbackOuterRadius",
        160.0,
        minl,
        0x80u,
        "The radius player is allow to throwback a grenade once the player has been in the inner radius");
    minm.value.max = FLT_MAX;
    minm.value.min = 0.0;
    player_MGUseRadius = Dvar_RegisterFloat(
        "player_MGUseRadius",
        128.0,
        minm,
        0x80u,
        "The radius within which a player can mount a machine gun");
    minn.value.max = FLT_MAX;
    minn.value.min = 0.0;
    g_minGrenadeDamageSpeed = Dvar_RegisterFloat(
        "g_minGrenadeDamageSpeed",
        400.0,
        minn,
        0x80u,
        "Minimum speed at which getting hit be a grenade will do damage (not the grenade explosion damage)");
    g_compassShowEnemies = Dvar_RegisterBool(
        "g_compassShowEnemies",
        0,
        0x84u,
        "Whether enemies are visible on the compass at all times");
    pickupPrints = Dvar_RegisterBool(
        "pickupPrints",
        0,
        0x80u,
        "Print a message to the game window when picking up ammo, etc.");
    g_dumpAnims = Dvar_RegisterInt(
        "g_dumpAnims",
        -1,
        (DvarLimits)0x3FFFFFFFFFFLL,
        0x80u,
        "Animation debugging info for the given character number");
    g_useholdtime = Dvar_RegisterInt(
        "g_useholdtime",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0,
        "Time to hold the 'use' button to activate use");
    g_useholdspawndelay = Dvar_RegisterInt(
        "g_useholdspawndelay",
        MY_DEFAULT_USEHOLDSPAWNDELAY,
        (DvarLimits)0x3E800000000LL,
        0x81u,
        "Time in milliseconds that the player is unable to 'use' after spawning");
    g_redCrosshairs = Dvar_RegisterBool("g_redCrosshairs", 1, 0x21u, "Whether red crosshairs are enabled");
    g_mantleBlockTimeBuffer = Dvar_RegisterInt(
        "g_mantleBlockTimeBuffer",
        500,
        (DvarLimits)0xEA6000000000LL,
        0x80u,
        "Time that the client think is delayed after mantling");
    anim_deltas_debug = Dvar_RegisterBool("anim_deltas_debug", 0, 0, "Enable animation debug data");
    Helicopter_RegisterDvars();
    G_VehRegisterDvars();
    G_RegisterMissileDvars();
    G_RegisterMissileDebugDvars();
    BG_RegisterDvars();
    g_fogColorReadOnly = Dvar_RegisterColor(
        "g_fogColorReadOnly",
        1.0,
        0.0,
        0.0,
        1.0,
        0x10C0u,
        "Fog color that was set in the most recent call to \"setexpfog\"");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0;
    g_fogStartDistReadOnly = Dvar_RegisterFloat(
        "g_fogStartDistReadOnly",
        0.0,
        mino,
        0x10C0u,
        "Fog start distance that was set in the most recent call to \"setexpfog\"");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0;
    result = Dvar_RegisterFloat(
        "g_fogHalfDistReadOnly",
        0.1,
        minp,
        0x10C0u,
        "Fog start distance that was set in the most recent call to \"setexpfog\"");
    g_fogHalfDistReadOnly = result;
    return result;
}

void __cdecl G_CreateDObj(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle,
    int unusedLocalClientNum)
{
    if (unusedLocalClientNum != -1)
        MyAssertHandler(
            ".\\game_mp\\g_main_mp.cpp",
            836,
            0,
            "unusedLocalClientNum == UNUSED_LOCAL_CLIENT_NUM\n\t%i, %i",
            unusedLocalClientNum,
            -1);
    Com_ServerDObjCreate(dobjModels, numModels, tree, handle);
}

DObj_s *__cdecl G_GetDObj(unsigned int handle, int unusedLocalClientNum)
{
    if (unusedLocalClientNum != -1)
        MyAssertHandler(
            ".\\game_mp\\g_main_mp.cpp",
            843,
            0,
            "unusedLocalClientNum == UNUSED_LOCAL_CLIENT_NUM\n\t%i, %i",
            unusedLocalClientNum,
            -1);
    return Com_GetServerDObj(handle);
}

XAnimTree_s *G_LoadAnimTreeInstances()
{
    XAnimTree_s *result; // eax
    XAnim_s *generic_human; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    result = (XAnimTree_s *)level_bgs.generic_human.tree.anims;
    generic_human = level_bgs.generic_human.tree.anims;
    for (i = 0; i < 64; ++i)
    {
        result = XAnimCreateTree(generic_human, (void *(__cdecl *)(int))Hunk_AllocXAnimServer);
        level_bgs.clientinfo[i].pXAnimTree = result;
    }
    for (ia = 0; ia < 8; ++ia)
    {
        result = XAnimCreateTree(generic_human, (void *(__cdecl *)(int))Hunk_AllocXAnimServer);
        g_scr_data.playerCorpseInfo[ia].tree = result;
    }
    return result;
}

void G_PrintAllFastFileErrors()
{
    if (!sv_mapname)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 973, 0, "%s", "sv_mapname");
    G_PrintFastFileErrors("code_post_gfx_mp");
    G_PrintFastFileErrors("common_mp");
    G_PrintFastFileErrors(sv_mapname->current.string);
}

void __cdecl G_PrintFastFileErrors(const char *fastfile)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    if (!fastfile)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 957, 0, "%s", "fastfile");
    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fastfile).rawfile;
    if (!rawfile)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 960, 1, "%s", "rawfile");
    if (rawfile->len)
    {
        Com_PrintError(1, "There were errors when building fast file '%s'\n", fastfile);
        Com_PrintError(1, (char *)rawfile->buffer);
    }
}

void __cdecl G_ShutdownGame(int freeScripts)
{
    int file; // [esp+0h] [ebp-4h]

    Com_Printf(15, "==== ShutdownGame (%d) ====\n", freeScripts);
    if (level.logFile)
    {
        G_LogPrintf("ShutdownGame:\n");
        G_LogPrintf("------------------------------------------------------------\n");
        FS_FCloseFile(level.logFile);
    }
    bgs = 0;
    G_FreeEntities();
    HudElem_DestroyAll();
    if (Scr_IsSystemActive() && !level.savepersist)
        SV_FreeClientScriptPers();
    Scr_ShutdownSystem(1u, level.savepersist == 0);
    if (freeScripts)
    {
        Mantle_ShutdownAnims();
        GScr_FreeScripts();
        Scr_FreeScripts(1u);
        G_FreeAnimTreeInstances();
        Com_FreeWeaponInfoMemory(1);
        Hunk_ClearToMarkLow(0);
    }
    for (file = 0; file < 1; ++file)
    {
        if (level.openScriptIOFileBuffers[file])
            Z_VirtualFree(level.openScriptIOFileBuffers[file]);
        level.openScriptIOFileBuffers[file] = 0;
        if (level.openScriptIOFileHandles[file])
            FS_FCloseFile(level.openScriptIOFileHandles[file]);
        level.openScriptIOFileHandles[file] = 0;
    }
    EntHandle::Shutdown();
    SV_track_shutdown();
}

int G_FreeAnimTreeInstances()
{
    int result; // eax
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    for (i = 0; i < 64; ++i)
    {
        if (level_bgs.clientinfo[i].pXAnimTree)
        {
            XAnimFreeTree(level_bgs.clientinfo[i].pXAnimTree, 0);
            level_bgs.clientinfo[i].pXAnimTree = 0;
        }
        result = i + 1;
    }
    for (ia = 0; ia < 8; ++ia)
    {
        result = 1244 * ia;
        if (g_scr_data.playerCorpseInfo[ia].tree)
        {
            XAnimFreeTree(g_scr_data.playerCorpseInfo[ia].tree, 0);
            result = 1244 * ia;
            g_scr_data.playerCorpseInfo[ia].tree = 0;
        }
    }
    return result;
}

void __cdecl SendScoreboardMessageToAllIntermissionClients()
{
    int i; // [esp+4h] [ebp-4h]

    if (level.bUpdateScoresForIntermission)
    {
        for (i = 0; i < level.maxclients; ++i)
        {
            if (level.clients[i].sess.connected == CON_CONNECTED && level.clients[i].ps.pm_type == 5)
                SendScoreboard(&g_entities[i]);
        }
        level.bUpdateScoresForIntermission = 0;
    }
}

void __cdecl CalculateRanks()
{
    int i; // [esp+0h] [ebp-4h]

    level.numConnectedClients = 0;
    level.numVotingClients = 0;
    for (i = 0; i < level.maxclients; ++i)
    {
        if (level.clients[i].sess.connected)
        {
            level.sortedClients[level.numConnectedClients++] = i;
            if (level.clients[i].sess.cs.team != TEAM_SPECTATOR && level.clients[i].sess.connected == CON_CONNECTED)
                ++level.numVotingClients;
        }
    }
    qsort(level.sortedClients, level.numConnectedClients, 4u, (int(__cdecl *)(const void *, const void *))SortRanks);
    level.bUpdateScoresForIntermission = 1;
}

int __cdecl SortRanks(unsigned int *a, unsigned int *b)
{
    gclient_s *cb; // [esp+0h] [ebp-8h]
    gclient_s *ca; // [esp+4h] [ebp-4h]

    ca = &level.clients[*a];
    cb = &level.clients[*b];
    if (ca->sess.connected == CON_CONNECTING)
        return 1;
    if (cb->sess.connected == CON_CONNECTING)
        return -1;
    if (ca->sess.cs.team == TEAM_SPECTATOR && cb->sess.cs.team == TEAM_SPECTATOR)
    {
        if (ca >= cb)
            return ca > cb;
        else
            return -1;
    }
    else if (ca->sess.cs.team == TEAM_SPECTATOR)
    {
        return 1;
    }
    else if (cb->sess.cs.team == TEAM_SPECTATOR)
    {
        return -1;
    }
    else if (ca->sess.score <= cb->sess.score)
    {
        if (ca->sess.score >= cb->sess.score)
        {
            if (ca->sess.deaths >= cb->sess.deaths)
                return ca->sess.deaths > cb->sess.deaths;
            else
                return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return -1;
    }
}

void __cdecl ExitLevel()
{
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    Cbuf_AddText(0, "map_rotate\n");
    level.teamScores[1] = 0;
    level.teamScores[2] = 0;
    for (i = 0; i < g_maxclients->current.integer; ++i)
    {
        if (level.clients[i].sess.connected == CON_CONNECTED)
            level.clients[i].sess.score = 0;
    }
    for (ia = 0; ia < g_maxclients->current.integer; ++ia)
    {
        if (level.clients[ia].sess.connected == CON_CONNECTED)
            level.clients[ia].sess.connected = CON_CONNECTING;
    }
    G_LogPrintf("ExitLevel: executed\n");
}

void G_LogPrintf(const char *fmt, ...)
{
    char string[1024]; // [esp+10h] [ebp-818h] BYREF
    char *argptr; // [esp+410h] [ebp-418h]
    int tens; // [esp+414h] [ebp-414h]
    char string2[1028]; // [esp+418h] [ebp-410h] BYREF
    int min; // [esp+820h] [ebp-8h]
    int sec; // [esp+824h] [ebp-4h]
    va_list va; // [esp+834h] [ebp+Ch] BYREF

    va_start(va, fmt);
    if (level.logFile)
    {
        _vsnprintf(string2, 0x400u, fmt, va);
        argptr = 0;
        min = level.time / 1000 / 60;
        tens = level.time / 1000 % 60 / 10;
        sec = level.time / 1000 % 60 % 10;
        Com_sprintf(string, 0x400u, "%3i:%i%i %s", min, tens, sec, string2);
        FS_Write(string, &string[strlen(string) + 1] - &string[1], level.logFile);
    }
}

void __cdecl CheckVote()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    float v3; // [esp+8h] [ebp-20h]
    int passCount; // [esp+24h] [ebp-4h]

    if (level.voteExecuteTime)
    {
        if (level.voteExecuteTime < level.time)
        {
            level.voteExecuteTime = 0;
            v0 = va("%s\n", level.voteString);
            Cbuf_AddText(0, v0);
        }
    }
    if (level.voteTime)
    {
        if (level.time - level.voteTime >= 0)
        {
            v3 = (double)(level.numVotingClients - (level.voteNo + level.voteYes)) * g_voteAbstainWeight->current.value;
            if (level.voteYes <= (int)(v3 + 0.4999999990686774) + level.voteNo)
                goto LABEL_11;
        LABEL_9:
            v1 = va("%c \"GAME_VOTEPASSED\"", 101);
            SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v1);
            level.voteExecuteTime = level.time + 3000;
        LABEL_13:
            level.voteTime = 0;
            SV_SetConfigstring(13, (char *)"");
            return;
        }
        passCount = level.numVotingClients / 2 + 1;
        if (level.voteYes >= passCount)
            goto LABEL_9;
        if (level.voteNo > level.numVotingClients - passCount)
        {
        LABEL_11:
            v2 = va("%c \"GAME_VOTEFAILED\"", 101);
            SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v2);
            goto LABEL_13;
        }
    }
}

void __cdecl G_UpdateObjectiveToClients()
{
    objective_t *obj; // [esp+8h] [ebp-18h]
    int team; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+10h] [ebp-10h]
    int clientNum; // [esp+14h] [ebp-Ch]
    int objNum; // [esp+18h] [ebp-8h]
    playerState_s *ps; // [esp+1Ch] [ebp-4h]

    for (clientNum = 0; clientNum < level.maxclients; ++clientNum)
    {
        ent = &level.gentities[clientNum];
        if (ent->r.inuse)
        {
            if (!ent->client)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1544, 0, "%s", "ent->client");
            ps = &ent->client->ps;
            team = ps[1].movementDir;
            for (objNum = 0; objNum < 16; ++objNum)
            {
                obj = &level.objectives[objNum];
                if (obj->state && (!obj->teamNum || obj->teamNum == team))
                    memcpy(&ps->objective[objNum], obj, sizeof(ps->objective[objNum]));
                else
                    ps->objective[objNum].state = OBJST_EMPTY;
            }
        }
    }
}

void __cdecl G_UpdateHudElemsToClients()
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int clientNum; // [esp+4h] [ebp-4h]

    for (clientNum = 0; clientNum < level.maxclients; ++clientNum)
    {
        ent = &level.gentities[clientNum];
        if (ent->r.inuse)
        {
            if (!ent->client)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1575, 0, "%s", "ent->client");
            HudElem_UpdateClient(ent->client, ent->s.number, HUDELEM_UPDATE_ARCHIVAL_AND_CURRENT);
        }
    }
}

void __cdecl G_RunThink(gentity_s *ent)
{
    void(__cdecl * think)(gentity_s *); // [esp+0h] [ebp-8h]
    int thinktime; // [esp+4h] [ebp-4h]

    thinktime = ent->nextthink;
    if (thinktime > 0 && thinktime <= level.time)
    {
        ent->nextthink = 0;
        think = entityHandlers[ent->handler].think;
        if (!think)
            Com_Error(ERR_DROP, "NULL ent->think");
        think(ent);
    }
}

void __cdecl DebugDumpAnims()
{
    if (g_dumpAnims->current.integer >= 0)
    {
        if (g_dumpAnims->current.integer >= 1024)
            MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1617, 1, "%s", "g_dumpAnims->current.integer < MAX_GENTITIES");
        SV_DObjDisplayAnim(&level.gentities[g_dumpAnims->current.integer], "server:\n");
    }
}

void __cdecl G_XAnimUpdateEnt(gentity_s *ent)
{
    while (ent->r.inuse && (ent->flags & 0x2000) == 0 && G_DObjUpdateServerTime(ent, 1))
        Scr_RunCurrentThreads();
}

void __cdecl TeamplayInfoMessage(gentity_s *ent)
{
    ent->client->ps.stats[3] = -1;
}

void __cdecl CheckTeamStatus()
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (level.time - level.lastTeammateHealthTime > 0)
    {
        level.lastTeammateHealthTime = level.time;
        for (i = 0; i < g_maxclients->current.integer; ++i)
        {
            ent = &g_entities[i];
            if (ent->r.inuse)
            {
                if ((ent->client->ps.otherFlags & 2) == 0)
                    TeamplayInfoMessage(ent);
            }
        }
    }
}

void __cdecl G_RunFrame(int levelTime)
{
    trigger_info_t *v1; // ecx
    char *v2; // eax
    float dtime; // [esp+4h] [ebp-530h]
    gentity_s *other; // [esp+110h] [ebp-424h]
    trigger_info_t *trigger_info; // [esp+114h] [ebp-420h]
    unsigned __int8 index; // [esp+11Bh] [ebp-419h]
    unsigned __int8 entIndex[1028]; // [esp+11Ch] [ebp-418h] BYREF
    gentity_s *ent; // [esp+524h] [ebp-10h]
    int bMoreTriggered; // [esp+528h] [ebp-Ch]
    int i; // [esp+52Ch] [ebp-8h]
    int entnum; // [esp+530h] [ebp-4h]

    Profile_Begin(230);
    SV_CheckThread();
    ++level.framenum;
    level.previousTime = level.time;
    level.time = levelTime;
    level.frametime = levelTime - level.previousTime;
    level_bgs.time = levelTime;
    level_bgs.latestSnapshotTime = levelTime;
    level_bgs.frametime = levelTime - level.previousTime;
    if (bgs)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1880, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
    bgs = &level_bgs;
    if (level.frametime < 0)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1883, 0, "%s", "level.frametime >= 0");
    ent = g_entities;
    i = 0;
    while (i < level.maxclients)
    {
        if (ent->r.inuse)
        {
            if (!ent->client)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1890, 0, "%s", "ent->client");
            if (!ent->client->noclip)
                G_TouchTriggers(ent);
        }
        ++i;
        ++ent;
    }
    Profile_Begin(320);
    ent = g_entities;
    i = 0;
    while (i < level.num_entities)
    {
        if (ent->r.inuse)
        {
            dtime = (double)level.frametime * EQUAL_EPSILON;
            SV_DObjInitServerTime(ent, dtime);
        }
        ++i;
        ++ent;
    }
    Profile_EndInternal(0);
    memset(entIndex, 0, 0x400u);
    index = 0;
    Profile_Begin(271);
    if (level.currentTriggerListSize)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1914, 0, "%s", "level.currentTriggerListSize == 0");
    Com_Memcpy((char *)level.currentTriggerList, (char *)level.pendingTriggerList, 12 * level.pendingTriggerListSize);
    level.currentTriggerListSize = level.pendingTriggerListSize;
    level.pendingTriggerListSize = 0;
    do
    {
        bMoreTriggered = 0;
        ++index;
        for (i = 0; i < level.currentTriggerListSize; ++i)
        {
            trigger_info = &level.currentTriggerList[i];
            entnum = trigger_info->entnum;
            ent = &g_entities[entnum];
            if (ent->useCount == trigger_info->useCount)
            {
                if (!ent->r.inuse)
                    MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1930, 0, "%s", "ent->r.inuse");
                other = &g_entities[trigger_info->otherEntnum];
                if (other->useCount == trigger_info->otherUseCount)
                {
                    if (!other->r.inuse)
                        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1934, 0, "%s", "other->r.inuse");
                    if (entIndex[entnum] == index)
                    {
                        bMoreTriggered = 1;
                        continue;
                    }
                    entIndex[entnum] = index;
                    Scr_AddEntity(other);
                    Scr_Notify(ent, scr_const.trigger, 1u);
                }
            }
            --level.currentTriggerListSize;
            --i;
            v1 = &level.currentTriggerList[level.currentTriggerListSize];
            *(unsigned int *)&trigger_info->entnum = *(unsigned int *)&v1->entnum;
            trigger_info->useCount = v1->useCount;
            trigger_info->otherUseCount = v1->otherUseCount;
        }
        Profile_Begin(272);
        Scr_RunCurrentThreads();
        Profile_EndInternal(0);
    } while (bMoreTriggered);
    if (level.currentTriggerListSize)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1956, 0, "%s", "level.currentTriggerListSize == 0");
    Profile_EndInternal(0);
    ent = g_entities;
    i = 0;
    while (i < level.maxclients)
    {
        if (ent->r.inuse)
            G_ClientDoPerFrameNotifies(ent);
        ++i;
        ++ent;
    }
    Profile_Begin(320);
    ent = g_entities;
    i = 0;
    while (i < level.num_entities)
    {
        G_XAnimUpdateEnt(ent);
        ++i;
        ++ent;
    }
    Profile_EndInternal(0);
    Scr_IncTime();
    SV_ResetSkeletonCache();
    Profile_Begin(260);
    if (level.currentEntityThink != -1)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1984, 0, "%s", "level.currentEntityThink == -1");
    ent = g_entities;
    level.currentEntityThink = 0;
    while (level.currentEntityThink < level.num_entities)
    {
        if (ent->r.inuse)
            G_RunFrameForEntity(ent);
        ++level.currentEntityThink;
        ++ent;
    }
    level.currentEntityThink = -1;
    Profile_EndInternal(0);
    Profile_Begin(263);
    G_UpdateObjectiveToClients();
    Profile_EndInternal(0);
    Profile_Begin(264);
    G_UpdateHudElemsToClients();
    Profile_EndInternal(0);
    Profile_Begin(265);
    ent = g_entities;
    i = 0;
    while (i < level.maxclients)
    {
        if (ent->r.inuse)
        {
            if ((unsigned int)i >= 0x40)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 2012, 0, "i doesn't index MAX_CLIENTS\n\t%i not in [0, %i)", i, 64);
            if (!level_bgs.clientinfo[i].infoValid)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 2013, 0, "%s", "level_bgs.clientinfo[i].infoValid");
            if (ent->client - level.clients != i)
                MyAssertHandler(
                    ".\\game_mp\\g_main_mp.cpp",
                    2014,
                    0,
                    "ent->client - level.clients == i\n\t%i, %i",
                    ent->client - level.clients,
                    i);
            ClientEndFrame(ent);
        }
        ++i;
        ++ent;
    }
    Profile_EndInternal(0);
    CheckTeamStatus();
    if (g_oldVoting->current.enabled)
        CheckVote();
    SendScoreboardMessageToAllIntermissionClients();
    if (g_listEntity->current.enabled)
    {
        for (i = 0; i < 1024; ++i)
        {
            v2 = SL_ConvertToString(g_entities[i].classname);
            Com_Printf(15, "%4i: %s\n", i, v2);
        }
        Dvar_SetBool((dvar_s *)g_listEntity, 0);
    }
    if (level.registerWeapons)
        SaveRegisteredWeapons();
    if (level.bRegisterItems)
        SaveRegisteredItems();
    DebugDumpAnims();
    if (bgs != &level_bgs)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 2043, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
    bgs = 0;
    ShowEntityInfo();
    Profile_EndInternal(0);
}

void __cdecl G_ClientDoPerFrameNotifies(gentity_s *ent)
{
    WeaponDef *WeaponDef; // eax
    bool v2; // al
    bool IsSprinting; // al
    bool previouslySprinting; // [esp-Ch] [ebp-14h]
    unsigned __int16 sprint_begin; // [esp-8h] [ebp-10h]
    unsigned __int16 sprint_end; // [esp-4h] [ebp-Ch]
    gclient_s *client; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1649, 0, "%s", "ent");
    client = ent->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1653, 0, "%s", "client");
    if (client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1654, 0, "%s", "client->sess.connected != CON_DISCONNECTED");
    if (client->ps.weapon != client->lastWeapon)
    {
        WeaponDef = BG_GetWeaponDef(client->ps.weapon);
        Scr_AddString((char *)WeaponDef->szInternalName);
        Scr_Notify(ent, scr_const.weapon_change, 1u);
        client->lastWeapon = client->ps.weapon;
    }
    if (client->ps.weaponstate == 5 && client->ps.pm_type < 7)
        v2 = DoPerFrameNotify(ent, 1, client->previouslyFiring, scr_const.begin_firing, scr_const.end_firing);
    else
        v2 = DoPerFrameNotify(ent, 0, client->previouslyFiring, scr_const.begin_firing, scr_const.end_firing);
    client->previouslyFiring = v2;
    client->previouslyUsingNightVision = DoPerFrameNotify(
        ent,
        (client->ps.weapFlags & 0x40) != 0,
        client->previouslyUsingNightVision,
        scr_const.night_vision_on,
        scr_const.night_vision_off);
    sprint_end = scr_const.sprint_end;
    sprint_begin = scr_const.sprint_begin;
    previouslySprinting = client->previouslySprinting;
    IsSprinting = PM_IsSprinting(&client->ps);
    client->previouslySprinting = DoPerFrameNotify(ent, IsSprinting, previouslySprinting, sprint_begin, sprint_end);
}

bool __cdecl DoPerFrameNotify(
    gentity_s *ent,
    bool isCurrently,
    bool wasPreviously,
    unsigned __int16 begin,
    unsigned __int16 end)
{
    if (isCurrently == wasPreviously)
        return wasPreviously;
    if (isCurrently)
        Scr_Notify(ent, begin, 0);
    else
        Scr_Notify(ent, end, 0);
    return isCurrently;
}

const dvar_s *ShowEntityInfo()
{
#ifndef DEDICATED
    const dvar_s *result; // eax
    const char *EntityTypeName; // eax
    float origin[3]; // [esp+14h] [ebp-18h] BYREF
    const char *text; // [esp+20h] [ebp-Ch]
    gentity_s *ent; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    result = g_entinfo;
    if (g_entinfo->current.integer)
    {
        ent = g_entities;
        i = 0;
        while (1)
        {
            result = (const dvar_s *)i;
            if (i >= level.num_entities)
                break;
            if (ent->s.eType < 17 && ent->r.inuse && ent->r.linked)
            {
                if (ent->s.eType == 3)
                {
                    ShowEntityInfo_Items(ent);
                }
                else
                {
                    origin[0] = ent->r.currentOrigin[0];
                    origin[1] = ent->r.currentOrigin[1];
                    origin[2] = ent->r.currentOrigin[2];
                    EntityTypeName = G_GetEntityTypeName(ent);
                    text = va("#%i; \"%s\"", i, EntityTypeName);
                    CL_AddDebugString(origin, colorWhiteFaded, 0.5, (char *)text, 1, 1);
                    origin[2] = origin[2] - 10.0;
                    text = va("ground: #%i", ent->s.groundEntityNum);
                    CL_AddDebugString(origin, colorWhiteFaded, 0.5, (char *)text, 1, 1);
                }
            }
            ++i;
            ++ent;
        }
    }
    return result;
#else
    return NULL;
#endif
}

void __cdecl ShowEntityInfo_Items(gentity_s *ent)
{
#ifndef DEDICATED
    WeaponDef *weapDef; // [esp+14h] [ebp-18h]
    char *text; // [esp+18h] [ebp-14h]
    float origin[3]; // [esp+1Ch] [ebp-10h] BYREF
    int idx; // [esp+28h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1678, 0, "%s", "ent");
    if (ent == (gentity_s *)-436)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1679, 0, "%s", "ent->item");
    origin[0] = ent->r.currentOrigin[0];
    origin[1] = ent->r.currentOrigin[1];
    origin[2] = ent->r.currentOrigin[2] - -4.0 * 0.5;
    for (idx = 0; idx < 2; ++idx)
    {
        if (ent->item[idx].index)
        {
            weapDef = BG_GetWeaponDef(ent->item[idx].index);
            text = va("%s (%i + %i)", weapDef->szInternalName, ent->item[idx].clipAmmoCount, ent->item[idx].ammoCount);
            CL_AddDebugString(origin, colorRedFaded, 0.30000001, text, 1, 1);
            origin[2] = origin[2] + -4.0;
        }
    }
#endif
}

void __cdecl G_RunFrameForEntity(gentity_s *ent)
{
    char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    const char *v6; // eax

    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1743, 0, "%s", "ent->r.inuse");
    if (ent->processedFrame != level.framenum)
    {
        ent->processedFrame = level.framenum;
        if (ent->tagInfo)
        {
            if (!ent->tagInfo->parent)
                MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1752, 0, "%s", "ent->tagInfo->parent");
            G_RunFrameForEntity(ent->tagInfo->parent);
        }
        if ((ent->r.svFlags & 6) == 6)
            MyAssertHandler(
                ".\\game_mp\\g_main_mp.cpp",
                1756,
                0,
                "%s\n\t(ent->s.number) = %i",
                "((ent->r.svFlags & ((1<<1) | (1<<2))) != ((1<<1) | (1<<2)))",
                ent->s.number);
        if (ent->r.mins[0] > (double)ent->r.maxs[0])
        {
            v1 = SL_ConvertToString(ent->classname);
            v2 = va(
                "entnum: %d, origin: %g %g %g, classname: %s",
                ent->s.number,
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2],
                v1);
            MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1758, 0, "%s\n\t%s", "ent->r.maxs[0] >= ent->r.mins[0]", v2);
        }
        if (ent->r.mins[1] > (double)ent->r.maxs[1])
        {
            v3 = SL_ConvertToString(ent->classname);
            v4 = va(
                "entnum: %d, origin: %g %g %g, classname: %s",
                ent->s.number,
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2],
                v3);
            MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1759, 0, "%s\n\t%s", "ent->r.maxs[1] >= ent->r.mins[1]", v4);
        }
        if (ent->r.mins[2] > (double)ent->r.maxs[2])
        {
            v5 = SL_ConvertToString(ent->classname);
            v6 = va(
                "entnum: %d, origin: %g %g %g, classname: %s",
                ent->s.number,
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2],
                v5);
            MyAssertHandler(".\\game_mp\\g_main_mp.cpp", 1760, 0, "%s\n\t%s", "ent->r.maxs[2] >= ent->r.mins[2]", v6);
        }
        if ((ent->s.lerp.eFlags & 0x10000) != 0 && level.time > ent->s.time2)
            goto LABEL_18;
        if (level.time - ent->eventTime > 300)
        {
            if (ent->freeAfterEvent)
            {
            LABEL_18:
                G_FreeEntity(ent);
                return;
            }
            if (ent->unlinkAfterEvent)
            {
                ent->unlinkAfterEvent = 0;
                SV_UnlinkEntity(ent);
            }
        }
        if (!ent->freeAfterEvent)
        {
            switch (ent->s.eType)
            {
            case 4:
                G_RunMissile(ent);
                return;
            case 3:
                if (ent->tagInfo)
                {
                    G_GeneralLink(ent);
                    G_RunThink(ent);
                    return;
                }
            LABEL_33:
                G_RunItem(ent);
                return;
            case 2:
                G_RunCorpse(ent);
                return;
            }
            if (ent->physicsObject)
                goto LABEL_33;
            if (ent->s.eType == 6 || ent->s.eType == 13 || ent->s.eType == 10)
            {
                G_RunMover(ent);
            }
            else if (ent->client)
            {
                G_RunClient(ent);
            }
            else
            {
                if (!ent->s.eType)
                {
                    if (ent->tagInfo)
                        G_GeneralLink(ent);
                }
                G_RunThink(ent);
            }
        }
    }
}

void __cdecl G_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask)
{
    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-Ch] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    SV_Trace(results, (float*)start, (float *)mins, (float *)maxs, (float *)end, &ignoreEntParams, contentmask, 0, 0, 0);
}

int __cdecl G_TraceCapsuleComplete(
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int passEntityNum,
    int contentmask)
{
    return SV_TracePassed(start, mins, maxs, end, passEntityNum, 1023, contentmask, 0, 0, 0);
}

void __cdecl G_LocationalTrace(
    trace_t *results,
    float *start,
    float *end,
    int passEntityNum,
    int contentmask,
    unsigned __int8 *priorityMap)
{
    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-Ch] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    SV_Trace(
        results,
        start,
        (float *)vec3_origin,
        (float *)vec3_origin,
        end,
        &ignoreEntParams,
        contentmask,
        1,
        priorityMap,
        1);
}

void __cdecl G_LocationalTraceAllowChildren(
    trace_t *results,
    float *start,
    float *end,
    int passEntityNum,
    int contentmask,
    unsigned __int8 *priorityMap)
{
    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-Ch] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    ignoreEntParams.ignoreChildren = 0;
    SV_Trace(
        results,
        start,
        (float *)vec3_origin,
        (float *)vec3_origin,
        end,
        &ignoreEntParams,
        contentmask,
        1,
        priorityMap,
        1);
}

int __cdecl G_LocationalTracePassed(
    float *start,
    float *end,
    int passEntityNum,
    int passEntityNum1,
    int contentmask,
    unsigned __int8 *priorityMap)
{
    return SV_TracePassed(
        start,
        (float *)vec3_origin,
        (float *)vec3_origin,
        end,
        passEntityNum,
        passEntityNum1,
        contentmask,
        1,
        priorityMap,
        1);
}

void __cdecl G_SightTrace(int *hitNum, float *start, float *end, int passEntityNum, int contentmask)
{
    SV_SightTrace(hitNum, start, (float *)vec3_origin, (float *)vec3_origin, end, passEntityNum, 1023, contentmask);
}

void __cdecl G_AddDebugString(const float *xyz, const float *color, float scale, const char *text, int duration)
{
#ifndef DEDICATED
    CL_AddDebugString(xyz, color, scale, text, 1, duration);
#endif
}

BOOL __cdecl OnSameTeam(struct gentity_s *ent1, struct gentity_s *ent2)
{
    if (!ent1->client || !ent2->client)
        return 0;
    if (ent1->client->sess.cs.team)
        return ent1->client->sess.cs.team == ent2->client->sess.cs.team;
    return 0;
}
