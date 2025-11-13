#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"

#include "g_utils_mp.h"

#include <client/client.h>
#include <client_mp/client_mp.h>

#include <database/database.h>

#include <server/sv_game.h>
#include <server_mp/server_mp.h>

#include <script/scr_animtree.h>
#include <script/scr_const.h>
#include <script/scr_main.h>
#include <script/scr_stringlist.h>
#include <script/scr_variable.h>
#include <script/scr_vm.h>

#include <universal/com_sndalias.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <xanim/xanim.h>
#include <qcommon/mem_track.h>
#include <stringed/stringed_hooks.h>
#include <server/sv_world.h>
#include <universal/surfaceflags.h>
#include <qcommon/cmd.h>
#include <universal/com_files.h>

//   struct BuiltinFunctionDef *functions 827b5700     g_scr_main_mp.obj
//   struct scr_data_t g_scr_data 82e8a4b8     g_scr_main_mp.obj
scr_data_t g_scr_data;

BuiltinFunctionDef functions[] =
{
  { "createprintchannel", &GScr_CreatePrintChannel, 1 },
  { "setprintchannel", &GScr_printChannelSet, 1 },
  { "print", &print, 1 },
  { "println", &println, 1 },
  { "iprintln", &iprintln, 0 },
  { "iprintlnbold", &iprintlnbold, 0 },
  { "print3d", &GScr_print3d, 1 },
  { "line", &GScr_line, 1 },
  { "logstring", &KISAK_NULLSUB, 0 },
  { "getent", &Scr_GetEnt, 0 },
  { "getentarray", &Scr_GetEntArray, 0 },
  { "spawn", &GScr_Spawn, 0 },
  { "spawnplane", &GScr_SpawnPlane, 0 },
  { "spawnturret", &GScr_SpawnTurret, 0 },
  { "precacheturret", (void(__cdecl *)())&GScr_PrecacheTurret, 0},
  { "spawnstruct", &Scr_AddStruct, 0 },
  { "spawnhelicopter", &GScr_SpawnHelicopter, 0 },
  { "assert", &assertCmd, 1 },
  { "assertex", &assertexCmd, 1 },
  { "assertmsg", &assertmsgCmd, 1 },
  { "isdefined", &GScr_IsDefined, 0 },
  { "isstring", &GScr_IsString, 0 },
  { "isalive", &GScr_IsAlive, 0 },
  { "getdvar", &GScr_GetDvar, 0 },
  { "getdvarint", &GScr_GetDvarInt, 0 },
  { "getdvarfloat", &GScr_GetDvarFloat, 0 },
  { "setdvar", &GScr_SetDvar, 0 },
  { "gettime", &GScr_GetTime, 0 },
  { "getentbynum", &Scr_GetEntByNum, 1 },
  { "getweaponmodel", &Scr_GetWeaponModel, 0 },
  { "getanimlength", &GScr_GetAnimLength, 0 },
  { "animhasnotetrack", &GScr_AnimHasNotetrack, 0 },
  { "getnotetracktimes", &GScr_GetNotetrackTimes, 0 },
  { "getbrushmodelcenter", &GScr_GetBrushModelCenter, 0 },
  { "objective_add", &Scr_Objective_Add, 0 },
  { "objective_delete", &Scr_Objective_Delete, 0 },
  { "objective_state", &Scr_Objective_State, 0 },
  { "objective_icon", &Scr_Objective_Icon, 0 },
  { "objective_position", &Scr_Objective_Position, 0 },
  { "objective_onentity",  (void(__cdecl *)()) & Scr_Objective_OnEntity, 0 },
  { "objective_current", &Scr_Objective_Current, 0 },
  { "missile_createattractorent", &Scr_MissileCreateAttractorEnt, 0 },
  { "missile_createattractororigin", &Scr_MissileCreateAttractorOrigin, 0 },
  { "missile_createrepulsorent", &Scr_MissileCreateRepulsorEnt, 0 },
  { "missile_createrepulsororigin", &Scr_MissileCreateRepulsorOrigin, 0 },
  { "missile_deleteattractor", &Scr_MissileDeleteAttractor, 0 },
  { "bullettrace", &Scr_BulletTrace, 0 },
  { "bullettracepassed", &Scr_BulletTracePassed, 0 },
  { "sighttracepassed", &Scr_SightTracePassed, 0 },
  { "physicstrace", &Scr_PhysicsTrace, 0 },
  { "playerphysicstrace", &Scr_PlayerPhysicsTrace, 0 },
  { "getmovedelta", &GScr_GetMoveDelta, 0 },
  { "getangledelta", &GScr_GetAngleDelta, 0 },
  { "getnorthyaw", &GScr_GetNorthYaw, 0 },
  { "randomint", &Scr_RandomInt, 0 },
  { "randomfloat", &Scr_RandomFloat, 0 },
  { "randomintrange", &Scr_RandomIntRange, 0 },
  { "randomfloatrange", &Scr_RandomFloatRange, 0 },
  { "sin", &GScr_sin, 0 },
  { "cos", &GScr_cos, 0 },
  { "tan", &GScr_tan, 0 },
  { "asin", &GScr_asin, 0 },
  { "acos", &GScr_acos, 0 },
  { "atan", &GScr_atan, 0 },
  { "int", &GScr_CastInt, 0 },
  { "abs", &GScr_abs, 0 },
  { "min", &GScr_min, 0 },
  { "max", &GScr_max, 0 },
  { "floor", &GScr_floor, 0 },
  { "ceil", &GScr_ceil, 0 },
  { "sqrt", &GScr_sqrt, 0 },
  { "vectorfromlinetopoint", &GScr_VectorFromLineToPoint, 0 },
  { "pointonsegmentnearesttopoint", &GScr_PointOnSegmentNearestToPoint, 0 },
  { "distance", &Scr_Distance, 0 },
  { "distance2d", &Scr_Distance2D, 0 },
  { "distancesquared", &Scr_DistanceSquared, 0 },
  { "length", &Scr_Length, 0 },
  { "lengthsquared", &Scr_LengthSquared, 0 },
  { "closer", &Scr_Closer, 0 },
  { "vectordot", &Scr_VectorDot, 0 },
  { "vectornormalize", &Scr_VectorNormalize, 0 },
  { "vectortoangles", &Scr_VectorToAngles, 0 },
  { "vectorlerp", &Scr_VectorLerp, 0 },
  { "anglestoup", &Scr_AnglesToUp, 0 },
  { "anglestoright", &Scr_AnglesToRight, 0 },
  { "anglestoforward", &Scr_AnglesToForward, 0 },
  { "combineangles", &Scr_CombineAngles, 0 },
  { "issubstr", &Scr_IsSubStr, 0 },
  { "getsubstr", &Scr_GetSubStr, 0 },
  { "tolower", &Scr_ToLower, 0 },
  { "strtok", &Scr_StrTok, 0 },
  { "musicplay", &Scr_MusicPlay, 0 },
  { "musicstop", &Scr_MusicStop, 0 },
  { "soundfade", &Scr_SoundFade, 0 },
  { "ambientplay", &Scr_AmbientPlay, 0 },
  { "ambientstop", &Scr_AmbientStop, 0 },
  { "precachemodel", (void(__cdecl *)()) & Scr_PrecacheModel, 0 },
  { "precacheshellshock", &Scr_PrecacheShellShock, 0 },
  { "precacheitem", &Scr_PrecacheItem, 0 },
  { "precacheshader", (void(__cdecl *)()) & Scr_PrecacheShader, 0 },
  { "precachestring", (void(__cdecl *)()) & Scr_PrecacheString, 0 },
  { "precacherumble", &KISAK_NULLSUB, 0 },
  { "loadfx", &Scr_LoadFX, 0 },
  { "playfx", &Scr_PlayFX, 0 },
  { "playfxontag", &Scr_PlayFXOnTag, 0 },
  { "playloopedfx", &Scr_PlayLoopedFX, 0 },
  { "spawnfx", &Scr_SpawnFX, 0 },
  { "triggerfx", &Scr_TriggerFX, 0 },
  { "physicsexplosionsphere", &Scr_PhysicsExplosionSphere, 0 },
  { "physicsexplosioncylinder", &Scr_PhysicsExplosionCylinder, 0 },
  { "physicsjolt", &Scr_PhysicsRadiusJolt, 0 },
  { "physicsjitter", &Scr_PhysicsRadiusJitter, 0 },
  { "setexpfog", &Scr_SetExponentialFog, 0 },
  { "grenadeexplosioneffect", &Scr_GrenadeExplosionEffect, 0 },
  { "radiusdamage", &GScr_RadiusDamage, 0 },
  { "setplayerignoreradiusdamage", (void(__cdecl *)()) & GScr_SetPlayerIgnoreRadiusDamage, 0 },
  { "getnumparts", &GScr_GetNumParts, 0 },
  { "getpartname", &GScr_GetPartName, 0 },
  { "earthquake", (void(__cdecl *)()) & GScr_Earthquake, 0 },
  { "newhudelem", &GScr_NewHudElem, 0 },
  { "newclienthudelem", &GScr_NewClientHudElem, 0 },
  { "newteamhudelem", &GScr_NewTeamHudElem, 0 },
  { "resettimeout", &Scr_ResetTimeout, 0 },
  { "weaponfiretime", &GScr_WeaponFireTime, 0 },
  { "isweaponcliponly", &GScr_IsWeaponClipOnly, 0 },
  { "isweapondetonationtimed", &GScr_IsWeaponDetonationTimed, 0 },
  { "weaponfiretime", &GScr_WeaponFireTime, 0 },
  { "weaponclipsize", &GScr_WeaponClipSize, 0 },
  { "weaponissemiauto", &GScr_WeaponIsSemiAuto, 0 },
  { "weaponisboltaction", &GScr_WeaponIsBoltAction, 0 },
  { "weapontype", &GScr_WeaponType, 0 },
  { "weaponclass", &GScr_WeaponClass, 0 },
  { "weaponinventorytype", &GScr_WeaponInventoryType, 0 },
  { "weaponstartammo", &GScr_WeaponStartAmmo, 0 },
  { "weaponmaxammo", &GScr_WeaponMaxAmmo, 0 },
  { "weaponaltweaponname", &GScr_WeaponAltWeaponName, 0 },
  { "isplayer", &GScr_IsPlayer, 0 },
  { "isplayernumber", &GScr_IsPlayerNumber, 0 },
  { "setwinningplayer", &GScr_SetWinningPlayer, 0 },
  { "setwinningteam", &GScr_SetWinningTeam, 0 },
  { "announcement", &GScr_Announcement, 0 },
  { "clientannouncement", &GScr_ClientAnnouncement, 0 },
  { "getteamscore", &GScr_GetTeamScore, 0 },
  { "setteamscore", &GScr_SetTeamScore, 0 },
  { "setclientnamemode", &GScr_SetClientNameMode, 0 },
  { "updateclientnames", &GScr_UpdateClientNames, 0 },
  { "getteamplayersalive", &GScr_GetTeamPlayersAlive, 0 },
  { "objective_team", &GScr_Objective_Team, 0 },
  { "logprint", &GScr_LogPrint, 0 },
  { "worldentnumber", &GScr_WorldEntNumber, 0 },
  { "obituary",  (void(__cdecl *)()) & GScr_Obituary, 0 },
  { "positionwouldtelefrag", &GScr_positionWouldTelefrag, 0 },
  { "getstarttime", &GScr_getStartTime, 0 },
  { "precachemenu", &GScr_PrecacheMenu, 0 },
  { "precachestatusicon", &GScr_PrecacheStatusIcon, 0 },
  { "precacheheadicon", &GScr_PrecacheHeadIcon, 0 },
  { "precachelocationselector", &GScr_PrecacheLocationSelector, 0 },
  { "map_restart", &GScr_MapRestart, 0 },
  { "exitlevel", &GScr_ExitLevel, 0 },
  { "addtestclient", &GScr_AddTestClient, 0 },
  { "makedvarserverinfo", &GScr_MakeDvarServerInfo, 0 },
  { "setarchive", &KISAK_NULLSUB, 0 },
  { "allclientsprint", &GScr_AllClientsPrint, 0 },
  { "clientprint", &GScr_ClientPrint, 0 },
  { "mapexists", &GScr_MapExists, 0 },
  { "isvalidgametype", &GScr_IsValidGameType, 0 },
  { "matchend", &KISAK_NULLSUB, 0 },
  { "setplayerteamrank", &KISAK_NULLSUB, 0 },
  { "sendranks", &KISAK_NULLSUB, 0 },
  { "endparty", &KISAK_NULLSUB, 0 },
  { "setteamradar",  (void(__cdecl *)()) & GScr_SetTeamRadar, 0 },
  { "getteamradar", &GScr_GetTeamRadar, 0 },
  { "getassignedteam", (void(__cdecl *)()) & PlayerCmd_buttonPressedDEVONLY, 0 },
  { "setvotestring", &GScr_SetVoteString, 0 },
  { "setvotetime", &GScr_SetVoteTime, 0 },
  { "setvoteyescount", &GScr_SetVoteYesCount, 0 },
  { "setvotenocount", &GScr_SetVoteNoCount, 0 },
  { "openfile", &GScr_OpenFile, 1 },
  { "closefile", &GScr_CloseFile, 1 },
  { "fprintln", &GScr_FPrintln, 1 },
  { "fprintfields", &GScr_FPrintFields, 1 },
  { "freadln", &GScr_FReadLn, 1 },
  { "fgetarg", &GScr_FGetArg, 1 },
  { "kick", &GScr_KickPlayer, 0 },
  { "ban", &GScr_BanPlayer, 0 },
  { "map", &GScr_LoadMap, 0 },
  { "playrumbleonposition", &KISAK_NULLSUB, 0 },
  { "playrumblelooponposition", &KISAK_NULLSUB, 0 },
  { "stopallrumbles", &KISAK_NULLSUB, 0 },
  { "soundexists", &ScrCmd_SoundExists, 0 },
  { "issplitscreen", (void(__cdecl *)()) & PlayerCmd_buttonPressedDEVONLY, 0 },
  { "setminimap", &GScr_SetMiniMap, 0 },
  { "setmapcenter", &GScr_SetMapCenter, 0 },
  { "setgameendtime", &GScr_SetGameEndTime, 0 },
  { "getarraykeys", &GScr_GetArrayKeys, 0 },
  { "searchforonlinegames", &KISAK_NULLSUB, 0 },
  { "quitlobby", &KISAK_NULLSUB, 0 },
  { "quitparty", &KISAK_NULLSUB, 0 },
  { "startparty", &KISAK_NULLSUB, 0 },
  { "startprivatematch", &KISAK_NULLSUB, 0 },
  { "visionsetnaked", &Scr_VisionSetNaked, 0 },
  { "visionsetnight", &Scr_VisionSetNight, 0 },
  { "tablelookup", &Scr_TableLookup, 0 },
  { "tablelookupistring", &Scr_TableLookupIString, 0 },
  { "endlobby", &KISAK_NULLSUB, 0 }
}; // idb

void __cdecl ScrCmd_NULLSUB(scr_entref_t entref)
{

}

BuiltinMethodDef methods_2[] =
{
  { "attach", &ScrCmd_attach, 0 },
  { "detach", &ScrCmd_detach, 0 },
  { "detachall", &ScrCmd_detachAll, 0 },
  { "getattachsize", &ScrCmd_GetAttachSize, 0 },
  { "getattachmodelname", &ScrCmd_GetAttachModelName, 0 },
  { "getattachtagname", &ScrCmd_GetAttachTagName, 0 },
  { "getattachignorecollision", &ScrCmd_GetAttachIgnoreCollision, 0 },
  { "getammocount", &GScr_GetAmmoCount, 0 },
  { "getclanid", &ScrCmd_GetClanId, 0 },
  { "getclanname", &ScrCmd_GetClanName, 0 },
  { "hidepart", &ScrCmd_hidepart, 0 },
  { "showpart", &ScrCmd_showpart, 0 },
  { "showallparts", &ScrCmd_showallparts, 0 },
  { "linkto", &ScrCmd_LinkTo, 0 },
  { "unlink", &ScrCmd_Unlink, 0 },
  { "enablelinkto", &ScrCmd_EnableLinkTo, 0 },
  { "getorigin", &ScrCmd_GetOrigin, 0 },
  { "geteye", &ScrCmd_GetEye, 0 },
  { "useby", &ScrCmd_UseBy, 0 },
  { "setstablemissile", &Scr_SetStableMissile, 0 },
  { "istouching", &ScrCmd_IsTouching, 0 },
  { "playsound", &ScrCmd_PlaySound, 0 },
  { "playsoundasmaster", &ScrCmd_PlaySoundAsMaster, 0 },
  { "playsoundtoteam", &ScrCmd_PlaySoundToTeam, 0 },
  { "playsoundtoplayer", &ScrCmd_PlaySoundToPlayer, 0 },
  { "playloopsound", &ScrCmd_PlayLoopSound, 0 },
  { "stoploopsound", &ScrCmd_StopLoopSound, 0 },
  { "playrumbleonentity", ScrCmd_NULLSUB, 0},
  { "playrumblelooponentity", ScrCmd_NULLSUB, 0 },
  { "stoprumble", ScrCmd_NULLSUB, 0 },
  { "delete", &ScrCmd_Delete, 0 },
  { "setmodel", &ScrCmd_SetModel, 0 },
  { "getnormalhealth", &ScrCmd_GetNormalHealth, 0 },
  { "setnormalhealth", &ScrCmd_SetNormalHealth, 0 },
  { "show", &ScrCmd_Show, 0 },
  { "hide", &ScrCmd_Hide, 0 },
  { "laseron", &ScrCmd_LaserOn, 0 },
  { "laseroff", &ScrCmd_LaserOff, 0 },
  { "showtoplayer", &ScrCmd_ShowToPlayer, 0 },
  { "setcontents", &ScrCmd_SetContents, 0 },
  { "getstance", &ScrCmd_GetStance, 0 },
  { "setcursorhint", &GScr_SetCursorHint, 0 },
  { "sethintstring", &GScr_SetHintString, 0 },
  { "usetriggerrequirelookat", &GScr_UseTriggerRequireLookAt, 0 },
  { "shellshock", &GScr_ShellShock, 0 },
  { "gettagorigin", &GScr_GetTagOrigin, 0 },
  { "gettagangles", &GScr_GetTagAngles, 0 },
  { "stopshellshock", &GScr_StopShellShock, 0 },
  { "setdepthoffield", &GScr_SetDepthOfField, 0 },
  { "setviewmodeldepthoffield", &GScr_SetViewModelDepthOfField, 0 },
  { "viewkick", &GScr_ViewKick, 0 },
  { "localtoworldcoords", &GScr_LocalToWorldCoords, 0 },
  { "setrightarc", &GScr_SetRightArc, 0 },
  { "setleftarc", &GScr_SetLeftArc, 0 },
  { "settoparc", &GScr_SetTopArc, 0 },
  { "setbottomarc", &GScr_SetBottomArc, 0 },
  { "radiusdamage", &GScr_EntityRadiusDamage, 0 },
  { "detonate", &GScr_Detonate, 0 },
  { "damageconetrace", &GScr_DamageConeTrace, 0 },
  { "sightconetrace", &GScr_SightConeTrace, 0 },
  { "getentitynumber", &GScr_GetEntityNumber, 0 },
  { "enablegrenadetouchdamage", &GScr_EnableGrenadeTouchDamage, 0 },
  { "disablegrenadetouchdamage", &GScr_DisableGrenadeTouchDamage, 0 },
  { "enableaimassist", &GScr_EnableAimAssist, 0 },
  { "disableaimassist", &GScr_DisableAimAssist, 0 },
  { "placespawnpoint", &GScr_PlaceSpawnPoint, 0 },
  { "setteamfortrigger", &GScr_SetTeamForTrigger, 0 },
  { "clientclaimtrigger", &GScr_ClientClaimTrigger, 0 },
  { "clientreleasetrigger", &GScr_ClientReleaseTrigger, 0 },
  { "releaseclaimedtrigger", &GScr_ReleaseClaimedTrigger, 0 },
  { "getstat", &GScr_GetStat, 0 },
  { "setstat", &GScr_SetStat, 0 },
  { "sendleaderboards",ScrCmd_NULLSUB, 0 },
  { "setmovespeedscale", &ScrCmd_SetMoveSpeedScale, 0 },
  { "logstring", ScrCmd_NULLSUB, 0 },
  { "missile_settarget", &GScr_MissileSetTarget, 0 },
  { "isonladder", &GScr_IsOnLadder, 0 },
  { "ismantling", &GScr_IsMantling, 0 },
  { "startragdoll", &GScr_StartRagdoll, 0 },
  { "isragdoll", &GScr_IsRagdoll, 0 },
  { "getcorpseanim", &GScr_GetCorpseAnim, 0 },
  { "itemweaponsetammo", &ScrCmd_ItemWeaponSetAmmo, 0 }
}; // idb

unsigned int __cdecl GScr_AllocString(const char *s)
{
    return Scr_AllocString((char*)s, 1);
}

void __cdecl TRACK_g_scr_main()
{
    track_static_alloc_internal(&scr_const, 372, "scr_const", 7);
    track_static_alloc_internal(&g_scr_data, 14236, "g_scr_data", 7);
}

void __cdecl Scr_LoadLevel()
{
    uint16_t t; // [esp+0h] [ebp-4h]

    if (g_scr_data.levelscript)
    {
        t = Scr_ExecThread(g_scr_data.levelscript, 0);
        Scr_FreeThread(t);
    }
}

void __cdecl GScr_LoadGameTypeScript()
{
    char filename[68]; // [esp+0h] [ebp-48h] BYREF

    Com_sprintf(filename, 0x40u, "maps/mp/gametypes/%s", g_gametype->current.string);
    g_scr_data.gametype.main = GScr_LoadScriptAndLabel(filename, "main", 1);
    g_scr_data.gametype.startupgametype = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_StartGameType",
        1);
    g_scr_data.gametype.playerconnect = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_PlayerConnect",
        1);
    g_scr_data.gametype.playerdisconnect = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_PlayerDisconnect",
        1);
    g_scr_data.gametype.playerdamage = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_PlayerDamage",
        1);
    g_scr_data.gametype.playerkilled = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_PlayerKilled",
        1);
    g_scr_data.gametype.playerlaststand = GScr_LoadScriptAndLabel(
        "maps/mp/gametypes/_callbacksetup",
        "CodeCallback_PlayerLastStand",
        1);
}

int32_t __cdecl GScr_LoadScriptAndLabel(const char *filename, const char *label, int32_t bEnforceExists)
{
    int32_t func; // [esp+0h] [ebp-4h]

    if (G_ExitAfterConnectPaths())
        return 0;
    if (!Scr_LoadScript(filename) && bEnforceExists)
        Com_Error(ERR_DROP, "Could not find script %s", filename);
    func = Scr_GetFunctionHandle(filename, label);
    if (!func)
    {
        if (bEnforceExists)
            Com_Error(ERR_DROP, "Could not find label %s in script %s", label, filename);
    }
    return func;
}

void __cdecl GScr_LoadScripts()
{
    Scr_BeginLoadScripts();
    g_scr_data.delete_ = GScr_LoadScriptAndLabel("codescripts/delete", "main", 1);
    g_scr_data.initstructs = GScr_LoadScriptAndLabel("codescripts/struct", "initstructs", 1);
    g_scr_data.createstruct = GScr_LoadScriptAndLabel("codescripts/struct", "createstruct", 1);
    GScr_LoadGameTypeScript();
    GScr_LoadLevelScript();
    Scr_PostCompileScripts();
    GScr_PostLoadScripts();
    Scr_EndLoadScripts();
}

int32_t GScr_LoadLevelScript()
{
    int32_t result; // eax
    char filename[64]; // [esp+0h] [ebp-48h] BYREF
    const dvar_s *mapname; // [esp+44h] [ebp-4h]

    mapname = Dvar_RegisterString("mapname", (char *)"", DVAR_SERVERINFO | DVAR_ROM, "The current map name");
    Com_sprintf(filename, 0x40u, "maps/mp/%s", mapname->current.string);
    result = GScr_LoadScriptAndLabel(filename, "main", 0);
    g_scr_data.levelscript = result;
    return result;
}

void GScr_PostLoadScripts()
{
    int32_t classnum; // [esp+0h] [ebp-4h]

    for (classnum = 0; classnum < 4; ++classnum)
        Scr_SetClassMap(classnum);
    GScr_AddFieldsForEntity();
    GScr_AddFieldsForHudElems();
    GScr_AddFieldsForRadiant();
}

void __cdecl GScr_FreeScripts()
{
    int32_t classnum; // [esp+0h] [ebp-4h]

    for (classnum = 0; classnum < 4; ++classnum)
        Scr_RemoveClassMap(classnum);
}

void __cdecl ScrCmd_GetClanId(scr_entref_t entref)
{
    Scr_AddString("0");
}

void __cdecl ScrCmd_GetClanName(scr_entref_t entref)
{
    Scr_AddString((char *)"");
}

void GScr_CreatePrintChannel()
{
    const char *name; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam() != 1)
        Scr_Error("illegal call to createprintchannel()");

    name = Scr_GetString(0);

    if (!Con_OpenChannel((char*)name, 1))
        Scr_Error("Unable to create new channel.  Maximum number of channels exeeded.");
}

void GScr_printChannelSet()
{
    int32_t Type; // [esp+0h] [ebp-10h]
    int32_t oldChannel; // [esp+4h] [ebp-Ch]
    int32_t channel; // [esp+8h] [ebp-8h] BYREF
    const char *name; // [esp+Ch] [ebp-4h]

    channel = 24;
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("illegal call to setprintchannel()");
        return;
    }
    oldChannel = level.scriptPrintChannel;
    Type = Scr_GetType(0);
    if (Type == 2)
    {
        name = Scr_GetString(0);
        if (!Con_GetChannel(name, &channel))
        {
            Scr_ParamError(0, "Invalid Print Channel");
            return;
        }
    }
    else if (Type != 6 || (channel = Scr_GetInt(0), !Con_IsChannelOpen(channel)))
    {
        Scr_ParamError(0, "Invalid Print Channel");
        return;
    }
    if (Con_ScriptHasPermission(channel))
    {
        level.scriptPrintChannel = channel;
        Scr_AddInt(oldChannel);
    }
    else
    {
        Scr_ParamError(0, "Script does not have permission to print to this channel");
    }
}

void print()
{
    int32_t NumParam; // r28
    int32_t i; // r31
    const char *DebugString; // r3

    if (!g_NoScriptSpam->current.enabled)
    {
        NumParam = Scr_GetNumParam();
        for (i = 0; i < NumParam; ++i)
        {
            DebugString = Scr_GetDebugString(i);
            Com_Printf(level.scriptPrintChannel, "%s", DebugString);
        }
    }
}

void println()
{
    if (!g_NoScriptSpam->current.enabled)
    {
        print();
        Com_Printf(level.scriptPrintChannel, "\n");
    }
}

void __cdecl Scr_LocalizationError(uint32_t  iParm, const char *pszErrorMessage)
{
    Scr_ParamError(iParm, pszErrorMessage);
}

void __cdecl Scr_ConstructMessageString(
    int32_t firstParmIndex,
    int32_t lastParmIndex,
    const char *errorContext,
    char *string,
    uint32_t  stringLimit)
{
    uint32_t  v10; // [esp+0h] [ebp-54h]
    uint32_t  v11; // [esp+10h] [ebp-44h]
    uint32_t  charIndex; // [esp+34h] [ebp-20h]
    uint32_t  charIndexa; // [esp+34h] [ebp-20h]
    uint32_t  tokenLen; // [esp+38h] [ebp-1Ch]
    int32_t type; // [esp+40h] [ebp-14h]
    gentity_s *ent; // [esp+44h] [ebp-10h]
    const char *token; // [esp+4Ch] [ebp-8h]
    uint32_t  stringLen; // [esp+50h] [ebp-4h]

    stringLen = 0;
    while (firstParmIndex <= lastParmIndex)
    {
        type = Scr_GetType(firstParmIndex);
        if (type == 3)
        {
            token = Scr_GetIString(firstParmIndex);
            tokenLen = strlen(token);
            Scr_ValidateLocalizedStringRef(firstParmIndex, token, tokenLen);
            if (stringLen + tokenLen + 1 >= stringLimit)
            {
                Scr_ParamError(firstParmIndex, va("%s is too long. Max length is %i\n", errorContext, stringLimit));
            }
            if (stringLen)
                string[stringLen++] = 20;
        }
        else if (type == 1 && Scr_GetPointerType(firstParmIndex) == 20)
        {
            ent = Scr_GetEntity(firstParmIndex);
            if (!ent->client)
                Scr_ParamError(firstParmIndex, "Entity is not a player");
            token = va("%s^7", ent->client->sess.cs.name);
            v11 = strlen(token);
            tokenLen = v11;
            if (stringLen + v11 + 1 >= stringLimit)
            {
                Scr_ParamError(firstParmIndex, va("%s is too long. Max length is %i\n", errorContext, stringLimit));
            }
            if (v11)
                string[stringLen++] = 21;
        }
        else
        {
            token = Scr_GetString(firstParmIndex);
            v10 = strlen(token);
            tokenLen = v10;
            for (charIndex = 0; charIndex < v10; ++charIndex)
            {
                if (token[charIndex] == 20 || token[charIndex] == 21 || token[charIndex] == 22)
                {
                    Scr_ParamError(firstParmIndex, va("bad escape character (%i) present in string", token[charIndex]));
                }
                if (isalpha(token[charIndex]))
                {
                    if (loc_warnings->current.enabled)
                    {
                        if (loc_warningsAsErrors->current.enabled)
                        {
                            Scr_LocalizationError(firstParmIndex, va("non-localized %s strings are not allowed to have letters in them: \"%s\"", errorContext, token));
                        }
                        else
                        {
                            Com_PrintWarning(
                                17,
                                "WARNING: Non-localized %s string is not allowed to have letters in it. Must be changed over to a localiz"
                                "ed string: \"%s\"\n",
                                errorContext,
                                token);
                        }
                    }
                    break;
                }
            }
            if (stringLen + v10 + 1 >= stringLimit)
            {
                Scr_ParamError(firstParmIndex, va("%s is too long. Max length is %i\n", errorContext, stringLimit));
            }
            if (v10)
                string[stringLen++] = 21;
        }
        for (charIndexa = 0; charIndexa < tokenLen; ++charIndexa)
        {
            if (token[charIndexa] == 20 || token[charIndexa] == 21 || token[charIndexa] == 22)
                string[stringLen] = 46;
            else
                string[stringLen] = token[charIndexa];
            ++stringLen;
        }
        ++firstParmIndex;
    }
    string[stringLen] = 0;
}

void __cdecl Scr_ValidateLocalizedStringRef(uint32_t  parmIndex, const char *token, int32_t tokenLen)
{
    int32_t charIter; // [esp+0h] [ebp-4h]

    iassert(token);
    iassert(tokenLen >= 0);

    if (tokenLen > 1)
    {
        for (charIter = 0; charIter < tokenLen; ++charIter)
        {
            if (!isalnum(token[charIter]) && token[charIter] != 95)
            {
                Scr_ParamError(parmIndex, va("Illegal localized string reference: %s must contain only alpha-numeric characters and underscores", token));
            }
        }
    }
}

void __cdecl Scr_MakeGameMessage(int32_t iClientNum, const char *pszCmd)
{
    uint32_t  NumParam; // eax
    const char *v3; // eax
    char string[1028]; // [esp+0h] [ebp-408h] BYREF

    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Game Message", string, 0x400u);
    v3 = va("%s \"%s\"", pszCmd, string);
    SV_GameSendServerCommand(iClientNum, SV_CMD_CAN_IGNORE, v3);
}

void __cdecl Scr_VerifyWeaponIndex(int32_t weaponIndex, const char *weaponName)
{
    const char *v2; // eax

    if (!weaponName)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 517, 0, "%s", "weaponName");
    if (!weaponIndex)
    {
        if (I_stricmp("none", weaponName))
            v2 = va(
                "Unknown weapon name \"%s\": script may need to call PreCacheItem(\"%s\") during level init.\n",
                weaponName,
                weaponName);
        else
            v2 = va("Weapon name \"%s\" is not valid.\n", weaponName);
        Scr_ParamError(0, v2);
    }
}

void iprintln()
{
    const char *v0; // eax

    v0 = va("%c", 102);
    Scr_MakeGameMessage(-1, v0);
}

void iprintlnbold()
{
    const char *v0; // eax

    v0 = va("%c", 103);
    Scr_MakeGameMessage(-1, v0);
}

void GScr_print3d()
{
    VariableUnion duration; // [esp+10h] [ebp-34h]
    float origin[3]; // [esp+14h] [ebp-30h] BYREF
    float rgb[3]; // [esp+20h] [ebp-24h] BYREF
    float scale; // [esp+2Ch] [ebp-18h]
    float color[4]; // [esp+30h] [ebp-14h] BYREF
    const char *text; // [esp+40h] [ebp-4h]

    duration.intValue = 1;
    scale = 1.0;
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = 1.0;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto $LN11_16;
    case 3u:
        goto $LN3_26;
    case 4u:
        goto $LN4_36;
    case 5u:
        goto $LN5_37;
    case 6u:
        duration.intValue = Scr_GetInt(5);
    $LN5_37:
        scale = Scr_GetFloat(4);
    $LN4_36:
        color[3] = Scr_GetFloat(3);
    $LN3_26:
        Scr_GetVector(2u, rgb);
        color[0] = rgb[0];
        color[1] = rgb[1];
        color[2] = rgb[2];
    $LN11_16:
        text = Scr_GetString(1);
        Scr_GetVector(0, origin);
        G_AddDebugString(origin, color, scale, text, duration.intValue);
        break;
    default:
        Scr_Error("illegal call to print3d()");
        break;
    }
}

void GScr_line()
{
    VariableUnion duration; // [esp+4h] [ebp-3Ch]
    float rgb[3]; // [esp+8h] [ebp-38h] BYREF
    float start[3]; // [esp+14h] [ebp-2Ch] BYREF
    float end[3]; // [esp+20h] [ebp-20h] BYREF
    float color[4]; // [esp+2Ch] [ebp-14h] BYREF
    int32_t depthTest; // [esp+3Ch] [ebp-4h]

    duration.intValue = 0;
    depthTest = 0;
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = 1.0;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto $LN11_17;
    case 3u:
        goto $LN3_27;
    case 4u:
        goto $LN4_37;
    case 5u:
        goto $LN5_38;
    case 6u:
        duration.intValue = Scr_GetInt(5);
    $LN5_38:
        depthTest = Scr_GetInt(4);
    $LN4_37:
        color[3] = Scr_GetFloat(3);
    $LN3_27:
        Scr_GetVector(2u, rgb);
        color[0] = rgb[0];
        color[1] = rgb[1];
        color[2] = rgb[2];
    $LN11_17:
        Scr_GetVector(1u, end);
        Scr_GetVector(0, start);
        break;
    default:
        Scr_Error("illegal call to line()");
        break;
    }
    CL_AddDebugLine(start, end, color, depthTest, duration.intValue, 1);
}

void assertCmd()
{
    if (!Scr_GetInt(0))
        Scr_Error("assert fail");
}

void assertexCmd()
{
    const char *String; // eax
    const char *v1; // eax

    if (!Scr_GetInt(0))
    {
        String = Scr_GetString(1);
        v1 = va("assert fail: %s", String);
        Scr_Error(v1);
    }
}

void assertmsgCmd()
{
    const char *String; // eax
    const char *v1; // eax

    String = Scr_GetString(0);
    v1 = va("assert fail: %s", String);
    Scr_Error(v1);
}

void GScr_IsDefined()
{
    int32_t type; // [esp+4h] [ebp-4h]
    int32_t typea; // [esp+4h] [ebp-4h]

    type = Scr_GetType(0);
    if (type == 1)
    {
        typea = Scr_GetPointerType(0);
        if (typea < 14)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 656, 0, "%s", "type >= FIRST_OBJECT");
        if (typea >= 22 || typea == 19)
            Scr_AddInt(0);
        else
            Scr_AddInt(1);
    }
    else
    {
        if (type >= 14)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 661, 0, "%s", "type < FIRST_OBJECT");
        Scr_AddInt(type != 0);
    }
}

void GScr_IsString()
{
    int32_t Type; // eax

    Type = Scr_GetType(0);
    Scr_AddInt(Type == 2);
}

void GScr_IsAlive()
{
    if (Scr_GetType(0) == 1 && Scr_GetPointerType(0) == 20 && Scr_GetEntity(0)->health > 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void GScr_GetDvar()
{
    const char *dvarName; // [esp+0h] [ebp-8h]
    const char *dvarValue; // [esp+4h] [ebp-4h]

    dvarName = Scr_GetString(0);
    dvarValue = Dvar_GetVariantString(dvarName);
    Scr_AddString(dvarValue);
}

void GScr_GetDvarInt()
{
    const char *dvarName; // [esp+0h] [ebp-8h]
    const char *dvarValue; // [esp+4h] [ebp-4h]

    dvarName = Scr_GetString(0);
    dvarValue = Dvar_GetVariantString(dvarName);

    Scr_AddInt(atoi(dvarValue));
}

void GScr_GetDvarFloat()
{
    const char *dvarName; // [esp+8h] [ebp-8h]
    const char *dvarValue; // [esp+Ch] [ebp-4h]

    dvarName = Scr_GetString(0);
    dvarValue = Dvar_GetVariantString(dvarName);

    Scr_AddFloat(atof(dvarValue));
}

void GScr_SetDvar()
{
    uint32_t  NumParam; // eax
    char v1; // al
    const char *v2; // eax
    bool v3; // [esp+0h] [ebp-840h]
    char string[1028]; // [esp+18h] [ebp-828h] BYREF
    int32_t len; // [esp+41Ch] [ebp-424h]
    char outString[1024]; // [esp+420h] [ebp-420h] BYREF
    const char *dvarName; // [esp+828h] [ebp-18h]
    int32_t type; // [esp+82Ch] [ebp-14h]
    int32_t i; // [esp+830h] [ebp-10h]
    const dvar_s *dvar; // [esp+834h] [ebp-Ch]
    const char *dvarValue; // [esp+838h] [ebp-8h]
    char *pCh; // [esp+83Ch] [ebp-4h]

    dvarName = Scr_GetString(0);
    type = Scr_GetType(1);
    if (type == 3)
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(1, NumParam - 1, "Dvar Value", string, 0x400u);
        dvarValue = string;
    }
    else
    {
        dvarValue = Scr_GetString(1);
    }
    len = strlen(dvarValue);
    pCh = outString;
    i = 0;
    while (i < 1023 && dvarValue[i])
    {
        v1 = I_CleanChar(dvarValue[i]);
        *pCh = v1;
        if (*pCh == 34)
            *pCh = 39;
        ++i;
        ++pCh;
    }
    *pCh = 0;
    if (Dvar_IsValidName(dvarName))
    {
        v3 = Scr_GetNumParam() >= 3 && Scr_GetInt(2);
        Dvar_SetFromStringByName(dvarName, (char *)dvarValue);
        if (v3)
        {
            dvar = Dvar_FindVar(dvarName);
            if (!dvar)
                MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 778, 0, "%s", "dvar");
            Dvar_AddFlags(dvar, 1024);
        }
    }
    else
    {
        v2 = va("Dvar %s has an invalid dvar name", dvarName);
        Scr_Error(v2);
    }
}

void GScr_GetTime()
{
    Scr_AddInt(level.time);
}

void Scr_GetEntByNum()
{
    int32_t entnum; // [esp+4h] [ebp-4h]

    entnum = Scr_GetInt(0);
    if ((uint32_t )entnum < 0x400)
    {
        if (g_entities[entnum].r.inuse)
            Scr_AddEntity(&g_entities[entnum]);
    }
}

void Scr_GetWeaponModel()
{
    WeaponDef *WeaponDef; // eax
    uint32_t  weaponModel; // [esp+0h] [ebp-Ch]
    int32_t iWeaponIndex; // [esp+4h] [ebp-8h]
    const char *pszWeaponName; // [esp+8h] [ebp-4h]

    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    weaponModel = 0;
    if (iWeaponIndex)
    {
        if (Scr_GetNumParam() == 2)
        {
            weaponModel = Scr_GetInt(1);
            if (weaponModel < 0x100)
            {
                if (!BG_GetWeaponDef(iWeaponIndex)->worldModel[weaponModel])
                    weaponModel = 0;
            }
            else
            {
                weaponModel = 0;
            }
        }
        WeaponDef = BG_GetWeaponDef(iWeaponIndex);
        Scr_AddString(XModelGetName(WeaponDef->worldModel[weaponModel]));
    }
    else
    {
        if (*pszWeaponName)
        {
            if (I_stricmp(pszWeaponName, "none"))
            {
                Com_Printf(17, va("unknown weapon '%s' in getWeaponModel\n", pszWeaponName));
            }
        }
        Scr_AddString((char *)"");
    }
}

void __cdecl GScr_GetAmmoCount(scr_entref_t entref)
{
    int32_t v1; // eax
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GetPlayerEntity(entref);

    iassert(ent);
    iassert(ent->client);

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    if (weaponIndex)
    {
        v1 = BG_WeaponAmmo(&ent->client->ps, weaponIndex);
        Scr_AddInt(v1);
    }
    else
    {
        Scr_AddInt(0);
    }
}

gentity_s *__cdecl GetPlayerEntity(scr_entref_t entref)
{
    const char *v2; // eax
    const char *v4; // [esp+20h] [ebp-8h]
    gentity_s *ent; // [esp+24h] [ebp-4h]

    ent = GetEntity(entref);
    iassert(ent);

    if (!ent->client)
    {
        if (ent->targetname)
            v4 = SL_ConvertToString(ent->targetname);
        else
            v4 = "<undefined>";
        v2 = va(
            "only valid on players; called on entity %i at %.0f %.0f %.0f classname %s targetname %s\n",
            entref.entnum,
            ent->r.currentOrigin[0],
            ent->r.currentOrigin[1],
            ent->r.currentOrigin[2],
            SL_ConvertToString(ent->classname),
            v4);
        Scr_Error(v2);
    }
    return ent;
}

gentity_s *__cdecl GetEntity(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        return 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 198, 0, "%s", "entref.entnum < MAX_GENTITIES");
        return &g_entities[entref.entnum];
    }
}

void GScr_AnimHasNotetrack()
{
    const XAnim_s *Anims; // eax
    uint8_t v1; // al
    uint32_t  floatValue; // [esp-4h] [ebp-10h]
    const char *anim; // [esp+8h] [ebp-4h]

    anim = Scr_GetAnim(0, 0).linkPointer;
    floatValue = Scr_GetConstString(1);
    Anims = Scr_GetAnims(HIWORD(anim));
    v1 = XAnimNotetrackExists(Anims, (uint16_t)anim, floatValue);
    Scr_AddBool(v1);
}

void GScr_GetNotetrackTimes()
{
    const XAnim_s *Anims; // eax
    VariableUnion name; // [esp+4h] [ebp-8h]
    const char *anim; // [esp+8h] [ebp-4h]

    anim = Scr_GetAnim(0, 0).linkPointer;
    name.intValue = Scr_GetConstString(1);
    Scr_MakeArray();
    Anims = Scr_GetAnims(HIWORD(anim));
    XAnimAddNotetrackTimesToScriptArray(Anims, (uint16_t)anim, name.stringValue);
}

void GScr_GetBrushModelCenter()
{
    gentity_s *pEnt; // [esp+8h] [ebp-10h]
    float vCenter[3]; // [esp+Ch] [ebp-Ch] BYREF

    pEnt = Scr_GetEntity(0);
    Vec3Add(pEnt->r.absmin, pEnt->r.absmax, vCenter);
    Vec3Scale(vCenter, 0.5, vCenter);
    Scr_AddVector(vCenter);
}

void GScr_Spawn()
{
    float *currentOrigin; // [esp+0h] [ebp-1Ch]
    float origin[3]; // [esp+4h] [ebp-18h] BYREF
    int32_t iSpawnFlags; // [esp+10h] [ebp-Ch]
    unsigned short classname; // [esp+14h] [ebp-8h]
    gentity_s *ent; // [esp+18h] [ebp-4h]

    classname = Scr_GetConstString(0);
    Scr_GetVector(1u, origin);
    if (Scr_GetNumParam() <= 2)
        iSpawnFlags = 0;
    else
        iSpawnFlags = Scr_GetInt(2);
    ent = G_Spawn();
    Scr_SetString(&ent->classname, classname);
    currentOrigin = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = origin[0];
    currentOrigin[1] = origin[1];
    currentOrigin[2] = origin[2];
    ent->spawnflags = iSpawnFlags;
    if (G_CallSpawnEntity(ent))
    {
        Scr_AddEntity(ent);
    }
    else
    {
        Scr_Error(va("unable to spawn \"%s\" entity", SL_ConvertToString(classname)));
    }
}

void GScr_SpawnPlane()
{
    float *currentOrigin; // [esp+0h] [ebp-28h]
    float origin[3]; // [esp+4h] [ebp-24h] BYREF
    int32_t iSpawnFlags; // [esp+10h] [ebp-18h]
    int32_t team; // [esp+14h] [ebp-14h]
    gentity_s *owner; // [esp+18h] [ebp-10h]
    uint16_t classname; // [esp+1Ch] [ebp-Ch]
    gentity_s *ent; // [esp+20h] [ebp-8h]
    int32_t ownerIndex; // [esp+24h] [ebp-4h]

    owner = Scr_GetEntity(0);
    if (!owner->client)
        Scr_ParamError(0, "Owner entity is not a player");
    classname = Scr_GetConstString(1);
    Scr_GetVector(2u, origin);
    if (Scr_GetNumParam() <= 3)
        iSpawnFlags = 0;
    else
        iSpawnFlags = Scr_GetInt(3);
    ent = G_Spawn();
    Scr_SetString(&ent->classname, classname);
    currentOrigin = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = origin[0];
    currentOrigin[1] = origin[1];
    currentOrigin[2] = origin[2];
    ent->spawnflags = iSpawnFlags;
    team = owner->client->sess.cs.team;
    if ((uint32_t )team >= 4)
        MyAssertHandler(
            ".\\game_mp\\g_scr_main_mp.cpp",
            964,
            0,
            "team doesn't index (1 << 2)\n\t%i not in [0, %i)",
            team,
            4);
    ownerIndex = owner->client - level.clients;
    if (G_CallSpawnEntity(ent))
    {
        Scr_AddEntity(ent);
        ent->s.eType = ET_PLANE;
        ent->s.lerp.u.vehicle.teamAndOwnerIndex = team | (4 * ownerIndex);
    }
    else
    {
        ent->s.eType = ET_PLANE;
        ent->s.lerp.u.vehicle.teamAndOwnerIndex = team | (4 * ownerIndex);
        Scr_Error(va("unable to spawn \"%s\" entity", SL_ConvertToString(classname)));
    }
}

void GScr_SpawnTurret()
{
    float *currentOrigin; // [esp+0h] [ebp-1Ch]
    float origin[3]; // [esp+4h] [ebp-18h] BYREF
    uint16_t classname; // [esp+10h] [ebp-Ch]
    gentity_s *ent; // [esp+14h] [ebp-8h]
    const char *weaponinfoname; // [esp+18h] [ebp-4h]

    classname = Scr_GetConstString(0);
    Scr_GetVector(1u, origin);
    weaponinfoname = Scr_GetString(2);
    ent = G_Spawn();
    Scr_SetString(&ent->classname, classname);
    currentOrigin = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = origin[0];
    currentOrigin[1] = origin[1];
    currentOrigin[2] = origin[2];
    G_SpawnTurret(ent, weaponinfoname);
    Scr_AddEntity(ent);
}

void GScr_SpawnHelicopter()
{
    float *currentAngles; // [esp+0h] [ebp-30h]
    float *currentOrigin; // [esp+4h] [ebp-2Ch]
    float origin[3]; // [esp+8h] [ebp-28h] BYREF
    gentity_s *owner; // [esp+14h] [ebp-1Ch]
    const char *vehicleInfoName; // [esp+18h] [ebp-18h]
    float angles[3]; // [esp+1Ch] [ebp-14h] BYREF
    gentity_s *ent; // [esp+28h] [ebp-8h]
    const char *modelname; // [esp+2Ch] [ebp-4h]

    owner = Scr_GetEntity(0);
    if (!owner->client)
        Scr_ParamError(0, "Owner entity is not a player");
    Scr_GetVector(1u, origin);
    Scr_GetVector(2u, angles);
    vehicleInfoName = Scr_GetString(3);
    modelname = Scr_GetString(4);
    ent = G_Spawn();
    Scr_SetString(&ent->classname, scr_const.script_vehicle);
    currentOrigin = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = origin[0];
    currentOrigin[1] = origin[1];
    currentOrigin[2] = origin[2];
    currentAngles = ent->r.currentAngles;
    ent->r.currentAngles[0] = angles[0];
    currentAngles[1] = angles[1];
    currentAngles[2] = angles[2];
    G_SpawnHelicopter(ent, owner, vehicleInfoName, (char *)modelname);
    Scr_AddEntity(ent);
}

uint32_t  GScr_PrecacheTurret()
{
    const char *turretInfo; // [esp+0h] [ebp-4h]

    if (!level.initializing)
        Scr_Error("precacheTurret must be called before any wait statements in the level script\n");

    turretInfo = Scr_GetString(0);
    return G_GetWeaponIndexForName(turretInfo);
}

void __cdecl ScrCmd_SetMoveSpeedScale(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetPlayerEntity(entref);
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1086, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1087, 0, "%s", "ent->client");
    ent->client->sess.moveSpeedScaleMultiplier = Scr_GetFloat(0);
}

void __cdecl ScrCmd_attach(scr_entref_t entref)
{
    uint32_t  v5; // [esp+0h] [ebp-18h]
    uint32_t  v6; // [esp+4h] [ebp-14h]
    const char *modelName; // [esp+Ch] [ebp-Ch]
    gentity_s *ent; // [esp+10h] [ebp-8h]

    ent = GetEntity(entref);
    modelName = Scr_GetString(0);

    if (Scr_GetNumParam() < 2)
        v6 = scr_const._;
    else
        v6 = Scr_GetConstLowercaseString(1);

    if (Scr_GetNumParam() < 3)
        v5 = 0;
    else
        v5 = Scr_GetInt(2);

    if (G_EntDetach(ent, modelName, v6))
    {
        Scr_Error(va("model '%s' already attached to tag '%s'", modelName, SL_ConvertToString(v6)));
    }
    if (!G_EntAttach(ent, (char*)modelName, v6, v5))
    {
        Scr_Error(va("failed to attach model '%s' to tag '%s'", modelName, SL_ConvertToString(v6)));
    }
}

void __cdecl ScrCmd_detach(scr_entref_t entref)
{
    uint32_t  v6; // [esp+0h] [ebp-14h]
    const char *modelName; // [esp+8h] [ebp-Ch]
    gentity_s *ent; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]

    ent = GetEntity(entref);
    modelName = Scr_GetString(0);
    if (Scr_GetNumParam() < 2)
        v6 = scr_const._;
    else
        v6 = Scr_GetConstLowercaseString(1);
    if (!G_EntDetach(ent, modelName, v6))
    {
        Com_Printf(23, "Current attachments:\n");
        for (i = 0; i < 19; ++i)
        {
            if (ent->attachModelNames[i])
            {
                if (ent->attachTagNames[i])
                {
                    Com_Printf(23, "model: '%s', tag: '%s'\n", SL_ConvertToString(G_ModelName(ent->attachModelNames[i])), SL_ConvertToString(ent->attachTagNames[i]));
                }
            }
        }
        Scr_Error(va("failed to detach model '%s' from tag '%s'", modelName, SL_ConvertToString(v6)));
    }
}

void __cdecl ScrCmd_detachAll(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    G_EntDetachAll(ent);
}

void __cdecl ScrCmd_GetAttachSize(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    for (i = 0; i < 19 && ent->attachModelNames[i]; ++i)
        ;
    Scr_AddInt(i);
}

void __cdecl ScrCmd_GetAttachModelName(scr_entref_t entref)
{
    uint32_t  v1; // eax
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    i = Scr_GetInt(0);
    if ((uint32_t )i >= 0x13 || !ent->attachModelNames[i])
        Scr_ParamError(0, "bad index");
    v1 = G_ModelName(ent->attachModelNames[i]);
    Scr_AddConstString(v1);
}

void __cdecl ScrCmd_GetAttachTagName(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    i = Scr_GetInt(0);
    if ((uint32_t )i >= 0x13 || !ent->attachModelNames[i])
        Scr_ParamError(0, "bad index");
    if (!ent->attachTagNames[i])
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1196, 0, "%s", "ent->attachTagNames[i]");
    Scr_AddConstString(ent->attachTagNames[i]);
}

void __cdecl ScrCmd_GetAttachIgnoreCollision(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    i = Scr_GetInt(0);
    if ((uint32_t )i >= 0x13 || !ent->attachModelNames[i])
        Scr_ParamError(0, "bad index");
    Scr_AddBool((ent->attachIgnoreCollision & (1 << i)) != 0);
}

void __cdecl G_EntityStateSetPartBits(gentity_s *ent, const uint32_t  *partBits)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1222, 0, "%s", "ent");
    ent->s.partBits[0] = *partBits;
    ent->s.partBits[1] = partBits[1];
    ent->s.partBits[2] = partBits[2];
    ent->s.partBits[3] = partBits[3];
}

void __cdecl G_EntityStateGetPartBits(const gentity_s *ent, uint32_t  *partBits)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1230, 0, "%s", "ent");
    *partBits = ent->s.partBits[0];
    partBits[1] = ent->s.partBits[1];
    partBits[2] = ent->s.partBits[2];
    partBits[3] = ent->s.partBits[3];
}

void __cdecl ScrCmd_hidepart(scr_entref_t entref)
{
    uint32_t  tagName; // [esp+0h] [ebp-24h]
    uint8_t boneIndex; // [esp+7h] [ebp-1Dh] BYREF
    DObj_s *obj; // [esp+8h] [ebp-1Ch]
    const char *modelName; // [esp+Ch] [ebp-18h]
    gentity_s *ent; // [esp+10h] [ebp-14h]
    uint32_t  partBits[4]; // [esp+14h] [ebp-10h] BYREF

    ent = GetEntity(entref);
    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        Scr_Error("entity has no model");
    boneIndex = -2;
    tagName = Scr_GetConstLowercaseString(0);
    if (Scr_GetNumParam() == 1)
    {
        if (!DObjGetBoneIndex(obj, tagName, &boneIndex))
        {
            Scr_Error(va("cannot find part '%s' in entity model", SL_ConvertToString(tagName)));
        }
    }
    else
    {
        modelName = Scr_GetString(1);
        if (!DObjGetModelBoneIndex(obj, modelName, tagName, &boneIndex))
        {
            Scr_Error(va("cannot find part '%s' in entity model '%s'", SL_ConvertToString(tagName), modelName));
        }
    }
    G_EntityStateGetPartBits(ent, partBits);
    partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
    DObjSetHidePartBits(obj, partBits);
    G_EntityStateSetPartBits(ent, partBits);
}

void __cdecl ScrCmd_showpart(scr_entref_t entref)
{
    uint32_t  tagName; // [esp+0h] [ebp-24h]
    uint8_t boneIndex; // [esp+7h] [ebp-1Dh] BYREF
    DObj_s *obj; // [esp+8h] [ebp-1Ch]
    const char *modelName; // [esp+Ch] [ebp-18h]
    gentity_s *ent; // [esp+10h] [ebp-14h]
    uint32_t  partBits[4]; // [esp+14h] [ebp-10h] BYREF

    ent = GetEntity(entref);
    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        Scr_Error("entity has no model");
    boneIndex = -2;
    tagName = Scr_GetConstLowercaseString(0);
    if (Scr_GetNumParam() == 1)
    {
        if (!DObjGetBoneIndex(obj, tagName, &boneIndex))
        {
            Scr_Error(va("cannot find part '%s' in entity model", SL_ConvertToString(tagName)));
        }
    }
    else
    {
        modelName = Scr_GetString(1);
        if (!DObjGetModelBoneIndex(obj, modelName, tagName, &boneIndex))
        {
            Scr_Error(va("cannot find part '%s' in entity model '%s'", SL_ConvertToString(tagName), modelName));
        }
    }
    G_EntityStateGetPartBits(ent, partBits);
    partBits[(int)boneIndex >> 5] &= ~(0x80000000 >> (boneIndex & 0x1F));
    DObjSetHidePartBits(obj, partBits);
    G_EntityStateSetPartBits(ent, partBits);
}

void __cdecl ScrCmd_showallparts(scr_entref_t entref)
{
    DObj_s *obj; // [esp+0h] [ebp-18h]
    gentity_s *ent; // [esp+4h] [ebp-14h]
    uint32_t  partBits[4]; // [esp+8h] [ebp-10h] BYREF

    ent = GetEntity(entref);
    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        Scr_Error("entity has no model");
    memset(partBits, 0, sizeof(partBits));
    DObjSetHidePartBits(obj, partBits);
    G_EntityStateSetPartBits(ent, partBits);
}

void __cdecl ScrCmd_LinkTo(scr_entref_t entref)
{
    uint32_t  tagName; // [esp+0h] [ebp-28h]
    float originOffset[3]; // [esp+4h] [ebp-24h] BYREF
    float anglesOffset[3]; // [esp+10h] [ebp-18h] BYREF
    int32_t numParam; // [esp+1Ch] [ebp-Ch]
    gentity_s *parent; // [esp+20h] [ebp-8h]
    gentity_s *ent; // [esp+24h] [ebp-4h]

    ent = GetEntity(entref);
    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        Scr_ParamError(0, "not an entity");
    if ((ent->flags & 0x1000) == 0)
    {
        Scr_ObjectError(va("entity (classname: '%s') does not currently support linkTo", SL_ConvertToString(ent->classname)));
    }
    parent = Scr_GetEntity(0);
    numParam = Scr_GetNumParam();
    tagName = 0;
    if (numParam >= 2)
    {
        tagName = Scr_GetConstLowercaseString(1);
        if (tagName == scr_const._)
            tagName = 0;
    }
    if (numParam > 2)
    {
        Scr_GetVector(2u, originOffset);
        Scr_GetVector(3u, anglesOffset);
        if (G_EntLinkToWithOffset(ent, parent, tagName, originOffset, anglesOffset))
            return;
    }
    else if (G_EntLinkTo(ent, parent, tagName))
    {
        return;
    }
    if (!SV_DObjExists(parent))
    {
        if (!parent->model)
            Scr_Error("failed to link entity since parent has no model");
        Scr_Error(va("failed to link entity since parent model '%s' is invalid", SL_ConvertToString(G_ModelName(parent->model))));
    }
    if (!parent->model)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1384, 0, "%s", "parent->model");
    if (tagName)
    {
        if (SV_DObjGetBoneIndex(parent, tagName) < 0)
        {
            SV_DObjDumpInfo(parent);
            Scr_Error(va("failed to link entity since tag '%s' does not exist in parent model '%s'", SL_ConvertToString(tagName), SL_ConvertToString(G_ModelName(parent->model))));
        }
    }
    Scr_Error("failed to link entity");
}

void __cdecl ScrCmd_Unlink(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    G_EntUnlink(ent);
}

void __cdecl ScrCmd_EnableLinkTo(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if ((ent->flags & 0x1000) != 0)
        Scr_ObjectError("entity already has linkTo enabled");
    if (ent->s.eType || ent->physicsObject)
    {
        Scr_ObjectError(va("entity (classname: '%s') does not currently support enableLinkTo", SL_ConvertToString(ent->classname)));
    }

    iassert(!ent->client);

    ent->flags |= FL_SUPPORTS_LINKTO;
}

void __cdecl ScrCmd_GetOrigin(scr_entref_t entref)
{
    float origin[3]; // [esp+4h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+10h] [ebp-4h]

    ent = GetEntity(entref);
    origin[0] = ent->r.currentOrigin[0];
    origin[1] = ent->r.currentOrigin[1];
    origin[2] = ent->r.currentOrigin[2];
    Scr_AddVector(origin);
}

void __cdecl ScrCmd_GetEye(scr_entref_t entref)
{
    gentity_s *ent; // [esp+4h] [ebp-10h]
    float eye[3]; // [esp+8h] [ebp-Ch] BYREF

    ent = GetEntity(entref);
    eye[0] = ent->r.currentOrigin[0];
    eye[1] = ent->r.currentOrigin[1];
    eye[2] = ent->r.currentOrigin[2] + 40.0;
    Scr_AddVector(eye);
}

void __cdecl ScrCmd_UseBy(scr_entref_t entref)
{
    gentity_s *pOther; // [esp+0h] [ebp-Ch]
    void(__cdecl * use)(gentity_s *, gentity_s *, gentity_s *); // [esp+4h] [ebp-8h]
    gentity_s *pEnt; // [esp+8h] [ebp-4h]

    pEnt = GetEntity(entref);
    pOther = Scr_GetEntity(0);
    Scr_AddEntity(pOther);
    Scr_Notify(pEnt, scr_const.trigger, 1u);
    use = entityHandlers[pEnt->handler].use;
    if (use)
        use(pEnt, pOther, pOther);
}

void __cdecl ScrCmd_IsTouching(scr_entref_t entref)
{
    const gentity_s *pOther; // [esp+1Ch] [ebp-28h]
    float vMins[3]; // [esp+20h] [ebp-24h] BYREF
    gentity_s *pEnt; // [esp+2Ch] [ebp-18h]
    int32_t bTouching; // [esp+30h] [ebp-14h]
    gentity_s *pTemp; // [esp+34h] [ebp-10h]
    float vMaxs[3]; // [esp+38h] [ebp-Ch] BYREF

    bTouching = 0;
    pEnt = GetEntity(entref);
    if (pEnt->r.bmodel || (pEnt->r.svFlags & 0x60) != 0)
    {
        pTemp = pEnt;
        pEnt = Scr_GetEntity(0);
        if (pEnt->r.bmodel || (pEnt->r.svFlags & 0x60) != 0)
            Scr_Error("istouching cannot be called on 2 brush/cylinder entities");
        pOther = pTemp;
    }
    else
    {
        pOther = Scr_GetEntity(0);
    }
    iassert(pEnt);
    iassert(pEnt->r.maxs[0] >= pEnt->r.mins[0]);
    iassert(pEnt->r.maxs[1] >= pEnt->r.mins[1]);
    iassert(pEnt->r.maxs[2] >= pEnt->r.mins[2]);

    Vec3Add(pEnt->r.currentOrigin, pEnt->r.mins, vMins);
    Vec3Add(pEnt->r.currentOrigin, pEnt->r.maxs, vMaxs);
    ExpandBoundsToWidth(vMins, vMaxs);
    bTouching = SV_EntityContact(vMins, vMaxs, pOther);
    Scr_AddInt(bTouching);
}

void ScrCmd_SoundExists()
{
    snd_alias_list_t *SoundAlias; // eax
    const char *soundName; // [esp+0h] [ebp-4h]

    soundName = Scr_GetString(0);
    SoundAlias = Com_TryFindSoundAlias(soundName);
    Scr_AddBool(SoundAlias != 0);
}

void __cdecl ScrCmd_PlaySound(scr_entref_t entref)
{
    uint32_t  NumParam; // eax
    const char *v2; // eax

    StartScriptPlaySoundOnEnt(entref);
    if (Scr_GetNumParam() != 1)
    {
        NumParam = Scr_GetNumParam();
        v2 = va("playsound has %d parameters.  There should be exactly one.", NumParam);
        Scr_Error(v2);
    }
}

gentity_s *__cdecl StartScriptPlaySoundOnEnt(scr_entref_t entref)
{
    gentity_s *result; // eax
    const char *pszSoundName; // [esp+0h] [ebp-10h]
    uint8_t iSoundIndex; // [esp+7h] [ebp-9h]
    gentity_s *pEnt; // [esp+8h] [ebp-8h]

    pEnt = GetEntity(entref);
    pszSoundName = Scr_GetString(0);
    iSoundIndex = G_SoundAliasIndex((char*)pszSoundName);
    result = G_TempEntity(pEnt->r.currentOrigin, 3);
    result->r.svFlags |= 8u;
    result->s.eventParm = iSoundIndex;
    return result;
}

void __cdecl ScrCmd_PlaySoundToTeam(scr_entref_t entref)
{
    gentity_s *tempEnt; // [esp+0h] [ebp-18h]
    int32_t teamNum; // [esp+4h] [ebp-14h]
    uint16_t team; // [esp+8h] [ebp-10h]
    gentity_s *ignoreClientEnt; // [esp+Ch] [ebp-Ch]
    int32_t entIndex; // [esp+10h] [ebp-8h]
    gentity_s *clientEnt; // [esp+14h] [ebp-4h]

    tempEnt = StartScriptPlaySoundOnEnt(entref);
    team = Scr_GetConstString(1);
    if (team != scr_const.allies && team != scr_const.axis)
    {
        Scr_Error(va("Illegal team string '%s'. Must be allies, or axis.", SL_ConvertToString(team)));
    }
    if (team == scr_const.allies)
        teamNum = 2;
    else
        teamNum = 1;
    if (Scr_GetNumParam() >= 3)
    {
        ignoreClientEnt = Scr_GetEntity(2);
        if (!ignoreClientEnt->client)
        {
            Scr_ObjectError(va("entity %i is not a player", ignoreClientEnt->s.number));
        }
    }
    else
    {
        ignoreClientEnt = 0;
    }
    tempEnt->r.clientMask[0] = -1;
    tempEnt->r.clientMask[1] = -1;
    clientEnt = g_entities;
    for (entIndex = 0; entIndex < g_maxclients->current.integer; ++entIndex)
    {
        if (clientEnt->r.inuse && clientEnt != ignoreClientEnt)
        {
            if (!clientEnt->client)
                MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 1597, 0, "%s", "clientEnt->client");
            if (clientEnt->client->sess.cs.team == teamNum)
                tempEnt->r.clientMask[clientEnt->s.number >> 5] &= ~(1 << (clientEnt->s.number & 0x1F));
        }
        ++clientEnt;
    }
}

void __cdecl ScrCmd_PlaySoundToPlayer(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *tempEnt; // [esp+0h] [ebp-8h]
    gentity_s *clientEnt; // [esp+4h] [ebp-4h]

    tempEnt = StartScriptPlaySoundOnEnt(entref);
    clientEnt = Scr_GetEntity(1);
    if (!clientEnt->client)
    {
        v1 = va("entity %i is not a player", clientEnt->s.number);
        Scr_ObjectError(v1);
    }
    tempEnt->r.clientMask[0] = -1;
    tempEnt->r.clientMask[1] = -1;
    tempEnt->r.clientMask[clientEnt->s.number >> 5] &= ~(1 << (clientEnt->s.number & 0x1F));
}

void __cdecl ScrCmd_PlaySoundAsMaster(scr_entref_t entref)
{
    uint32_t  NumParam; // eax
    const char *pszSoundName; // [esp+0h] [ebp-10h]
    uint8_t iSoundIndex; // [esp+7h] [ebp-9h]
    gentity_s *pEnt; // [esp+8h] [ebp-8h]
    gentity_s *pTempEnt; // [esp+Ch] [ebp-4h]

    pEnt = GetEntity(entref);
    pszSoundName = Scr_GetString(0);
    iSoundIndex = G_SoundAliasIndex((char*)pszSoundName);
    pTempEnt = G_TempEntity(pEnt->r.currentOrigin, 4);
    pTempEnt->r.svFlags |= 8u;
    pTempEnt->s.eventParm = iSoundIndex;
    if (Scr_GetNumParam() != 1)
    {
        NumParam = Scr_GetNumParam();
        Scr_Error(va("playsoundasmaster has %d parameters.  There should be exactly one.", NumParam));
    }
}

void __cdecl ScrCmd_PlayLoopSound(scr_entref_t entref)
{
    const char *pszSoundName; // [esp+0h] [ebp-Ch]
    gentity_s *pEnt; // [esp+8h] [ebp-4h]

    pEnt = GetEntity(entref);
    pszSoundName = Scr_GetString(0);
    pEnt->r.broadcastTime = -1;
    pEnt->s.loopSound = G_SoundAliasIndex((char*)pszSoundName);
}

void __cdecl ScrCmd_StopLoopSound(scr_entref_t entref)
{
    gentity_s *Entity; // edx

    Entity = GetEntity(entref);
    Entity->r.broadcastTime = level.time + 300;
    Entity->s.loopSound = 0;
}

void __cdecl ScrCmd_Delete(scr_entref_t entref)
{
    gentity_s *pEnt; // [esp+0h] [ebp-4h]

    pEnt = GetEntity(entref);
    if (pEnt->client)
        Scr_Error("Cannot delete a client entity");
    if (level.currentEntityThink == pEnt->s.number)
        Scr_Error("Cannot delete entity during its think");
    Scr_Notify(pEnt, scr_const.death, 0);
    G_FreeEntity(pEnt);
}

void __cdecl ScrCmd_SetModel(scr_entref_t entref)
{
    const char *modelName; // [esp+0h] [ebp-8h]
    gentity_s *pEnt; // [esp+4h] [ebp-4h]

    pEnt = GetEntity(entref);
    modelName = Scr_GetString(0);
    G_SetModel(pEnt, (char*)modelName);
    G_DObjUpdate(pEnt);
    SV_LinkEntity(pEnt);
}

void __cdecl ScrCmd_GetNormalHealth(scr_entref_t entref)
{
    float value; // [esp+0h] [ebp-Ch]
    gentity_s *pEnt; // [esp+8h] [ebp-4h]

    pEnt = GetEntity(entref);
    if (pEnt->client)
    {
        if (pEnt->health)
        {
            Scr_AddFloat((double)pEnt->health / (double)pEnt->client->sess.maxHealth);
        }
        else
        {
            Scr_AddFloat(0.0f);
        }
    }
    else
    {
        Scr_AddFloat((float)pEnt->health);
    }
}

void __cdecl ScrCmd_SetNormalHealth(scr_entref_t entref)
{
    int32_t newHealth; // [esp+10h] [ebp-Ch]
    float normalHealth; // [esp+14h] [ebp-8h]
    gentity_s *ent; // [esp+18h] [ebp-4h]

    ent = GetEntity(entref);
    normalHealth = Scr_GetFloat(0);
    if (normalHealth > 1.0)
        normalHealth = 1.0;
    if (ent->client)
    {
        newHealth = (int)((float)ent->client->sess.maxHealth * normalHealth);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, va("%c \"%i\"", 74, 0));
    }
    else if (ent->maxHealth)
    {
        newHealth = (int)((double)ent->maxHealth * normalHealth);
    }
    else
    {
        newHealth = (int)normalHealth;
    }
    if (newHealth > 0)
        ent->health = newHealth;
    else
        Com_PrintError(23, "ERROR: Cannot setnormalhealth to 0 or below.\n");
}

void __cdecl ScrCmd_Show(scr_entref_t entref)
{
    gentity_s *Entity; // eax

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags &= ~0x20u;
    Entity->r.clientMask[0] = 0;
    Entity->r.clientMask[1] = 0;
}

void __cdecl ScrCmd_Hide(scr_entref_t entref)
{
    gentity_s *Entity; // eax

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags |= 0x20u;
    Entity->r.clientMask[0] = -1;
    Entity->r.clientMask[1] = -1;
}

void __cdecl ScrCmd_LaserOn(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    ent->s.lerp.eFlags |= 0x4000u;
}

void __cdecl ScrCmd_LaserOff(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    ent->s.lerp.eFlags &= ~0x4000u;
}

void __cdecl ScrCmd_ShowToPlayer(scr_entref_t entref)
{
    gentity_s *pEnt; // [esp+0h] [ebp-8h]
    gentity_s *clientEnt; // [esp+4h] [ebp-4h]

    pEnt = GetEntity(entref);
    clientEnt = Scr_GetEntity(0);
    if (clientEnt->s.number < 64)
    {
        pEnt->s.lerp.eFlags &= ~0x20u;
        pEnt->r.clientMask[clientEnt->s.number >> 5] &= ~(1 << (clientEnt->s.number & 0x1F));
    }
    else
    {
        Scr_Error("showToClient error: param must be a client entity\n");
    }
}

void __cdecl ScrCmd_SetContents(scr_entref_t entref)
{
    int32_t contents; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    contents = Scr_GetInt(0);
    Scr_AddInt(ent->r.contents);
    ent->r.contents = contents;
    SV_LinkEntity(ent);
}

void __cdecl ScrCmd_GetStance(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->client)
    {
        if ((ent->client->ps.pm_flags & PMF_PRONE) != 0)
        {
            Scr_AddConstString(scr_const.prone);
        }
        else if ((ent->client->ps.pm_flags & PMF_DUCKED) != 0)
        {
            Scr_AddConstString(scr_const.crouch);
        }
        else
        {
            Scr_AddConstString(scr_const.stand);
        }
    }
    else
    {
        Scr_Error("GetStance is only defined for players.");
    }
}

void __cdecl Scr_SetStableMissile(scr_entref_t entref)
{
    gentityFlags_t  v1; // eax
    int32_t stableMissile; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    stableMissile = Scr_GetInt(0);
    if (ent->s.eType != ET_PLAYER)
        Scr_Error("Type should be a player");
    if (stableMissile)
        v1 = ent->flags | FL_STABLE_MISSILES;
    else
        v1 = ent->flags & ~(FL_STABLE_MISSILES);
    ent->flags = v1;
}

void __cdecl GScr_SetCursorHint(scr_entref_t entref)
{
    const char *pszHint; // [esp+0h] [ebp-Ch]
    gentity_s *pEnt; // [esp+4h] [ebp-8h]
    int32_t i; // [esp+8h] [ebp-4h]
    int32_t ia; // [esp+8h] [ebp-4h]

    pEnt = GetEntity(entref);
    
    iassert(pEnt->s.eType != ET_MISSILE);

    pszHint = Scr_GetString(0);
    if ((pEnt->classname == scr_const.trigger_use || pEnt->classname == scr_const.trigger_use_touch)
        && !I_stricmp(pszHint, "HINT_INHERIT"))
    {
        pEnt->s.un2.hintString = -1;
    }
    else
    {
        for (i = 1; i < 5; ++i)
        {
            if (!I_stricmp(pszHint, hintStrings[i]))
            {
                pEnt->s.un2.hintString = i;
                return;
            }
        }
        Com_Printf(23, "List of valid hint type strings\n");
        if (pEnt->classname == scr_const.trigger_use || pEnt->classname == scr_const.trigger_use_touch)
            Com_Printf(23, "HINT_INHERIT (for trigger_use or trigger_use_touch entities only)\n");
        for (ia = 1; ia < 5; ++ia)
            Com_Printf(23, "%s\n", hintStrings[ia]);
        Scr_Error(va("%s is not a valid hint type. See above for list of valid hint types\n", pszHint));
    }
}

int32_t __cdecl G_GetHintStringIndex(int32_t *piIndex, char *pszString)
{
    char szConfigString[1024]; // [esp+14h] [ebp-408h] BYREF
    int32_t i; // [esp+418h] [ebp-4h]

    for (i = 0; i < 32; ++i)
    {
        SV_GetConfigstring(i + 277, szConfigString, 1024);
        if (!szConfigString[0])
        {
            SV_SetConfigstring(i + 277, pszString);
            *piIndex = i;
            return 1;
        }
        if (!strcmp(pszString, szConfigString))
        {
            *piIndex = i;
            return 1;
        }
    }
    *piIndex = -1;
    return 0;
}

void __cdecl GScr_SetHintString(scr_entref_t entref)
{
    const char *String; // eax
    uint32_t  NumParam; // eax
    const char *v3; // eax
    char szHint[1024]; // [esp+0h] [ebp-410h] BYREF
    int32_t type; // [esp+404h] [ebp-Ch]
    gentity_s *pEnt; // [esp+408h] [ebp-8h]
    int32_t i; // [esp+40Ch] [ebp-4h] BYREF

    pEnt = GetEntity(entref);
    if (pEnt->classname != scr_const.trigger_use && pEnt->classname != scr_const.trigger_use_touch)
        Scr_Error("The setHintString command only works on trigger_use or trigger_use_touch entities.\n");
    type = Scr_GetType(0);
    if (type != 2 || (String = Scr_GetString(0), I_stricmp(String, "")))//I_stricmp(String, &::String)))
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(0, NumParam - 1, "Hint String", szHint, 0x400u);
        if (!G_GetHintStringIndex(&i, szHint))
        {
            v3 = va("Too many different hintstring values. Max allowed is %i different strings", 32);
            Scr_Error(v3);
        }
        pEnt->s.un1.scale = (uint8_t)i;
    }
    else
    {
        pEnt->s.un1.scale = 255;
    }
}

void __cdecl GScr_UseTriggerRequireLookAt(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->classname != scr_const.trigger_use)
        Scr_Error("The UseTriggerRequireLookAt command only works on trigger_use entities.\n");
    ent->trigger.requireLookAt = 1;
}

void __cdecl GScr_GetEntityNumber(scr_entref_t entref)
{
    gentity_s *pEnt; // [esp+0h] [ebp-4h]

    pEnt = GetEntity(entref);
    Scr_AddInt(pEnt->s.number);
}

void __cdecl GScr_EnableGrenadeTouchDamage(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->classname != scr_const.trigger_damage)
        Scr_Error("Currently on supported on damage triggers");
    ent->flags |= FL_GRENADE_TOUCH_DAMAGE;
}

void __cdecl GScr_DisableGrenadeTouchDamage(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->classname != scr_const.trigger_damage)
        Scr_Error("Currently on supported on damage triggers");
    ent->flags &= ~(FL_GRENADE_TOUCH_DAMAGE);
}

void __cdecl GScr_MissileSetTarget(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *Entity; // [esp+0h] [ebp-14h]
    gentity_s *missile; // [esp+Ch] [ebp-8h]

    missile = GetEntity(entref);
    if (Scr_GetType(0))
        Entity = Scr_GetEntity(0);
    else
        Entity = 0;
    if (missile->classname != scr_const.rocket)
    {
        v1 = va("Entity %i is not a rocket\n", missile->s.number);
        Scr_Error(v1);
    }
    missile->missileTargetEnt.setEnt(Entity);
    if (Scr_GetNumParam() <= 1)
    {
        missile->mover.pos2[1] = 0.0;
        missile->mover.pos2[2] = 0.0;
        missile->mover.pos3[0] = 0.0;
    }
    else
    {
        Scr_GetVector(1u, &missile->mover.pos2[1]);
    }
}

void __cdecl GScr_EnableAimAssist(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (!ent->r.bmodel)
        Scr_Error("Currently only supported on entities with brush models");
    ent->s.lerp.eFlags |= 0x800u;
}

void __cdecl GScr_DisableAimAssist(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (!ent->r.bmodel)
        Scr_Error("Currently only supported on entities with brush models");
    ent->s.lerp.eFlags &= ~0x800u;
}

void __cdecl G_InitObjectives()
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 16; ++i)
        ClearObjective(&level.objectives[i]);
}

int32_t __cdecl ObjectiveStateIndexFromString(objectiveState_t *piStateIndex, uint32_t  stateString)
{
    if (stateString == scr_const.empty)
    {
        *piStateIndex = OBJST_EMPTY;
    }
    else if (stateString == scr_const.invisible)
    {
        *piStateIndex = OBJST_INVISIBLE;
    }
    else if (stateString == scr_const.current)
    {
        *piStateIndex = OBJST_CURRENT;
    }
    else
    {
        if (stateString != scr_const.active)
        {
            *piStateIndex = OBJST_EMPTY;
            return 0;
        }
        *piStateIndex = OBJST_ACTIVE;
    }
    return 1;
}

void __cdecl ClearObjective(objective_t *obj)
{
    obj->state = OBJST_EMPTY;
    obj->origin[0] = 0.0;
    obj->origin[1] = 0.0;
    obj->origin[2] = 0.0;
    obj->entNum = ENTITYNUM_NONE;
    obj->teamNum = 0;
    obj->icon = 0;
}

void Scr_Objective_Add()
{
    objective_t *result; // eax
    objectiveState_t state; // [esp+Ch] [ebp-14h] BYREF
    objective_t *obj; // [esp+10h] [ebp-10h]
    int32_t numParam; // [esp+14h] [ebp-Ch]
    uint16_t stateName; // [esp+18h] [ebp-8h]
    int32_t objNum; // [esp+1Ch] [ebp-4h]

    numParam = Scr_GetNumParam();
    if (numParam < 2)
        Scr_Error(
            "objective_add needs at least the first two parameters out of its parameter list of: index state [string] [position]\n");
    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }
    obj = &level.objectives[objNum];
    ClearObjective_OnEntity(obj);
    stateName = Scr_GetConstString(1);
    if (!ObjectiveStateIndexFromString(&state, stateName))
    {
        Scr_ParamError(1u, va("Illegal objective state \"%s\". Valid states are \"empty\", \"invisible\", \"current\", \"active\"\n", SL_ConvertToString(stateName)));
    }
    result = obj;
    obj->state = state;
    if (numParam >= 3)
    {
        Scr_GetVector(2u, obj->origin);
        obj->origin[0] = (float)(int)obj->origin[0];
        obj->origin[1] = (float)(int)obj->origin[1];
        obj->origin[2] = (float)(int)obj->origin[2];
        result = obj;
        obj->entNum = ENTITYNUM_NONE;
        if (numParam >= 4)
            SetObjectiveIcon(obj, 3u);
    }
    obj->teamNum = 0;
}

void __cdecl ClearObjective_OnEntity(objective_t *obj)
{
    gentity_s *pEnt; // [esp+0h] [ebp-4h]

    if (obj->entNum != ENTITYNUM_NONE)
    {
        pEnt = &g_entities[obj->entNum];
        if (pEnt->r.inuse)
            pEnt->r.svFlags &= ~0x10u;
        obj->entNum = ENTITYNUM_NONE;
    }
}

void __cdecl SetObjectiveIcon(objective_t *obj, uint32_t  paramNum)
{
    const char *shaderName; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    shaderName = Scr_GetString(paramNum);
    for (i = 0; shaderName[i]; ++i)
    {
        if (shaderName[i] <= 31 || shaderName[i] >= 127)
        {
            Scr_ParamError(3u, va("Illegal character '%c'(ascii %i) in objective icon name: %s\n", shaderName[i], (uint8_t)shaderName[i], shaderName));
        }
    }
    if (i >= 64)
    {
        Scr_ParamError(3, va("Objective icon name is too long (> %i): %s\n", 63, shaderName));
    }
    obj->icon = G_MaterialIndex(shaderName);
}

void Scr_Objective_Delete()
{
    int32_t objNum; // [esp+0h] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }

    ClearObjective_OnEntity(&level.objectives[objNum]);
    ClearObjective(&level.objectives[objNum]);
}

void Scr_Objective_State()
{
    objectiveState_t state; // [esp+0h] [ebp-10h] BYREF
    objective_t *obj; // [esp+4h] [ebp-Ch]
    uint16_t stateName; // [esp+8h] [ebp-8h]
    int32_t objNum; // [esp+Ch] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }
    obj = &level.objectives[objNum];
    stateName = Scr_GetConstString(1);
    if (!ObjectiveStateIndexFromString(&state, stateName))
    {
        Scr_ParamError(1, va("Illegal objective state \"%s\". Valid states are \"empty\", \"invisible\", \"current\", \"active\"\n", Scr_GetString(1)));
    }
    obj->state = state;
    if (state == OBJST_EMPTY || state == OBJST_INVISIBLE)
        ClearObjective_OnEntity(obj);
}

void Scr_Objective_Icon()
{
    int32_t objNum; // [esp+0h] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }
    SetObjectiveIcon(&level.objectives[objNum], 1u);
}

void Scr_Objective_Position()
{
    objective_t *result; // eax
    objective_t *obj; // [esp+Ch] [ebp-8h]
    int32_t objNum; // [esp+10h] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }
    obj = &level.objectives[objNum];
    ClearObjective_OnEntity(obj);
    Scr_GetVector(1u, obj->origin);
    obj->origin[0] = (float)(int)obj->origin[0];
    obj->origin[1] = (float)(int)obj->origin[1];
    result = obj;
    obj->origin[2] = (float)(int)obj->origin[2];
}

objective_t *Scr_Objective_OnEntity()
{
    const char *v0; // eax
    objective_t *result; // eax
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int32_t objNum; // [esp+8h] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        v0 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15);
        Scr_ParamError(0, v0);
    }
    ClearObjective_OnEntity(&level.objectives[objNum]);
    ent = Scr_GetEntity(1);
    ent->r.svFlags |= 0x10u;
    result = &level.objectives[objNum];
    result->entNum = ent->s.number;
    return result;
}

void Scr_Objective_Current()
{
    const char *v0; // eax
    objective_t *obj; // [esp+0h] [ebp-50h]
    int32_t numParam; // [esp+4h] [ebp-4Ch]
    int32_t makeCurrent[16]; // [esp+8h] [ebp-48h] BYREF
    int32_t i; // [esp+48h] [ebp-8h]
    int32_t objNum; // [esp+4Ch] [ebp-4h]

    numParam = Scr_GetNumParam();
    memset((uint8_t *)makeCurrent, 0, sizeof(makeCurrent));
    for (i = 0; i < numParam; ++i)
    {
        objNum = Scr_GetInt(i);
        if ((uint32_t )objNum >= 0x10)
        {
            v0 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15);
            Scr_ParamError(i, v0);
        }
        makeCurrent[objNum] = 1;
    }
    for (objNum = 0; objNum < 16; ++objNum)
    {
        obj = &level.objectives[objNum];
        if (makeCurrent[objNum])
        {
            obj->state = OBJST_CURRENT;
        }
        else if (obj->state == OBJST_CURRENT)
        {
            obj->state = OBJST_ACTIVE;
        }
    }
}

void GScr_Objective_Team()
{
    objective_t *obj; // [esp+0h] [ebp-Ch]
    uint16_t team; // [esp+4h] [ebp-8h]
    int32_t objNum; // [esp+8h] [ebp-4h]

    objNum = Scr_GetInt(0);
    if ((uint32_t )objNum >= 0x10)
    {
        Scr_ParamError(0, va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", objNum, 15));
    }
    obj = &level.objectives[objNum];
    team = Scr_GetConstString(1);
    if (team == scr_const.allies)
    {
        obj->teamNum = 2;
    }
    else if (team == scr_const.axis)
    {
        obj->teamNum = 1;
    }
    else if (team == scr_const.none)
    {
        obj->teamNum = 0;
    }
    else
    {
        Scr_ParamError(1u, va("Illegal team string '%s'. Must be allies, axis, or none.", SL_ConvertToString(team)));
    }
}

void GScr_LogPrint()
{
    uint32_t  v0; // [esp+0h] [ebp-428h]
    int32_t iStringLen; // [esp+10h] [ebp-418h]
    char string[1024]; // [esp+18h] [ebp-410h] BYREF
    int32_t iNumParms; // [esp+41Ch] [ebp-Ch]
    int32_t i; // [esp+420h] [ebp-8h]
    const char *pszToken; // [esp+424h] [ebp-4h]

    string[0] = 0;
    iStringLen = 0;
    iNumParms = Scr_GetNumParam();
    for (i = 0; i < iNumParms; ++i)
    {
        pszToken = Scr_GetString(i);
        v0 = strlen(pszToken);
        if ((int)(v0 + iStringLen) >= 1024)
            break;
        I_strncat(string, 1024, (char *)pszToken);
        iStringLen += v0;
    }
    G_LogPrintf(string);
}

void GScr_WorldEntNumber()
{
    Scr_AddInt(ENTITYNUM_WORLD);
}

int32_t GScr_Obituary()
{
    int32_t result; // eax
    gentity_s *pOtherEnt; // [esp+0h] [ebp-14h]
    const char *pszWeapon; // [esp+4h] [ebp-10h]
    uint32_t  iWeaponNum; // [esp+8h] [ebp-Ch]
    gentity_s *pEnt; // [esp+Ch] [ebp-8h]
    int32_t iMODNum; // [esp+10h] [ebp-4h]

    pszWeapon = Scr_GetString(2);
    iWeaponNum = G_GetWeaponIndexForName(pszWeapon);
    iMODNum = G_MeansOfDeathFromScriptParam(3);
    pOtherEnt = Scr_GetEntity(0);
    pEnt = G_TempEntity(vec3_origin, 66);
    pEnt->s.otherEntityNum = pOtherEnt->s.number;
    if (Scr_GetType(1) == 1 && Scr_GetPointerType(1) == 20)
        pEnt->s.attackerEntityNum = Scr_GetEntity(1)->s.number;
    else
        pEnt->s.attackerEntityNum = ENTITYNUM_WORLD;
    pEnt->r.svFlags = 8;
    if (iMODNum == 15 || iMODNum == 7 || iMODNum == 8 || iMODNum == 12 || iMODNum == 11 || iMODNum == 9)
    {
        result = iMODNum | 0x80;
        pEnt->s.eventParm = iMODNum | 0x80;
    }
    else
    {
        result = iWeaponNum;
        pEnt->s.eventParm = iWeaponNum;
    }
    return result;
}

void GScr_positionWouldTelefrag()
{
    int32_t entityList[1024]; // [esp+0h] [ebp-1030h] BYREF
    float sum[3]; // [esp+1000h] [ebp-30h] BYREF
    gentity_s *v2; // [esp+100Ch] [ebp-24h]
    float maxs[3]; // [esp+1010h] [ebp-20h] BYREF
    int32_t v4; // [esp+101Ch] [ebp-14h]
    int32_t i; // [esp+1020h] [ebp-10h]
    float vectorValue[3]; // [esp+1024h] [ebp-Ch] BYREF

    Scr_GetVector(0, vectorValue);
    Vec3Add(vectorValue, playerMins, sum);
    Vec3Add(vectorValue, playerMaxs, maxs);
    v4 = CM_AreaEntities(sum, maxs, entityList, 1024, 0x2000000);
    for (i = 0; i < v4; ++i)
    {
        v2 = &g_entities[entityList[i]];
        if (v2->client && v2->client->ps.pm_type < PM_DEAD)
        {
            Scr_AddInt(1);
            return;
        }
    }
    Scr_AddInt(0);
}

void GScr_getStartTime()
{
    Scr_AddInt(level.startTime);
}

void GScr_PrecacheMenu()
{
    const char *pszNewMenu; // [esp+0h] [ebp-410h]
    int32_t iConfigNum; // [esp+4h] [ebp-40Ch]
    int32_t iConfigNuma; // [esp+4h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+8h] [ebp-408h] BYREF

    pszNewMenu = Scr_GetString(0);
    for (iConfigNum = 0; iConfigNum < 32; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 1970, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszNewMenu))
        {
            Com_DPrintf(23, "Script tried to precache the menu '%s' more than once\n", pszNewMenu);
            return;
        }
    }
    for (iConfigNuma = 0; iConfigNuma < 32; ++iConfigNuma)
    {
        SV_GetConfigstring(iConfigNuma + 1970, szConfigString, 1024);
        if (!szConfigString[0])
            break;
    }
    if (iConfigNuma == 32)
    {
        Scr_Error(va("Too many menus precached. Max allowed menus is %i", 32));
    }
    SV_SetConfigstring(iConfigNuma + 1970, pszNewMenu);
}

int32_t __cdecl GScr_GetScriptMenuIndex(const char *pszMenu)
{
    const char *v2; // eax
    int32_t iConfigNum; // [esp+0h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+4h] [ebp-408h] BYREF

    for (iConfigNum = 0; iConfigNum < 32; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 1970, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszMenu))
            return iConfigNum;
    }
    v2 = va("Menu '%s' was not precached\n", pszMenu);
    Scr_Error(v2);
    return 0;
}

void GScr_PrecacheStatusIcon()
{
    const char *pszNewIcon; // [esp+0h] [ebp-410h]
    int32_t iConfigNum; // [esp+4h] [ebp-40Ch]
    int32_t iConfigNuma; // [esp+4h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+8h] [ebp-408h] BYREF

    pszNewIcon = Scr_GetString(0);
    for (iConfigNum = 0; iConfigNum < 8; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 2259, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszNewIcon))
        {
            Com_DPrintf(23, "Script tried to precache the player status icon '%s' more than once\n", pszNewIcon);
            return;
        }
    }
    for (iConfigNuma = 0; iConfigNuma < 8; ++iConfigNuma)
    {
        SV_GetConfigstring(iConfigNuma + 2259, szConfigString, 1024);
        if (!szConfigString[0])
            break;
    }
    if (iConfigNuma == 8)
    {
        Scr_Error(va("Too many player status icons precached. Max allowed is %i", 8));
    }
    SV_SetConfigstring(iConfigNuma + 2259, pszNewIcon);
}

int32_t __cdecl GScr_GetStatusIconIndex(const char *pszIcon)
{
    int32_t iConfigNum; // [esp+0h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+4h] [ebp-408h] BYREF

    if (!*pszIcon)
        return 0;

    for (iConfigNum = 0; iConfigNum < 8; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 2259, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszIcon))
            return iConfigNum + 1;
    }

    Scr_Error(va("Status icon '%s' was not precached\n", pszIcon));
    return 0;
}

void GScr_PrecacheHeadIcon()
{
    const char *pszNewIcon; // [esp+0h] [ebp-410h]
    int32_t iConfigNum; // [esp+4h] [ebp-40Ch]
    int32_t iConfigNuma; // [esp+4h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+8h] [ebp-408h] BYREF

    pszNewIcon = Scr_GetString(0);
    for (iConfigNum = 0; iConfigNum < 15; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 2267, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszNewIcon))
        {
            Com_DPrintf(23, "Script tried to precache the player head icon '%s' more than once\n", pszNewIcon);
            return;
        }
    }
    for (iConfigNuma = 0; iConfigNuma < 15; ++iConfigNuma)
    {
        SV_GetConfigstring(iConfigNuma + 2267, szConfigString, 1024);
        if (!szConfigString[0])
            break;
    }
    if (iConfigNuma == 15)
    {
        Scr_Error(va("Too many player head icons precached. Max allowed is %i", 15));
    }
    SV_SetConfigstring(iConfigNuma + 2267, pszNewIcon);
}

int32_t __cdecl GScr_GetHeadIconIndex(const char *pszIcon)
{
    const char *v2; // eax
    int32_t iConfigNum; // [esp+0h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+4h] [ebp-408h] BYREF

    if (!*pszIcon)
        return 0;

    for (iConfigNum = 0; iConfigNum < 15; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 2267, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszIcon))
            return iConfigNum + 1;
    }

    Scr_Error(va("Head icon '%s' was not precached\n", pszIcon));
    return 0;
}

void GScr_WeaponClipSize()
{
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddInt(weapDef->iClipSize);
}

void GScr_WeaponIsSemiAuto()
{
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddInt(weapDef->fireType == WEAPON_FIRETYPE_SINGLESHOT);
}

void GScr_WeaponIsBoltAction()
{
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddInt(weapDef->bBoltAction);
}

void GScr_WeaponType()
{
    const char *WeaponTypeName; // eax
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    WeaponTypeName = BG_GetWeaponTypeName(weapDef->weapType);
    Scr_AddString(WeaponTypeName);
}

void GScr_WeaponClass()
{
    const char *WeaponClassName; // eax
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    if (weaponIndex)
    {
        weapDef = BG_GetWeaponDef(weaponIndex);
        WeaponClassName = BG_GetWeaponClassName(weapDef->weapClass);
        Scr_AddString(WeaponClassName);
    }
    else
    {
        Scr_AddString("none");
    }
}

void GScr_WeaponInventoryType()
{
    const char *WeaponInventoryTypeName; // eax
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    WeaponInventoryTypeName = BG_GetWeaponInventoryTypeName(weapDef->inventoryType);
    Scr_AddString(WeaponInventoryTypeName);
}

void GScr_WeaponStartAmmo()
{
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddInt(weapDef->iStartAmmo);
}

void GScr_WeaponMaxAmmo()
{
    const char *weaponName; // [esp+0h] [ebp-Ch]
    int32_t weaponIndex; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddInt(weapDef->iMaxAmmo);
}

void GScr_WeaponAltWeaponName()
{
    const char *weaponName; // [esp+0h] [ebp-14h]
    int32_t altWeaponIndex; // [esp+4h] [ebp-10h]
    int32_t weaponIndex; // [esp+8h] [ebp-Ch]
    WeaponDef *altWeapDef; // [esp+Ch] [ebp-8h]

    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    altWeaponIndex = BG_GetWeaponDef(weaponIndex)->altWeaponIndex;
    if (altWeaponIndex)
    {
        altWeapDef = BG_GetWeaponDef(altWeaponIndex);
        if (!altWeapDef)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 2990, 0, "%s", "altWeapDef");
        Scr_AddString((char *)altWeapDef->szInternalName);
    }
    else
    {
        Scr_AddConstString(scr_const.none);
    }
}

void GScr_WeaponFireTime()
{
    float value; // [esp+4h] [ebp-10h]
    int32_t iWeaponIndex; // [esp+Ch] [ebp-8h]
    const char *pszWeaponName; // [esp+10h] [ebp-4h]

    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    if (iWeaponIndex)
    {
        value = (double)BG_GetWeaponDef(iWeaponIndex)->iFireTime * EQUAL_EPSILON;
        Scr_AddFloat(value);
    }
    else
    {
        Scr_AddFloat(0.0f);
    }
}

void GScr_IsWeaponClipOnly()
{
    uint32_t  IsClipOnly; // eax
    const char *weapName; // [esp+0h] [ebp-8h]
    int32_t weapIdx; // [esp+4h] [ebp-4h]

    weapName = Scr_GetString(0);
    weapIdx = G_GetWeaponIndexForName(weapName);
    if (weapIdx)
    {
        IsClipOnly = BG_WeaponIsClipOnly(weapIdx);
        Scr_AddBool(IsClipOnly);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void GScr_IsWeaponDetonationTimed()
{
    const char *weapName; // [esp+0h] [ebp-Ch]
    int32_t weapIdx; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapName = Scr_GetString(0);
    weapIdx = G_GetWeaponIndexForName(weapName);
    if (weapIdx)
    {
        weapDef = BG_GetWeaponDef(weapIdx);
        Scr_AddBool(weapDef->timedDetonation);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void GScr_PrecacheLocationSelector()
{
    const char *v0; // eax
    int32_t iConfigNum; // [esp+0h] [ebp-40Ch]
    int32_t iConfigNuma; // [esp+0h] [ebp-40Ch]
    char szConfigString[1024]; // [esp+4h] [ebp-408h] BYREF
    const char *pszNewMtl; // [esp+408h] [ebp-4h]

    pszNewMtl = Scr_GetString(0);
    for (iConfigNum = 0; iConfigNum < 3; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 827, szConfigString, 1024);
        if (!I_stricmp(szConfigString, pszNewMtl))
        {
            Com_DPrintf(23, "Script tried to precache the location selector '%s' more than once\n", pszNewMtl);
            return;
        }
    }
    for (iConfigNuma = 0; iConfigNuma < 3; ++iConfigNuma)
    {
        SV_GetConfigstring(iConfigNuma + 827, szConfigString, 1024);
        if (!szConfigString[0])
            break;
    }
    if (iConfigNuma == 3)
    {
        v0 = va("Too many location selectors precached. Max allowed is %i", 3);
        Scr_Error(v0);
    }
    SV_SetConfigstring(iConfigNuma + 827, (char *)pszNewMtl);
}

int32_t __cdecl GScr_GetLocSelIndex(const char *mtlName)
{
    const char *v2; // eax
    int32_t iConfigNum; // [esp+0h] [ebp-40Ch]
    char szConfigString[1028]; // [esp+4h] [ebp-408h] BYREF

    if (!mtlName)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 3095, 0, "%s", "mtlName");
    if (!*mtlName)
        return 0;
    for (iConfigNum = 0; iConfigNum < 3; ++iConfigNum)
    {
        SV_GetConfigstring(iConfigNum + 827, szConfigString, 1024);
        if (!I_stricmp(szConfigString, mtlName))
            return iConfigNum + 1;
    }
    v2 = va("Location selector '%s' was not precached\n", mtlName);
    Scr_Error(v2);
    return 0;
}

void Scr_BulletTrace()
{
    const char *value; // eax
    float vNorm[3]; // [esp+14h] [ebp-74h] BYREF
    float vEnd[3]; // [esp+20h] [ebp-68h] BYREF
    gentity_s *pIgnoreEnt; // [esp+2Ch] [ebp-5Ch]
    int32_t iClipMask; // [esp+30h] [ebp-58h]
    trace_t trace; // [esp+38h] [ebp-50h] BYREF
    float endpos[3]; // [esp+64h] [ebp-24h] BYREF
    int32_t iIgnoreEntNum; // [esp+70h] [ebp-18h]
    int32_t iSurfaceTypeIndex; // [esp+74h] [ebp-14h]
    float vStart[3]; // [esp+78h] [ebp-10h] BYREF
    uint16_t hitEntId; // [esp+84h] [ebp-4h]

    pIgnoreEnt = 0;
    iIgnoreEntNum = ENTITYNUM_NONE;
    iClipMask = 0x2806831;
    Scr_GetVector(0, vStart);
    Scr_GetVector(1u, vEnd);
    if (!Scr_GetInt(2))
        iClipMask &= ~0x2000000u;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
    {
        pIgnoreEnt = Scr_GetEntity(3);
        iIgnoreEntNum = pIgnoreEnt->s.number;
    }
    G_LocationalTrace(&trace, vStart, vEnd, iIgnoreEntNum, iClipMask, 0);
    Scr_MakeArray();
    Scr_AddFloat(trace.fraction);
    Scr_AddArrayStringIndexed(scr_const.fraction);
    Vec3Lerp(vStart, vEnd, trace.fraction, endpos);
    Scr_AddVector(endpos);
    Scr_AddArrayStringIndexed(scr_const.position);
    hitEntId = Trace_GetEntityHitId(&trace);
    if (hitEntId == ENTITYNUM_NONE || hitEntId == ENTITYNUM_WORLD)
        Scr_AddUndefined();
    else
        Scr_AddEntity(&g_entities[hitEntId]);
    Scr_AddArrayStringIndexed(scr_const.entity);
    if (trace.fraction >= 1.0)
    {
        Vec3Sub(vEnd, vStart, vNorm);
        Vec3Normalize(vNorm);
        Scr_AddVector(vNorm);
        Scr_AddArrayStringIndexed(scr_const.normal);
        Scr_AddConstString(scr_const.none);
        Scr_AddArrayStringIndexed(scr_const.surfacetype);
    }
    else
    {
        Scr_AddVector(trace.normal);
        Scr_AddArrayStringIndexed(scr_const.normal);
        iSurfaceTypeIndex = (trace.surfaceFlags & 0x1F00000) >> 20;
        value = Com_SurfaceTypeToName(iSurfaceTypeIndex);
        Scr_AddString(value);
        Scr_AddArrayStringIndexed(scr_const.surfacetype);
    }
}

void Scr_BulletTracePassed()
{
    uint32_t  v0; // eax
    float vEnd[3]; // [esp+0h] [ebp-28h] BYREF
    gentity_s *pIgnoreEnt; // [esp+Ch] [ebp-1Ch]
    int32_t iClipMask; // [esp+10h] [ebp-18h]
    int32_t iIgnoreEntNum; // [esp+18h] [ebp-10h]
    float vStart[3]; // [esp+1Ch] [ebp-Ch] BYREF

    pIgnoreEnt = 0;
    iIgnoreEntNum = ENTITYNUM_NONE;
    iClipMask = 0x2806831;
    Scr_GetVector(0, vStart);
    Scr_GetVector(1u, vEnd);
    if (!Scr_GetInt(2))
        iClipMask &= ~0x2000000u;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
    {
        pIgnoreEnt = Scr_GetEntity(3);
        iIgnoreEntNum = pIgnoreEnt->s.number;
    }
    v0 = G_LocationalTracePassed(vStart, vEnd, iIgnoreEntNum, ENTITYNUM_NONE, iClipMask, 0);
    Scr_AddBool(v0);
}

void __cdecl Scr_SightTracePassed()
{
    float vEnd[3]; // [esp+0h] [ebp-2Ch] BYREF
    gentity_s *pIgnoreEnt; // [esp+Ch] [ebp-20h]
    int32_t iClipMask; // [esp+10h] [ebp-1Ch]
    int32_t iIgnoreEntNum; // [esp+18h] [ebp-14h]
    int32_t hitNum; // [esp+1Ch] [ebp-10h] BYREF
    float vStart[3]; // [esp+20h] [ebp-Ch] BYREF

    pIgnoreEnt = 0;
    iIgnoreEntNum = ENTITYNUM_NONE;
    iClipMask = 0x2801803;
    Scr_GetVector(0, vStart);
    Scr_GetVector(1u, vEnd);
    if (!Scr_GetInt(2))
        iClipMask &= ~0x2000000u;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
    {
        pIgnoreEnt = Scr_GetEntity(3);
        iIgnoreEntNum = pIgnoreEnt->s.number;
    }
    G_SightTrace(&hitNum, vStart, vEnd, iIgnoreEntNum, iClipMask);
    Scr_AddBool(hitNum == 0);
}

void Scr_PhysicsTrace()
{
    float start[3]; // [esp+8h] [ebp-50h] BYREF
    float end[3]; // [esp+14h] [ebp-44h] BYREF
    float endpos[3]; // [esp+20h] [ebp-38h] BYREF
    trace_t trace; // [esp+2Ch] [ebp-2Ch] BYREF

    Scr_GetVector(0, start);
    Scr_GetVector(1u, end);
    G_TraceCapsule(&trace, start, (float *)vec3_origin, (float *)vec3_origin, end, ENTITYNUM_NONE, 0x820011);
    Vec3Lerp(start, end, trace.fraction, endpos);
    Scr_AddVector(endpos);
}

void Scr_PlayerPhysicsTrace()
{
    float start[3]; // [esp+8h] [ebp-50h] BYREF
    float end[3]; // [esp+14h] [ebp-44h] BYREF
    float endpos[3]; // [esp+20h] [ebp-38h] BYREF
    trace_t trace; // [esp+2Ch] [ebp-2Ch] BYREF

    Scr_GetVector(0, start);
    Scr_GetVector(1u, end);
    G_TraceCapsule(&trace, start, (float *)playerMins, (float *)playerMaxs, end, ENTITYNUM_NONE, 0x820011);
    Vec3Lerp(start, end, trace.fraction, endpos);
    Scr_AddVector(endpos);
}

void Scr_RandomInt()
{
    int32_t v0; // eax
    int32_t iMax; // [esp+0h] [ebp-4h]

    iMax = Scr_GetInt(0);
    if (iMax > 0)
    {
        v0 = irand(0, iMax);
        Scr_AddInt(v0);
    }
    else
    {
        Com_Printf(23, "RandomInt parm: %d  ", iMax);
        Scr_Error("RandomInt parm must be positive integer.\n");
    }
}

void Scr_RandomFloat()
{
    float fMax; // [esp+8h] [ebp-4h]

    fMax = Scr_GetFloat(0);
    Scr_AddFloat(flrand(0.0, fMax));
}

void Scr_RandomIntRange()
{
    int32_t v0; // eax
    int32_t iMax; // [esp+0h] [ebp-8h]
    int32_t iMin; // [esp+4h] [ebp-4h]

    iMin = Scr_GetInt(0);
    iMax = Scr_GetInt(1);
    if (iMax <= iMin)
    {
        Com_Printf(23, "RandomIntRange parms: %d %d ", iMin, iMax);
        Scr_Error("RandomIntRange range must be positive integer.\n");
    }
    v0 = irand(iMin, iMax);
    Scr_AddInt(v0);
}

void Scr_RandomFloatRange()
{
    float fMin; // [esp+10h] [ebp-8h]
    float fMax; // [esp+14h] [ebp-4h]

    fMin = Scr_GetFloat(0);
    fMax = Scr_GetFloat(1);
    if (fMin >= (double)fMax)
    {
        Com_Printf(
            23,
            "Scr_RandomFloatRange parms: %d %d ",
            fMin, fMax
            //(uint32_t )COERCE_UNSIGNED_INT64(fMin),
            //(uint32_t )HIDWORD(COERCE_UNSIGNED_INT64(fMin))
        );
        Scr_Error("Scr_RandomFloatRange range must be positive float.\n");
    }
    Scr_AddFloat(flrand(fMin, fMax));
}

void GScr_sin()
{
    float v1; // [esp+8h] [ebp-4h]

    v1 = Scr_GetFloat(0) * 0.01745329238474369;
    Scr_AddFloat(sin(v1));
}

void GScr_cos()
{
    float v1; // [esp+8h] [ebp-4h]

    v1 = Scr_GetFloat(0) * 0.01745329238474369;
    Scr_AddFloat(cos(v1));
}

void GScr_tan()
{
    float v1; // [esp+10h] [ebp-Ch]
    float sinT; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    v1 = Scr_GetFloat(0) * 0.01745329238474369;

    cosT = cos(v1);
    sinT = sin(v1);

    if (cosT == 0.0)
        Scr_Error("divide by 0");

    Scr_AddFloat(sinT / cosT);
}

void GScr_asin()
{
    const char *v0; // eax
    float v2; // [esp+Ch] [ebp-8h]
    float x; // [esp+10h] [ebp-4h]

    x = Scr_GetFloat(0);
    if (x < -1.0 || x > 1.0)
    {
        v0 = va("%g out of range", x);
        Scr_Error(v0);
    }
    v2 = asin(x);
    Scr_AddFloat(v2 * 57.2957763671875);
}

void GScr_acos()
{
    const char *v0; // eax
    float v2; // [esp+Ch] [ebp-8h]
    float x; // [esp+10h] [ebp-4h]

    x = Scr_GetFloat(0);
    if (x < -1.0 || x > 1.0)
    {
        v0 = va("%g out of range", x);
        Scr_Error(v0);
    }
    v2 = acos(x);
    Scr_AddFloat(v2 * 57.2957763671875);
}

void GScr_atan()
{
    float v1; // [esp+8h] [ebp-8h]
    float Float; // [esp+Ch] [ebp-4h]

    Float = Scr_GetFloat(0);
    v1 = atan(Float);
    Scr_AddFloat(v1 * 57.2957763671875);
}

void GScr_abs()
{
    float Float; // [esp+8h] [ebp-4h]

    Float = Scr_GetFloat(0);
    Scr_AddFloat(I_fabs(Float));
}

void GScr_min()
{
    float value; // [esp+4h] [ebp-10h]
    float v1; // [esp+8h] [ebp-Ch]
    float Float; // [esp+Ch] [ebp-8h]
    float v3; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(0);
    v3 = Scr_GetFloat(1);
    v1 = v3 - Float;

    if (v1 < 0.0)
        value = v3;
    else
        value = Float;

    Scr_AddFloat(value);
}

void GScr_max()
{
    float value; // [esp+4h] [ebp-10h]
    float v1; // [esp+8h] [ebp-Ch]
    float Float; // [esp+Ch] [ebp-8h]
    float v3; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(0);
    v3 = Scr_GetFloat(1);
    v1 = Float - v3;
    if (v1 < 0.0)
        value = v3;
    else
        value = Float;
    Scr_AddFloat(value);
}

void GScr_floor()
{
    float Float; // [esp+Ch] [ebp-4h]

    Float = Scr_GetFloat(0);
    Scr_AddFloat(floor(Float));
}

void GScr_ceil()
{
    float Float; // [esp+Ch] [ebp-4h]

    Float = Scr_GetFloat(0);
    Scr_AddFloat(ceil(Float));
}

void GScr_sqrt()
{
    VariableUnion value; // [esp+4h] [ebp-8h]
    float Float; // [esp+8h] [ebp-4h]

    Float = Scr_GetFloat(0);
    Scr_AddFloat(sqrt(Float));
}

void GScr_CastInt()
{
    VariableUnion v0; // eax
    double Float; // st7
    const char *TypeName; // eax
    int32_t Type; // [esp+0h] [ebp-4h]

    Type = Scr_GetType(0);
    switch (Type)
    {
    case 2:
        Scr_AddInt(atoi(Scr_GetString(0)));
        break;
    case 5:
        Float = Scr_GetFloat(0);
        Scr_AddInt((int)Float);
        break;
    case 6:
        v0.intValue = Scr_GetInt(0);
        Scr_AddInt(v0.intValue);
        break;
    default:
        TypeName = Scr_GetTypeName(0);
        Scr_ParamError(0, va("cannot cast %s to int", TypeName));
        break;
    }
}

void GScr_VectorFromLineToPoint()
{
    double v0; // st7
    float scale; // [esp+0h] [ebp-5Ch]
    float segmentB[3]; // [esp+Ch] [ebp-50h] BYREF
    float result[3]; // [esp+18h] [ebp-44h] BYREF
    float BA[3]; // [esp+24h] [ebp-38h] BYREF
    float PA[3]; // [esp+30h] [ebp-2Ch] BYREF
    float fraction; // [esp+3Ch] [ebp-20h]
    float segmentLengthSq; // [esp+40h] [ebp-1Ch]
    float segmentA[3]; // [esp+44h] [ebp-18h] BYREF
    float P[3]; // [esp+50h] [ebp-Ch] BYREF

    Scr_GetVector(0, segmentA);
    Scr_GetVector(1u, segmentB);
    Scr_GetVector(2u, P);
    Vec3Sub(segmentB, segmentA, BA);
    segmentLengthSq = Vec3LengthSq(BA);
    if (segmentLengthSq == 0.0)
        Scr_ParamError(0, "The two points on the line must be different from each other");
    Vec3Sub(P, segmentA, PA);
    v0 = Vec3Dot(BA, PA);
    fraction = v0 / segmentLengthSq;
    scale = -fraction;
    Vec3Mad(PA, scale, BA, result);
    Scr_AddVector(result);
}

void GScr_PointOnSegmentNearestToPoint()
{
    double v0; // st7
    float segmentB[3]; // [esp+Ch] [ebp-50h] BYREF
    float BA[3]; // [esp+18h] [ebp-44h] BYREF
    float PA[3]; // [esp+24h] [ebp-38h] BYREF
    float fraction; // [esp+30h] [ebp-2Ch]
    float segmentLengthSq; // [esp+34h] [ebp-28h]
    float segmentA[3]; // [esp+38h] [ebp-24h] BYREF
    float P[3]; // [esp+44h] [ebp-18h] BYREF
    float nearPoint[3]; // [esp+50h] [ebp-Ch] BYREF

    Scr_GetVector(0, segmentA);
    Scr_GetVector(1u, segmentB);
    Scr_GetVector(2u, P);
    Vec3Sub(segmentB, segmentA, BA);
    segmentLengthSq = Vec3LengthSq(BA);
    if (segmentLengthSq == 0.0)
        Scr_ParamError(0, "Line segment must not have zero length");
    Vec3Sub(P, segmentA, PA);
    v0 = Vec3Dot(BA, PA);
    fraction = v0 / segmentLengthSq;
    if (fraction >= 0.0)
    {
        if (fraction <= 1.0)
        {
            Vec3Mad(segmentA, fraction, BA, nearPoint);
            Scr_AddVector(nearPoint);
        }
        else
        {
            Scr_AddVector(segmentB);
        }
    }
    else
    {
        Scr_AddVector(segmentA);
    }
}

void Scr_Distance()
{
    float diff[3]; // [esp+8h] [ebp-24h] BYREF
    float v0[3]; // [esp+14h] [ebp-18h] BYREF
    float v1[3]; // [esp+20h] [ebp-Ch] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1u, v1);
    Vec3Sub(v1, v0, diff);
    Scr_AddFloat(Vec3Length(diff));
}

void Scr_Distance2D()
{
    float v0[3]; // [esp+4h] [ebp-18h] BYREF
    float v1[3]; // [esp+10h] [ebp-Ch] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1u, v1);
    Scr_AddFloat(Vec2Distance(v0, v1));
}

void Scr_DistanceSquared()
{
    float diff[3]; // [esp+4h] [ebp-24h] BYREF
    float v0[3]; // [esp+10h] [ebp-18h] BYREF
    float v1[3]; // [esp+1Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1u, v1);
    Vec3Sub(v1, v0, diff);
    Scr_AddFloat(Vec3LengthSq(diff));
}

void Scr_Length()
{
    float v[3]; // [esp+8h] [ebp-Ch] BYREF

    Scr_GetVector(0, v);
    Scr_AddFloat(Vec3Length(v));
}

void Scr_LengthSquared()
{
    float v[3]; // [esp+4h] [ebp-Ch] BYREF

    Scr_GetVector(0, v);
    Scr_AddFloat(Vec3LengthSq(v));
}

void Scr_Closer()
{
    float v[3]; // [esp+4h] [ebp-44h] BYREF
    float diff[3]; // [esp+10h] [ebp-38h] BYREF
    float fDistBSqrd; // [esp+1Ch] [ebp-2Ch]
    float vB[3]; // [esp+20h] [ebp-28h] BYREF
    float fDistASqrd; // [esp+2Ch] [ebp-1Ch]
    float vRef[3]; // [esp+30h] [ebp-18h] BYREF
    float vA[3]; // [esp+3Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, vRef);
    Scr_GetVector(1u, vA);
    Scr_GetVector(2u, vB);
    Vec3Sub(vRef, vA, diff);
    fDistASqrd = Vec3LengthSq(diff);
    Vec3Sub(vRef, vB, v);
    fDistBSqrd = Vec3LengthSq(v);
    Scr_AddInt(fDistBSqrd > (double)fDistASqrd);
}

void Scr_VectorDot()
{
    float b[3]; // [esp+4h] [ebp-18h] BYREF
    float a[3]; // [esp+10h] [ebp-Ch] BYREF

    Scr_GetVector(0, a);
    Scr_GetVector(1u, b);
    Scr_AddFloat(Vec3Dot(a, b));
}

void Scr_VectorNormalize()
{
    float b[3]; // [esp+Ch] [ebp-18h] BYREF
    float a[3]; // [esp+18h] [ebp-Ch] BYREF

    if (Scr_GetNumParam() != 1)
        Scr_Error("wrong number of arguments to vectornormalize!");
    Scr_GetVector(0, a);
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    Vec3Normalize(b);
    Scr_AddVector(b);
}

void Scr_VectorToAngles()
{
    float angles[3]; // [esp+0h] [ebp-18h] BYREF
    float vec[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (Scr_GetNumParam() != 1)
        Scr_Error("wrong number of arguments to vectortoangle!");
    Scr_GetVector(0, vec);
    vectoangles(vec, angles);
    Scr_AddVector(angles);
}

void Scr_VectorLerp()
{
    float from[3]; // [esp+8h] [ebp-28h] BYREF
    float result[3]; // [esp+14h] [ebp-1Ch] BYREF
    float fraction; // [esp+20h] [ebp-10h]
    float to[3]; // [esp+24h] [ebp-Ch] BYREF

    if (Scr_GetNumParam() != 3)
        Scr_Error("wrong number of arguments to vectorlerp");
    Scr_GetVector(0, from);
    Scr_GetVector(1u, to);
    fraction = Scr_GetFloat(2);
    Vec3Lerp(from, to, fraction, result);
    Scr_AddVector(result);
}

void Scr_AnglesToUp()
{
    float angles[3]; // [esp+0h] [ebp-18h] BYREF
    float up[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, angles);
    AngleVectors(angles, 0, 0, up);
    Scr_AddVector(up);
}

void Scr_AnglesToRight()
{
    float right[3]; // [esp+0h] [ebp-18h] BYREF
    float angles[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, angles);
    AngleVectors(angles, 0, right, 0);
    Scr_AddVector(right);
}

void Scr_AnglesToForward()
{
    float forward[3]; // [esp+0h] [ebp-18h] BYREF
    float angles[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, angles);
    AngleVectors(angles, forward, 0, 0);
    Scr_AddVector(forward);
}

void Scr_CombineAngles()
{
    float anglesfinal[3]; // [esp+0h] [ebp-90h] BYREF
    float axisB[3][3]; // [esp+Ch] [ebp-84h] BYREF
    float anglesA[3]; // [esp+30h] [ebp-60h] BYREF
    float axisA[3][3]; // [esp+3Ch] [ebp-54h] BYREF
    float anglesB[3]; // [esp+60h] [ebp-30h] BYREF
    float combinedaxis[3][3]; // [esp+6Ch] [ebp-24h] BYREF

    Scr_GetVector(0, anglesA);
    Scr_GetVector(1u, anglesB);
    AnglesToAxis(anglesA, axisA);
    AnglesToAxis(anglesB, axisB);
    MatrixMultiply(axisB, axisA, combinedaxis);
    AxisToAngles(combinedaxis, anglesfinal);
    Scr_AddVector(anglesfinal);
}

void Scr_IsSubStr()
{
    uint8_t *v0; // eax
    char* v1; // eax
    uint8_t *String; // [esp-4h] [ebp-4h]

    String = (uint8_t *)Scr_GetString(1);
    v0 = (uint8_t *)Scr_GetString(0);
    v1 = strstr((char*)v0, (char *)String);
    Scr_AddBool(v1 != 0);
}

void Scr_GetSubStr()
{
    VariableUnion v0; // [esp+0h] [ebp-424h]
    int32_t source; // [esp+4h] [ebp-420h]
    char c; // [esp+Bh] [ebp-419h]
    char tempString[1028]; // [esp+Ch] [ebp-418h] BYREF
    int32_t start; // [esp+414h] [ebp-10h]
    int32_t end; // [esp+418h] [ebp-Ch]
    int32_t dest; // [esp+41Ch] [ebp-8h]
    const char *s; // [esp+420h] [ebp-4h]

    s = Scr_GetString(0);
    start = Scr_GetInt(1);
    if (Scr_GetNumParam() < 3)
        v0.intValue = 0x7FFFFFFF;
    else
        v0.intValue = Scr_GetInt(2);
    end = v0.intValue;
    source = start;
    for (dest = 0; source < end; ++dest)
    {
        if (dest >= 1024)
            Scr_Error("string too long");
        c = s[source];
        if (!c)
            break;
        tempString[dest] = c;
        ++source;
    }
    tempString[dest] = 0;
    Scr_AddString(tempString);
}

void Scr_ToLower()
{
    char v0; // al
    char tempString[1028]; // [esp+4h] [ebp-410h] BYREF
    const char *s; // [esp+40Ch] [ebp-8h]
    int32_t i; // [esp+410h] [ebp-4h]

    s = Scr_GetString(0);
    i = 0;
    while (i < 1024)
    {
        v0 = tolower(*s);
        tempString[i] = v0;
        if (!v0)
        {
            Scr_AddString(tempString);
            return;
        }
        ++i;
        ++s;
    }
    Scr_Error("string too long");
}

void Scr_StrTok()
{
    int32_t source; // [esp+10h] [ebp-42Ch]
    char c; // [esp+17h] [ebp-425h]
    uint32_t  delimId; // [esp+18h] [ebp-424h]
    char tempString[1028]; // [esp+1Ch] [ebp-420h] BYREF
    const char *delim; // [esp+424h] [ebp-18h]
    int32_t dest; // [esp+428h] [ebp-14h]
    const char *s; // [esp+42Ch] [ebp-10h]
    int32_t i; // [esp+430h] [ebp-Ch]
    int32_t delimLen; // [esp+434h] [ebp-8h]
    uint32_t  sId; // [esp+438h] [ebp-4h]

    sId = Scr_GetConstString(0);
    delimId = Scr_GetConstString(1);
    s = SL_ConvertToString(sId);
    delim = SL_ConvertToString(delimId);
    SL_AddRefToString(sId);
    SL_AddRefToString(delimId);
    delimLen = strlen(delim);
    dest = 0;
    Scr_MakeArray();
    for (source = 0; ; ++source)
    {
        c = s[source];
        if (!c)
            break;
        for (i = 0; i < delimLen; ++i)
        {
            if (c == delim[i])
            {
                if (dest)
                {
                    tempString[dest] = 0;
                    Scr_AddString(tempString);
                    Scr_AddArray();
                    dest = 0;
                }
                goto LABEL_2;
            }
        }
        tempString[dest++] = c;
        if (dest >= 1024)
        {
            SL_RemoveRefToString(sId);
            SL_RemoveRefToString(delimId);
            Scr_Error("string too long");
        }
    LABEL_2:
        ;
    }
    if (dest)
    {
        tempString[dest] = 0;
        Scr_AddString(tempString);
        Scr_AddArray();
        dest = 0;
    }
    SL_RemoveRefToString(sId);
    SL_RemoveRefToString(delimId);
}

void Scr_MusicPlay()
{

    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %s 1", 111, Scr_GetString(0)));
}

void Scr_MusicStop()
{
    uint32_t  NumParam; // [esp+0h] [ebp-18h]
    int32_t fadeTime; // [esp+14h] [ebp-4h]

    NumParam = Scr_GetNumParam();
    if (NumParam)
    {
        if (NumParam != 1)
        {
            Scr_Error(va("USAGE: musicStop([fadetime]);\n"));
            return;
        }
        fadeTime = (int)(Scr_GetFloat(0) * 1000.0f);
    }
    else
    {
        fadeTime = 0;
    }
    if (fadeTime < 0)
    {
        Scr_Error(va("musicStop: fade time must be >= 0\n"));
    }
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %i", 112, fadeTime));
}

void Scr_SoundFade()
{
    float fTargetVol; // [esp+Ch] [ebp-8h]
    int32_t iFadeTime; // [esp+10h] [ebp-4h]

    fTargetVol = Scr_GetFloat(0);
    if (Scr_GetNumParam() <= 1)
        iFadeTime = 0;
    else
        iFadeTime = (int)(Scr_GetFloat(1) * 1000.0);

    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %f %i\n", 113, fTargetVol, iFadeTime));
}

void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, const char *assetName)
{
    const char *XAssetTypeName; // eax
    const char *v3; // eax

    DB_FindXAssetHeader(type, assetName);
    if (DB_IsXAssetDefault(type, assetName))
    {
        XAssetTypeName = DB_GetXAssetTypeName(type);
        v3 = va("precache %s '%s' failed", XAssetTypeName, assetName);
        Scr_NeverTerminalError(v3);
    }
}

int32_t Scr_PrecacheModel()
{
    const char *modelName; // [esp+4h] [ebp-4h]

    if (!level.initializing)
        Scr_Error("precacheModel must be called before any wait statements in the gametype or level script\n");
    modelName = Scr_GetString(0);
    if (!*modelName)
        Scr_ParamError(0, "Model name string is empty");
    if (IsFastFileLoad())
        Scr_ErrorOnDefaultAsset(ASSET_TYPE_XMODEL, modelName);
    return G_ModelIndex((char*)modelName);
}

void Scr_PrecacheShellShock()
{
    shellshock_parms_t *ShellshockParms; // eax
    const char *shellshockName; // [esp+0h] [ebp-8h]
    uint32_t  index; // [esp+4h] [ebp-4h]

    if (!level.initializing)
        Scr_Error("precacheShellShock must be called before any wait statements in the gametype or level script\n");
    shellshockName = Scr_GetString(0);
    index = G_ShellShockIndex((char*)shellshockName);
    if (!BG_LoadShellShockDvars(shellshockName))
        Com_Error(ERR_DROP, "Couldn't find shell shock %s -- see console", shellshockName);
    ShellshockParms = BG_GetShellshockParms(index);
    BG_SetShellShockParmsFromDvars(ShellshockParms);
}

void Scr_PrecacheItem()
{
    const char *pszItemName; // [esp+4h] [ebp-4h]

    if (!level.initializing)
        Scr_Error("precacheItem must be called before any wait statements in the gametype or level script\n");

    pszItemName = Scr_GetString(0);

    if (!G_FindItem(pszItemName, 0))
    {
        Scr_ParamError(0, va("unknown item '%s'", pszItemName));
    }
}

int32_t Scr_PrecacheShader()
{
    const char *shaderName; // [esp+0h] [ebp-4h]

    if (!level.initializing)
        Scr_Error("precacheShader must be called before any wait statements in the gametype or level script\n");

    shaderName = Scr_GetString(0);

    if (!*shaderName)
    {
#ifdef KISAK_NO_FASTFILES
        return 0;
#else
        Scr_ParamError(0, "Shader name string is empty");
#endif
    }

    return G_MaterialIndex(shaderName);
}

char *Scr_PrecacheString()
{
    const char *result; // eax

    if (!level.initializing)
        Scr_Error("precacheString must be called before any wait statements in the gametype or level script\n");
    result = Scr_GetIString(0);
    if (*result)
        return (char *)G_LocalizedStringIndex((char*)result);
    return (char*)result;
}

void Scr_AmbientPlay()
{
    uint32_t  NumParam; // [esp+0h] [ebp-1Ch]
    int32_t iFadeTime; // [esp+14h] [ebp-8h]
    const char *pszAliasName; // [esp+18h] [ebp-4h]

    iFadeTime = 0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error(va("USAGE: ambientPlay(alias_name, <fadetime>);\n"));
            return;
        }
        iFadeTime = (int)(Scr_GetFloat(1) * 1000.0f);
    }
    pszAliasName = Scr_GetString(0);
    if (!*pszAliasName)
    {
        Scr_Error(va("ambientPlay: alias name cannot be the empty string... use stop or fade version\n"));
    }
    if (iFadeTime < 0)
    {
        Scr_Error(va("ambientPlay: fade time must be >= 0\n"));
    }
    SV_SetConfigstring(821, va("n\\%s\\t\\%i", pszAliasName, iFadeTime + level.time));
}

void Scr_AmbientStop()
{
    const char *v0; // eax
    const char *v1; // eax
    char *v2; // eax
    uint32_t  NumParam; // [esp+0h] [ebp-18h]
    int32_t iFadeTime; // [esp+14h] [ebp-4h]

    NumParam = Scr_GetNumParam();
    if (NumParam)
    {
        if (NumParam != 1)
        {
            v0 = va("USAGE: ambientStop(<fadetime>);\n");
            Scr_Error(v0);
            return;
        }
        iFadeTime = (int)(Scr_GetFloat(0) * 1000.0f);
    }
    else
    {
        iFadeTime = 0;
    }
    if (iFadeTime < 0)
    {
        v1 = va("ambientStop: fade time must be >= 0\n");
        Scr_Error(v1);
    }
    v2 = va("t\\%i", iFadeTime + level.time);
    SV_SetConfigstring(821, v2);
}

void Scr_GrenadeExplosionEffect()
{
    uint8_t v0; // al
    int32_t result; // eax
    float vDir[3]; // [esp+0h] [ebp-60h] BYREF
    float vOrg[3]; // [esp+Ch] [ebp-54h] BYREF
    float vEnd[3]; // [esp+18h] [ebp-48h] BYREF
    trace_t trace; // [esp+24h] [ebp-3Ch] BYREF
    gentity_s *pEnt; // [esp+50h] [ebp-10h]
    float vPos[3]; // [esp+54h] [ebp-Ch] BYREF

    Scr_GetVector(0, vOrg);
    vPos[0] = vOrg[0];
    vPos[1] = vOrg[1];
    vPos[2] = vOrg[2] + 1.0;
    pEnt = G_TempEntity(vPos, 45);
    vDir[0] = 0.0;
    vDir[1] = 0.0;
    vDir[2] = 1.0;
    v0 = DirToByte(vDir);
    pEnt->s.eventParm = v0;
    vEnd[0] = vPos[0];
    vEnd[1] = vPos[1];
    vEnd[2] = vPos[2] - 17.0;
    G_TraceCapsule(&trace, vPos, (float *)vec3_origin, (float *)vec3_origin, vEnd, ENTITYNUM_NONE, 2065);
    result = (trace.surfaceFlags & 0x1F00000) >> 20;
    pEnt->s.surfType = result;
}

void GScr_RadiusDamage()
{
    GScr_RadiusDamageInternal(0);
}

void __cdecl GScr_RadiusDamageInternal(gentity_s *inflictor)
{
    gentity_s *attacker; // [esp+20h] [ebp-24h]
    meansOfDeath_t mod; // [esp+24h] [ebp-20h]
    float max_damage; // [esp+28h] [ebp-1Ch]
    float origin[3]; // [esp+2Ch] [ebp-18h] BYREF
    float range; // [esp+38h] [ebp-Ch]
    int32_t weapon; // [esp+3Ch] [ebp-8h]
    float min_damage; // [esp+40h] [ebp-4h]

    Scr_GetVector(0, origin);
    range = Scr_GetFloat(1);
    max_damage = Scr_GetFloat(2);
    min_damage = Scr_GetFloat(3);
    attacker = &g_entities[ENTITYNUM_WORLD];
    if (Scr_GetNumParam() > 4 && Scr_GetType(4))
        attacker = Scr_GetEntity(4);
    mod = MOD_EXPLOSIVE;
    if (Scr_GetNumParam() > 5 && Scr_GetType(5))
        mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(5);
    weapon = -1;
    if (Scr_GetNumParam() > 6 && Scr_GetType(6))
    {
        weapon = G_GetWeaponIndexForName(Scr_GetString(6));
    }
    level.bPlayerIgnoreRadiusDamage = level.bPlayerIgnoreRadiusDamageLatched;
    G_RadiusDamage(origin, inflictor, attacker, max_damage, min_damage, range, 1.0, 0, inflictor, mod, weapon);
    level.bPlayerIgnoreRadiusDamage = 0;
}

void __cdecl GScr_EntityRadiusDamage(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    GScr_RadiusDamageInternal(ent);
}

void __cdecl GScr_Detonate(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    gentity_s *player; // [esp+8h] [ebp-4h]

    ent = GetEntity(entref);
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (ent->s.eType != ET_MISSILE || !weapDef || weapDef->weapType != WEAPTYPE_GRENADE)
        Scr_ObjectError("entity is not a grenade");
    if (Scr_GetNumParam())
    {
        if (Scr_GetType(0))
        {
            player = Scr_GetEntity(0);
            if (!player->client)
                Scr_ParamError(0, "Entity is not a player");
            ent->parent.setEnt(player);
        }
        else
        {
            ent->parent.setEnt(&g_entities[ENTITYNUM_WORLD]);
        }
    }
    G_ExplodeMissile(ent);
}

VariableUnion GScr_SetPlayerIgnoreRadiusDamage()
{
    VariableUnion result; // eax

    result.intValue = Scr_GetInt(0);
    level.bPlayerIgnoreRadiusDamageLatched = result.intValue;
    return result;
}

void __cdecl GScr_DamageConeTrace(scr_entref_t entref)
{
    GScr_DamageConeTraceInternal(entref, 0x802011);
}

void __cdecl GScr_DamageConeTraceInternal(scr_entref_t entref, int32_t contentMask)
{
    float damageOrigin[3]; // [esp+Ch] [ebp-18h] BYREF
    gentity_s *ignoreEnt; // [esp+18h] [ebp-Ch]
    float damageAmount; // [esp+1Ch] [ebp-8h]
    gentity_s *ent; // [esp+20h] [ebp-4h]

    ent = GetEntity(entref);
    ignoreEnt = 0;
    if (Scr_GetNumParam())
        ignoreEnt = Scr_GetEntity(1);
    Scr_GetVector(0, damageOrigin);
    damageAmount = CanDamage(ent, ignoreEnt, damageOrigin, 1.0, 0, contentMask);
    Scr_AddFloat(damageAmount);
}

void __cdecl GScr_SightConeTrace(scr_entref_t entref)
{
    GScr_DamageConeTraceInternal(entref, 2049);
}

void GScr_GetMoveDelta()
{
    const XAnim_s *Anims; // eax
    uint32_t  index; // [esp-Ch] [ebp-3Ch]
    float time1; // [esp+0h] [ebp-30h]
    float time2; // [esp+4h] [ebp-2Ch]
    uint32_t  NumParam; // [esp+8h] [ebp-28h]
    float trans[3]; // [esp+10h] [ebp-20h] BYREF
    float endTime; // [esp+1Ch] [ebp-14h]
    float startTime; // [esp+20h] [ebp-10h]
    float rot[2]; // [esp+24h] [ebp-Ch] BYREF
    scr_anim_s anim; // [esp+2Ch] [ebp-4h]

    startTime = 0.0;
    endTime = 1.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            endTime = Scr_GetFloat(2);
            if (endTime < 0.0 || endTime > 1.0)
                Scr_ParamError(2u, "end time must be between 0 and 1");
        }
        startTime = Scr_GetFloat(1);
        if (startTime < 0.0 || startTime > 1.0)
            Scr_ParamError(1u, "start time must be between 0 and 1");
    }
    anim = Scr_GetAnim(0, 0);
    time2 = endTime;
    time1 = startTime;
    index = anim.index;
    Anims = Scr_GetAnims(anim.tree);
    XAnimGetRelDelta(Anims, index, rot, trans, time1, time2);
    Scr_AddVector(trans);
}

void GScr_GetAngleDelta()
{
    const XAnim_s *Anims; // eax
    uint32_t  index; // [esp-Ch] [ebp-3Ch]
    float time1; // [esp+0h] [ebp-30h]
    float time2; // [esp+4h] [ebp-2Ch]
    VariableUnion time2a; // [esp+4h] [ebp-2Ch]
    uint32_t  NumParam; // [esp+8h] [ebp-28h]
    float trans[3]; // [esp+10h] [ebp-20h] BYREF
    float endTime; // [esp+1Ch] [ebp-14h]
    float startTime; // [esp+20h] [ebp-10h]
    float rot[2]; // [esp+24h] [ebp-Ch] BYREF
    scr_anim_s anim; // [esp+2Ch] [ebp-4h]

    startTime = 0.0;
    endTime = 1.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            endTime = Scr_GetFloat(2);
            if (endTime < 0.0 || endTime > 1.0)
                Scr_ParamError(2u, "end time must be between 0 and 1");
        }
        startTime = Scr_GetFloat(1);
        if (startTime < 0.0 || startTime > 1.0)
            Scr_ParamError(1u, "start time must be between 0 and 1");
    }
    anim = Scr_GetAnim(0, 0);
    time2 = endTime;
    time1 = startTime;
    index = anim.index;
    Anims = Scr_GetAnims(anim.tree);
    XAnimGetRelDelta(Anims, index, rot, trans, time1, time2);
    Scr_AddFloat(RotationToYaw(rot));
}

void GScr_GetNorthYaw()
{
    char northYawString[32]; // [esp+8h] [ebp-24h] BYREF

    SV_GetConfigstring(0x336u, northYawString, 32);
    Scr_AddFloat(atof(northYawString));
}

void Scr_LoadFX()
{
    const char *filename; // [esp+0h] [ebp-8h]
    int32_t id; // [esp+4h] [ebp-4h]

    filename = Scr_GetString(0);
    if (!I_strncmp(filename, "fx/", 3))
        Scr_ParamError(0, "effect name should start after the 'fx' folder.");
    id = G_EffectIndex((char*)filename);
    if (!id && !level.initializing)
        Scr_Error("loadFx must be called before any wait statements in the level script, or on an already loaded effect\n");
    Scr_AddInt(id);
}

void Scr_PlayFX()
{
    float pos[3]; // [esp+18h] [ebp-40h] BYREF
    int32_t numParams; // [esp+24h] [ebp-34h]
    int32_t fxId; // [esp+28h] [ebp-30h]
    gentity_s *ent; // [esp+2Ch] [ebp-2Ch]
    float axis[3][3]; // [esp+30h] [ebp-28h] BYREF
    float vecLength; // [esp+54h] [ebp-4h]

    numParams = Scr_GetNumParam();
    if (numParams < 2 || numParams > 4)
        Scr_Error("Incorrect number of parameters");
    fxId = Scr_GetInt(0);
    Scr_GetVector(1u, pos);
    ent = G_TempEntity(pos, EV_PLAY_FX);
    iassert(ent->s.lerp.apos.trType == TR_STATIONARY);
    ent->s.eventParm = (uint8_t)fxId;
    if (numParams == 2)
    {
        Scr_SetFxAngles(0, axis, ent->s.lerp.apos.trBase);
    }
    else
    {
        if (numParams != 3 && numParams != 4)
            MyAssertHandler(
                ".\\game_mp\\g_scr_main_mp.cpp",
                4365,
                1,
                "%s\n\t(numParams) = %i",
                "(numParams == 3 || numParams == 4)",
                numParams);
        Scr_GetVector(2u, axis[0]);
        vecLength = Vec3Normalize(axis[0]);
        if (vecLength == 0.0)
            Scr_FxParamError(2u, "playFx called with (0 0 0) forward direction", fxId);
        if (numParams == 3)
        {
            Scr_SetFxAngles(1u, axis, ent->s.lerp.apos.trBase);
        }
        else
        {
            if (numParams != 4)
                MyAssertHandler(
                    ".\\game_mp\\g_scr_main_mp.cpp",
                    4378,
                    1,
                    "%s\n\t(numParams) = %i",
                    "(numParams == 4)",
                    numParams);
            Scr_GetVector(3u, axis[2]);
            vecLength = Vec3Normalize(axis[2]);
            if (vecLength == 0.0)
                Scr_FxParamError(3u, "playFx called with (0 0 0) up direction", fxId);
            Scr_SetFxAngles(2u, axis, ent->s.lerp.apos.trBase);
        }
    }
}

void __cdecl Scr_SetFxAngles(uint32_t  givenAxisCount, float (*axis)[3], float *angles)
{
    const char *v3; // eax
    float scale; // [esp+0h] [ebp-20h]
    float dot; // [esp+1Ch] [ebp-4h]

    if (givenAxisCount > 2)
        MyAssertHandler(
            ".\\game_mp\\g_scr_main_mp.cpp",
            4284,
            0,
            "givenAxisCount not in [0, 2]\n\t%i not in [%i, %i]",
            givenAxisCount,
            0,
            2);
    if (givenAxisCount == 1)
    {
        vectoangles((const float *)axis, angles);
    }
    else if (givenAxisCount == 2)
    {
        dot = Vec3Dot((const float *)axis, &(*axis)[6]);
        scale = -dot;
        Vec3Mad(&(*axis)[6], scale, (const float *)axis, &(*axis)[6]);
        if (Vec3Normalize(&(*axis)[6]) == 0.0)
        {
            v3 = va("forward and up vectors are the same direction or exact opposite directions");
            Scr_Error(v3);
        }
        Vec3Cross(&(*axis)[6], (const float *)axis, &(*axis)[3]);
        AxisToAngles(*(const mat3x3*)axis, angles);
    }
    else
    {
        *angles = 270.0;
        angles[1] = 0.0;
        angles[2] = 0.0;
    }
}

void __cdecl Scr_FxParamError(uint32_t  paramIndex, const char *errorString, int32_t fxId)
{
    const char *v3; // eax
    char fxName[1028]; // [esp+0h] [ebp-408h] BYREF

    if (!errorString)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 4328, 0, "%s", "errorString");
    if (fxId)
        SV_GetConfigstring(fxId + 1598, fxName, 1024);
    else
        strcpy(fxName, "not successfully loaded");
    v3 = va("%s (effect = %s)\n", errorString, fxName);
    Scr_ParamError(paramIndex, v3);
}

void Scr_PlayFXOnTag()
{
    int32_t fxId; // [esp+0h] [ebp-10h]
    gentity_s *ent; // [esp+4h] [ebp-Ch]
    uint32_t  tag; // [esp+8h] [ebp-8h]
    int32_t csIndex; // [esp+Ch] [ebp-4h]

    if (Scr_GetNumParam() != 3)
        Scr_Error("Incorrect number of parameters");
    fxId = Scr_GetInt(0);
    if (fxId <= 0 || fxId >= 100)
    {
        Scr_ParamError(0, va("effect id %i is invalid\n", fxId));
    }
    ent = Scr_GetEntity(1);
    if (!ent->model)
        Scr_ParamError(1u, "cannot play fx on entity with no model");
    tag = Scr_GetConstLowercaseString(2);
    if (strchr(SL_ConvertToString(tag), '"'))
        Scr_ParamError(2u, "cannot use \" characters in tag names\n");
    if (SV_DObjGetBoneIndex(ent, tag) < 0)
    {
        SV_DObjDumpInfo(ent);
        Scr_ParamError(2u, va("tag '%s' does not exist on entity with model '%s'", SL_ConvertToString(tag), SL_ConvertToString(G_ModelName(ent->model))));
    }
    csIndex = G_FindConfigstringIndex(va("%02d%s", fxId, SL_ConvertToString(tag)), 1698, 256, 1, 0);
    if (csIndex <= 0 || csIndex >= 256)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 4421, 0, "%s", "csIndex > 0 && csIndex < MAX_EFFECT_TAGS");
    G_AddEvent(ent, 0x38u, csIndex);
}

void Scr_PlayLoopedFX()
{
    uint32_t  NumParam; // [esp+0h] [ebp-70h]
    float v1; // [esp+4h] [ebp-6Ch]
    float pos[3]; // [esp+2Ch] [ebp-44h] BYREF
    int32_t fxId; // [esp+38h] [ebp-38h]
    int32_t repeat; // [esp+3Ch] [ebp-34h]
    gentity_s *ent; // [esp+40h] [ebp-30h]
    int32_t givenAxisCount; // [esp+44h] [ebp-2Ch]
    float axis[3][3]; // [esp+48h] [ebp-28h] BYREF
    float cullDist; // [esp+6Ch] [ebp-4h]

    if (Scr_GetNumParam() < 3 || Scr_GetNumParam() > 6)
        Scr_Error("Incorrect number of parameters");
    givenAxisCount = 0;
    cullDist = 0.0;
    fxId = Scr_GetInt(0);
    NumParam = Scr_GetNumParam();
    if (NumParam != 4)
    {
        if (NumParam != 5)
        {
            if (NumParam != 6)
                goto LABEL_13;
            ++givenAxisCount;
            Scr_GetVector(5u, axis[2]);
            if (Vec3Normalize(axis[2]) == 0.0)
                Scr_FxParamError(5u, "playLoopedFx called with (0 0 0) up direction", fxId);
        }
        Scr_GetVector(4u, axis[0]);
        if (Vec3Normalize(axis[0]) == 0.0)
            Scr_FxParamError(4u, "playLoopedFx called with (0 0 0) forward direction", fxId);
        ++givenAxisCount;
    }
    cullDist = Scr_GetFloat(3);
LABEL_13:
    Scr_GetVector(2u, pos);
    repeat = (int)(Scr_GetFloat(1) * 1000.0f);
    if (repeat <= 0)
        Scr_FxParamError(1u, "playLoopedFx called with repeat < 0.001 seconds", fxId);
    ent = G_Spawn();
    ent->s.eType = ET_LOOP_FX;
    ent->r.svFlags |= 8u;
    ent->s.un1.scale = (uint8_t)fxId;
    if (ent->s.un1.scale != fxId)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 4471, 0, "%s", "ent->s.un1.eventParm2 == fxId");
    G_SetOrigin(ent, pos);
    Scr_SetFxAngles(givenAxisCount, axis, ent->s.lerp.apos.trBase);
    ent->s.lerp.u.loopFx.cullDist = cullDist;
    ent->s.lerp.u.loopFx.period = repeat;
    SV_LinkEntity(ent);
    Scr_AddEntity(ent);
}

void Scr_SpawnFX()
{
    uint32_t  NumParam; // [esp+0h] [ebp-58h]
    float pos[3]; // [esp+1Ch] [ebp-3Ch] BYREF
    int32_t fxId; // [esp+28h] [ebp-30h]
    gentity_s *ent; // [esp+2Ch] [ebp-2Ch]
    int32_t givenAxisCount; // [esp+30h] [ebp-28h]
    float axis[3][3]; // [esp+34h] [ebp-24h] BYREF

    if (Scr_GetNumParam() < 2 || Scr_GetNumParam() > 4)
        Scr_Error("Incorrect number of parameters");
    givenAxisCount = 0;
    fxId = Scr_GetInt(0);
    NumParam = Scr_GetNumParam();
    if (NumParam != 3)
    {
        if (NumParam != 4)
            goto LABEL_12;
        Scr_GetVector(3u, axis[2]);
        if (Vec3Normalize(axis[2]) == 0.0)
            Scr_FxParamError(3u, "spawnFx called with (0 0 0) up direction", fxId);
        ++givenAxisCount;
    }
    Scr_GetVector(2u, axis[0]);
    if (Vec3Normalize(axis[0]) == 0.0)
        Scr_FxParamError(2u, "spawnFx called with (0 0 0) forward direction", fxId);
    ++givenAxisCount;
LABEL_12:
    Scr_GetVector(1u, pos);
    ent = G_Spawn();
    ent->s.eType = ET_FX;
    ent->r.svFlags |= 8u;
    ent->s.un1.scale = (uint8_t)fxId;
    if (ent->s.un1.scale != fxId)
        MyAssertHandler(
            ".\\game_mp\\g_scr_main_mp.cpp",
            4518,
            0,
            "ent->s.un1.eventParm2 == fxId\n\t%i, %i",
            ent->s.un1.scale,
            fxId);
    G_SetOrigin(ent, pos);
    Scr_SetFxAngles(givenAxisCount, axis, ent->s.lerp.apos.trBase);
    if (ent->s.time2)
        MyAssertHandler(
            ".\\game_mp\\g_scr_main_mp.cpp",
            4521,
            1,
            "%s\n\t(ent->s.time2) = %i",
            "(ent->s.time2 == 0)",
            ent->s.time2);
    SV_LinkEntity(ent);
    Scr_AddEntity(ent);
}

void Scr_TriggerFX()
{
    int32_t result; // eax
    float v1; // [esp+4h] [ebp-14h]
    gentity_s *ent; // [esp+14h] [ebp-4h]

    if (!Scr_GetNumParam() || Scr_GetNumParam() > 2)
        Scr_Error("Incorrect number of parameters");
    ent = Scr_GetEntity(0);
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 4537, 0, "%s", "ent");
    if (ent->s.eType != ET_FX)
        Scr_ParamError(0, "entity wasn't created with 'newFx'");
    result = Scr_GetNumParam();
    if (result == 2)
    {
        result = (int)(Scr_GetFloat(1) * 1000.0f);
        ent->s.time2 = result;
    }
    else
    {
        ent->s.time2 = level.time;
    }
}

void Scr_PhysicsExplosionSphere()
{
    double Float; // st7
    gentity_s *result; // eax
    float pos[3]; // [esp+8h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+14h] [ebp-4h]

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, pos);
    ent = G_TempEntity(pos, EV_PHYS_EXPLOSION_SPHERE);
    ent->s.eventParm = Scr_GetInt(1);
    ent->s.lerp.u.explosion.innerRadius = Scr_GetFloat(2);
    if (ent->s.lerp.u.explosion.innerRadius < 0.0)
        Scr_ParamError(2u, "Radius is negative");
    if (ent->s.lerp.u.explosion.innerRadius > (double)ent->s.eventParm)
        Scr_Error("Inner radius is outside the outer radius");
    Float = Scr_GetFloat(3);
    result = ent;
    ent->s.lerp.u.explosion.magnitude = Float;
}

void Scr_PhysicsRadiusJolt()
{
    float pos[3]; // [esp+10h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+1Ch] [ebp-4h]

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, pos);
    ent = G_TempEntity(pos, EV_PHYS_EXPLOSION_JOLT);
    ent->s.eventParm = Scr_GetInt(1);
    ent->s.lerp.u.explosionJolt.innerRadius = Scr_GetFloat(2);
    if (ent->s.lerp.u.explosionJolt.innerRadius < 0.0f)
        Scr_ParamError(2u, "Radius is negative");
    if (ent->s.lerp.u.explosionJolt.innerRadius > (float)ent->s.eventParm)
        Scr_Error("Inner radius is outside the outer radius");
    Scr_GetVector(3u, &ent->s.lerp.u.explosionJolt.impulse[0]);
    if (ent->s.lerp.u.explosionJolt.impulse[0] == 0.0f
        && ent->s.lerp.u.explosionJolt.impulse[1] == 0.0f
        && ent->s.lerp.u.explosionJolt.impulse[2] == 0.0f)
    {
        ent->s.lerp.u.explosionJolt.impulse[0] = 1.1754944e-38f;
    }
}

void Scr_PhysicsRadiusJitter()
{
    float pos[3]; // [esp+8h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+14h] [ebp-4h]

    if (Scr_GetNumParam() != 5)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, pos);
    ent = G_TempEntity(pos, EV_PHYS_JITTER);
    ent->s.eventParm = Scr_GetInt(1);
    ent->s.lerp.u.physicsJitter.innerRadius = Scr_GetFloat(2);
    if (ent->s.lerp.u.physicsJitter.innerRadius < 0.0f)
        Scr_ParamError(2u, "Radius is negative");
    if (ent->s.lerp.u.physicsJitter.innerRadius > (float)ent->s.eventParm)
        Scr_Error("Inner radius is outside the outer radius");
    ent->s.lerp.u.physicsJitter.minDisplacement = Scr_GetFloat(3);
    ent->s.lerp.u.physicsJitter.maxDisplacement = Scr_GetFloat(4);
    if (ent->s.lerp.u.physicsJitter.maxDisplacement < ent->s.lerp.u.physicsJitter.minDisplacement)
        Scr_Error("Maximum jitter is less than minimum jitter");
}

void Scr_PhysicsExplosionCylinder()
{
    double Float; // st7
    gentity_s *result; // eax
    float pos[3]; // [esp+8h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+14h] [ebp-4h]

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, pos);
    ent = G_TempEntity(pos, EV_PHYS_EXPLOSION_CYLINDER);
    ent->s.eventParm = Scr_GetInt(1);
    ent->s.lerp.u.explosion.innerRadius = Scr_GetFloat(2);
    if (ent->s.lerp.u.explosion.innerRadius < 0.0f)
        Scr_ParamError(2u, "Radius is negative");
    if (ent->s.lerp.u.explosion.innerRadius > (float)ent->s.eventParm)
        Scr_Error("Inner radius is outside the outer radius");
    Float = Scr_GetFloat(3);
    result = ent;
    ent->s.lerp.u.explosion.magnitude = Float;
}

void Scr_SetExponentialFog()
{
    float green; // [esp+18h] [ebp-20h]
    float startDist; // [esp+1Ch] [ebp-1Ch]
    float blue; // [esp+20h] [ebp-18h]
    float red; // [esp+24h] [ebp-14h]
    float density; // [esp+28h] [ebp-10h]
    float time; // [esp+2Ch] [ebp-Ch]
    float halfwayDist; // [esp+30h] [ebp-8h]

    if (Scr_GetNumParam() != 6)
        Scr_Error(
            "Incorrect number of parameters\n"
            "USAGE: setExpFog(<startDist>, <halfwayDist>, <red>, <green>, <blue>, <transition time>)\n");
    startDist = Scr_GetFloat(0);
    if (startDist < 0.0)
        Scr_Error("setExpFog: startDist must be greater or equal to 0");
    halfwayDist = Scr_GetFloat(1);
    if (halfwayDist <= 0.0)
        Scr_Error("setExpFog: halfwayDist must be greater than 0");
    density = 0.69314718f / halfwayDist;
    red = Scr_GetFloat(2);
    green = Scr_GetFloat(3);
    blue = Scr_GetFloat(4);
    time = Scr_GetFloat(5);
    Dvar_SetColor((dvar_s *)g_fogColorReadOnly, red, green, blue, 1.0f);
    Dvar_SetFloat((dvar_s *)g_fogStartDistReadOnly, startDist);
    Dvar_SetFloat((dvar_s *)g_fogHalfDistReadOnly, halfwayDist);
    if (density <= 0.0f || density >= 1.0f)
        MyAssertHandler(
            ".\\game_mp\\g_scr_main_mp.cpp",
            4704,
            0,
            "%s\n\t(density) = %g",
            "(density > 0 && density < 1)",
            density);
    Scr_SetFog("setExpFog", startDist, density, red, green, blue, time);
}

void __cdecl Scr_SetFog(const char *cmd, float start, float density, float r, float g, float b, float time)
{
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    char *v10; // eax

    if (start < 0.0f)
    {
        v7 = va("%s: near distance must be >= 0", cmd);
        Scr_Error(v7);
    }
    if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f)
    {
        v8 = va("%s: red/green/blue color components must be in the range [0, 1]", cmd);
        Scr_Error(v8);
    }
    if (time < 0.0f)
    {
        v9 = va("%s: transition time must be >= 0 seconds", cmd);
        Scr_Error(v9);
    }
    v10 = va("%g %g %g %g %g %.0f", start, density, r, g, b, time * 1000.0f);
    G_setfog(v10);
}

void Scr_VisionSetNaked()
{
    uint32_t  NumParam; // [esp+0h] [ebp-1Ch]
    int32_t duration; // [esp+14h] [ebp-8h]
    const char *name; // [esp+18h] [ebp-4h]

    duration = 1000;
    NumParam = Scr_GetNumParam();
    if (NumParam == 1)
        goto LABEL_4;
    if (NumParam == 2)
    {
        duration = (int)(Scr_GetFloat(1) * 1000.0f);
    LABEL_4:
        name = Scr_GetString(0);
        SV_SetConfigstring(824, va("\"%s\" %i", name, duration));
        return;
    }
    Scr_Error("USAGE: VisionSetNaked( <visionset name>, <transition time> )\n");
}

void Scr_VisionSetNight()
{
    uint32_t  NumParam; // [esp+0h] [ebp-1Ch]
    int32_t duration; // [esp+14h] [ebp-8h]
    const char *name; // [esp+18h] [ebp-4h]

    duration = 1000;
    NumParam = Scr_GetNumParam();
    if (NumParam == 1)
        goto LABEL_4;
    if (NumParam == 2)
    {
        duration = (int)(Scr_GetFloat(1) * 1000.0f);
    LABEL_4:
        name = Scr_GetString(0);
        SV_SetConfigstring(825, va("\"%s\" %i", name, duration));
        return;
    }
    Scr_Error("USAGE: VisionSetNight( <visionset name>, <transition time> )\n");
}

void Scr_TableLookup()
{
    const char *stringValue; // [esp+4h] [ebp-18h]
    const char *filename; // [esp+8h] [ebp-14h]
    const char *returnValue; // [esp+Ch] [ebp-10h]
    StringTable *table; // [esp+10h] [ebp-Ch] BYREF
    int32_t returnValueColumn; // [esp+14h] [ebp-8h]
    int32_t comparisonColumn; // [esp+18h] [ebp-4h]

#ifdef KISAK_NO_FASTFILES
    if (true)
#else
    if (IsFastFileLoad())
#endif
    {
        if (Scr_GetNumParam() < 3)
            Scr_Error("USAGE: tableLookup( filename, searchColumnNum, searchValue, returnValueColumnNum )\n");
        filename = Scr_GetString(0);
        StringTable_GetAsset(filename, &table);
        comparisonColumn = Scr_GetInt(1);
        stringValue = Scr_GetString(2);
        returnValueColumn = Scr_GetInt(3);
        returnValue = (char *)StringTable_Lookup(table, comparisonColumn, stringValue, returnValueColumn);
        Scr_AddString(returnValue);
    }
    else
    {
        Com_Printf(16, "You cannot do table lookups without fastfiles.\n");
        Scr_AddString((char *)"");
    }
}

void Scr_TableLookupIString()
{
    const char *stringValue; // [esp+4h] [ebp-18h]
    const char *filename; // [esp+8h] [ebp-14h]
    const char *returnValue; // [esp+Ch] [ebp-10h]
    StringTable *table; // [esp+10h] [ebp-Ch] BYREF
    int32_t returnValueColumn; // [esp+14h] [ebp-8h]
    int32_t comparisonColumn; // [esp+18h] [ebp-4h]

#ifdef KISAK_NO_FASTFILES
    if (true)
#else
    if (IsFastFileLoad())
#endif
    {
        if (Scr_GetNumParam() < 3)
            Scr_Error("USAGE: tableLookupIString( filename, searchColumnNum, searchValue, returnValueColumnNum )\n");
        filename = Scr_GetString(0);
        StringTable_GetAsset(filename, &table);
        comparisonColumn = Scr_GetInt(1);
        stringValue = Scr_GetString(2);
        returnValueColumn = Scr_GetInt(3);
        returnValue = (char *)StringTable_Lookup(table, comparisonColumn, stringValue, returnValueColumn);
        Scr_AddIString(returnValue);
    }
    else
    {
        Com_Printf(16, "You cannot do table lookups without fastfiles.\n");
        Scr_AddIString((char *)"");
    }
}

void GScr_IsPlayer()
{
    if (Scr_GetType(0) == 1 && Scr_GetPointerType(0) == 20 && Scr_GetEntity(0)->client)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void GScr_IsPlayerNumber()
{
    if (Scr_GetInt(0) < 0x40)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void GScr_SetWinningPlayer()
{
    const char *v0; // eax
    char *pszWinner; // [esp+0h] [ebp-410h]
    int32_t iWinner; // [esp+4h] [ebp-40Ch]
    char buffer[1024]; // [esp+8h] [ebp-408h] BYREF
    gentity_s *pEnt; // [esp+40Ch] [ebp-4h]

    pEnt = Scr_GetEntity(0);
    iWinner = pEnt->s.number + 1;
    SV_GetConfigstring(0x13u, buffer, 1024);
    pszWinner = va("%i", iWinner);
    v0 = Info_ValueForKey(buffer, "winner");
    if (I_stricmp(v0, pszWinner))
    {
        Info_SetValueForKey(buffer, "winner", pszWinner);
        SV_SetConfigstring(19, buffer);
    }
}

void GScr_SetWinningTeam()
{
    char *pszWinner; // [esp+0h] [ebp-414h]
    uint16_t team; // [esp+4h] [ebp-410h]
    int32_t iWinner; // [esp+8h] [ebp-40Ch]
    char buffer[1028]; // [esp+Ch] [ebp-408h] BYREF

    team = Scr_GetConstString(0);
    if (team == scr_const.allies)
    {
        iWinner = -2;
    }
    else if (team == scr_const.axis)
    {
        iWinner = -1;
    }
    else
    {
        if (team != scr_const.none)
        {
            Scr_ParamError(0, va("Illegal team string '%s'. Must be allies, axis, or none.", SL_ConvertToString(team)));
            return;
        }
        iWinner = 0;
    }
    SV_GetConfigstring(0x13u, buffer, 1024);
    pszWinner = va("%i", iWinner);
    if (I_stricmp(Info_ValueForKey(buffer, "winner"), pszWinner))
    {
        Info_SetValueForKey(buffer, "winner", pszWinner);
        SV_SetConfigstring(19, buffer);
    }
}

void GScr_Announcement()
{
    uint32_t  NumParam; // eax
    const char *v1; // eax
    char string[1028]; // [esp+0h] [ebp-408h] BYREF

    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Announcement", string, 0x400u);
    v1 = va("%c \"%s\" 2", 99, string);
    SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v1);
}

void GScr_ClientAnnouncement()
{
    uint32_t  NumParam; // eax
    const char *v1; // eax
    char string[1024]; // [esp+0h] [ebp-408h] BYREF
    gentity_s *pEnt; // [esp+404h] [ebp-4h]

    pEnt = Scr_GetEntity(0);
    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(1, NumParam - 1, "Announcement", string, 0x400u);
    v1 = va("%c \"%s\" 2", 99, string);
    SV_GameSendServerCommand(pEnt->s.number, SV_CMD_CAN_IGNORE, v1);
}

void GScr_GetTeamScore()
{
    uint16_t team; // [esp+4h] [ebp-4h]

    team = Scr_GetConstString(0);
    if (team != scr_const.allies && team != scr_const.axis)
    {
        Scr_Error(va("Illegal team string '%s'. Must be allies, or axis.", SL_ConvertToString(team)));
    }
    if (team == scr_const.allies)
        Scr_AddInt(level.teamScores[2]);
    else
        Scr_AddInt(level.teamScores[1]);
}

void GScr_SetTeamScore()
{
    const char *v2; // eax
    unsigned short team; // [esp+0h] [ebp-8h]
    int32_t teamScore; // [esp+4h] [ebp-4h]

    team = Scr_GetConstString(0);
    if (team != scr_const.allies && team != scr_const.axis)
    {
        Scr_Error(va("Illegal team string '%s'. Must be allies, or axis.", SL_ConvertToString(team)));
    }
    teamScore = Scr_GetInt(1);
    if (team == scr_const.allies)
    {
        level.teamScores[2] = teamScore;
        v2 = va("%c %i", 72, teamScore);
    }
    else
    {
        level.teamScores[1] = teamScore;
        v2 = va("%c %i", 71, teamScore);
    }
    SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v2);
    level.bUpdateScoresForIntermission = 1;
}

void GScr_SetClientNameMode()
{
    unsigned short mode; // [esp+0h] [ebp-4h]

    mode = Scr_GetConstString(0);
    if (mode == scr_const.auto_change)
    {
        level.manualNameChange = 0;
    }
    else if (mode == scr_const.manual_change)
    {
        level.manualNameChange = 1;
    }
    else
    {
        Scr_Error("Unknown mode");
    }
}

void GScr_UpdateClientNames()
{
    int32_t result; // eax
    gclient_s *j; // [esp+14h] [ebp-2Ch]
    char oldname[32]; // [esp+18h] [ebp-28h] BYREF
    int32_t i; // [esp+3Ch] [ebp-4h]

    if (!level.manualNameChange)
        Scr_Error("Only works in [manual_change] mode");
    i = 0;
    for (j = level.clients; ; ++j)
    {
        result = i;
        if (i >= level.maxclients)
            break;
        if (j->sess.connected == CON_CONNECTED)
        {
            if (strcmp(j->sess.cs.name, j->sess.newnetname))
            {
                I_strncpyz(oldname, j->sess.cs.name, 32);
                I_strncpyz(j->sess.cs.name, j->sess.newnetname, 16);
                ClientUserinfoChanged(i);
            }
        }
        ++i;
    }
}

void GScr_GetTeamPlayersAlive()
{
    int32_t iLivePlayers; // [esp+0h] [ebp-14h]
    unsigned short team; // [esp+4h] [ebp-10h]
    int32_t iTeamNum; // [esp+8h] [ebp-Ch]
    gentity_s *pEnt; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]

    team = Scr_GetConstString(0);
    if (team != scr_const.allies && team != scr_const.axis)
    {
        Scr_Error(va("Illegal team string '%s'. Must be allies, or axis.", SL_ConvertToString(team)));
    }
    if (team == scr_const.allies)
        iTeamNum = 2;
    else
        iTeamNum = 1;
    iLivePlayers = 0;
    for (i = 0; i < g_maxclients->current.integer; ++i)
    {
        pEnt = &g_entities[i];
        if (pEnt->r.inuse && pEnt->client->sess.cs.team == iTeamNum && pEnt->health > 0)
            ++iLivePlayers;
    }
    Scr_AddInt(iLivePlayers);
}

void __cdecl GScr_GetNumParts()
{
    const XModel *model; // [esp+0h] [ebp-4h]

    model = SV_XModelGet((char*)Scr_GetString(0));
    Scr_AddInt(XModelNumBones(model));
}

void __cdecl GScr_GetPartName()
{
    XModel *model; // [esp+0h] [ebp-10h]
    uint16_t name; // [esp+4h] [ebp-Ch]
    uint32_t  index; // [esp+8h] [ebp-8h]
    uint32_t  numbones; // [esp+Ch] [ebp-4h]

    model = SV_XModelGet((char*)Scr_GetString(0));
    index = Scr_GetInt(1);
    numbones = XModelNumBones(model);
    if (index >= numbones)
    {
        Scr_ParamError(1, va("index out of range (0 - %d)", numbones - 1));
    }
    name = model->boneNames[index];
    if (!name)
        Scr_ParamError(0, "bad model");
    Scr_AddConstString(name);
}

gentity_s *GScr_Earthquake()
{
    gentity_s *result; // eax
    float v1; // [esp+0h] [ebp-2Ch]
    float source[3]; // [esp+10h] [ebp-1Ch] BYREF
    gentity_s *tent; // [esp+1Ch] [ebp-10h]
    int32_t duration; // [esp+20h] [ebp-Ch]
    float radius; // [esp+24h] [ebp-8h]
    float scale; // [esp+28h] [ebp-4h]

    scale = Scr_GetFloat(0);
    duration = (int)(Scr_GetFloat(1) * 1000.0f);
    Scr_GetVector(2u, source);
    radius = Scr_GetFloat(3);
    if (scale <= 0.0)
        Scr_ParamError(0, "Scale must be greater than 0");
    if (duration <= 0)
        Scr_ParamError(1u, "duration must be greater than 0");
    if (radius <= 0.0)
        Scr_ParamError(3u, "Radius must be greater than 0");
    tent = G_TempEntity(source, 61);
    tent->s.lerp.u.earthquake.scale = scale;
    tent->s.lerp.u.earthquake.duration = duration;
    result = tent;
    tent->s.lerp.u.earthquake.radius = radius;
    return result;
}

void __cdecl GScr_ShellShock(scr_entref_t entref)
{
    float v3; // [esp+8h] [ebp-424h]
    int32_t duration; // [esp+18h] [ebp-414h]
    const char *shock; // [esp+1Ch] [ebp-410h]
    gentity_s *ent; // [esp+20h] [ebp-40Ch]
    char s[1024]; // [esp+24h] [ebp-408h] BYREF
    int32_t id; // [esp+428h] [ebp-4h]

    SV_CheckThread();
    ent = GetPlayerEntity(entref);
    if (Scr_GetNumParam() != 2)
        Scr_Error("USAGE: <player> shellshock(<shellshockname>, <duration>)\n");
    shock = Scr_GetString(0);
    for (id = 1; ; ++id)
    {
        if (id >= 16)
        {
            Scr_Error(va("shellshock '%s' was not precached\n", shock));
            return;
        }
        SV_GetConfigstring(id + 1954, s, 1024);
        if (!I_stricmp(s, shock))
            break;
    }
    duration = (int)(Scr_GetFloat(1) * 1000.0f);
    if ((uint32_t )duration > 0xEA60)
    {
        Scr_ParamError(1u, va("duration %g should be >= 0 and <= 60", (double)duration * EQUAL_EPSILON));
    }
    ent->client->ps.shellshockIndex = id;
    ent->client->ps.shellshockTime = level.time;
    ent->client->ps.shellshockDuration = duration;
    if (ent->health > 0)
    {
        ent->client->ps.pm_flags |= PMF_SHELLSHOCKED;
        bgs = &level_bgs;
        BG_AnimScriptEvent(&ent->client->ps, ANIM_ET_SHELLSHOCK, 0, 1);
        if (bgs != &level_bgs)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 5272, 0, "%s", "bgs == &level_bgs");
    }
}

void __cdecl GScr_StopShellShock(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetPlayerEntity(entref);
    if (Scr_GetNumParam())
        Scr_Error("USAGE: <player> stopshellshock()\n");
    ent->client->ps.shellshockIndex = 0;
    ent->client->ps.shellshockTime = 0;
    ent->client->ps.shellshockDuration = 0;
    ent->client->ps.pm_flags &= ~PMF_SHELLSHOCKED;
}

void __cdecl GScr_GetTagOrigin(scr_entref_t entref)
{
    uint32_t  tagName; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    tagName = Scr_GetConstLowercaseString(0);
    GScr_UpdateTagInternal(ent, tagName, &level.cachedTagMat, 1);
    Scr_AddVector(level.cachedTagMat.tagMat[3]);
}

int32_t __cdecl GScr_UpdateTagInternal(
    gentity_s *ent,
    uint32_t  tagName,
    cached_tag_mat_t *cachedTag,
    int32_t showScriptError)
{
    iassert(ent);

    if (ent->s.number == cachedTag->entnum && level.time == cachedTag->time && tagName == cachedTag->name)
        return 1;

    if (!SV_DObjExists(ent))
    {
        if (showScriptError)
        {
            Scr_ObjectError(va("entity has no model defined (classname '%s')", SL_ConvertToString(ent->classname)));
        }
        return 0;
    }
    if (G_DObjGetWorldTagMatrix(ent, tagName, cachedTag->tagMat))
    {
        cachedTag->entnum = ent->s.number;
        cachedTag->time = level.time;
        Scr_SetString(&cachedTag->name, tagName);
        return 1;
    }
    if (showScriptError)
    {
        SV_DObjDumpInfo(ent);
        Scr_ParamError(0, va("tag '%s' does not exist in model '%s' (or any attached submodels)", SL_ConvertToString(tagName), SL_ConvertToString(G_ModelName(ent->model))));
    }
    return 0;
}

void __cdecl GScr_GetTagAngles(scr_entref_t entref)
{
    uint32_t  tagName; // [esp+0h] [ebp-14h]
    float angles[3]; // [esp+4h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+10h] [ebp-4h]

    ent = GetEntity(entref);
    tagName = Scr_GetConstLowercaseString(0);
    GScr_UpdateTagInternal(ent, tagName, &level.cachedTagMat, 1);
    AxisToAngles(*(const mat3x3*)&level.cachedTagMat.tagMat, angles);
    Scr_AddVector(angles);
}

void __cdecl GScr_SetDepthOfField(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float dofNearBlur; // [esp+10h] [ebp-1Ch]
    float dofFarBlur; // [esp+14h] [ebp-18h]
    float dofNearStart; // [esp+18h] [ebp-14h]
    float dofFarStart; // [esp+1Ch] [ebp-10h]
    gentity_s *ent; // [esp+20h] [ebp-Ch]
    float dofFarEnd; // [esp+24h] [ebp-8h]
    float dofNearEnd; // [esp+28h] [ebp-4h]

    ent = GetPlayerEntity(entref);
    if (Scr_GetNumParam() != 6)
        Scr_Error("Incorrect number of parameters\n");
    dofNearStart = Scr_GetFloat(0);
    dofNearEnd = Scr_GetFloat(1);
    dofFarStart = Scr_GetFloat(2);
    dofFarEnd = Scr_GetFloat(3);
    dofNearBlur = Scr_GetFloat(4);
    dofFarBlur = Scr_GetFloat(5);
    if (dofNearStart < 0.0)
        Scr_ParamError(0, "near start must be >= 0");
    if (dofNearEnd < 0.0)
        Scr_ParamError(1u, "near end must be >= 0");
    if (dofFarStart < 0.0)
        Scr_ParamError(2u, "far start must be >= 0");
    if (dofFarEnd < 0.0)
        Scr_ParamError(3u, "far end must be >= 0");
    if (dofNearBlur < 4.0 || dofNearBlur > 10.0)
    {
        v1 = va("near blur should be between %g and %g", 4.0, 10.0);
        Scr_ParamError(4u, v1);
    }
    if (dofFarBlur < 0.0 || dofNearBlur < (double)dofFarBlur)
    {
        v2 = va("far blur should be >= %g and <= near blur", 0.0);
        Scr_ParamError(5u, v2);
    }
    if (dofNearEnd <= (double)dofNearStart)
    {
        dofNearStart = 0.0;
        dofNearEnd = 0.0;
    }
    if (dofFarEnd <= (double)dofFarStart || dofFarBlur == 0.0)
    {
        dofFarStart = 0.0;
        dofFarEnd = 0.0;
    }
    else if (dofNearEnd > (double)dofFarStart)
    {
        Scr_ParamError(
            2u,
            "far start must be >= near end, or far depth of field should be disabled with far start >= far end or far blur == 0");
    }
    ent->client->ps.dofNearStart = dofNearStart;
    ent->client->ps.dofNearEnd = dofNearEnd;
    ent->client->ps.dofFarStart = dofFarStart;
    ent->client->ps.dofFarEnd = dofFarEnd;
    ent->client->ps.dofNearBlur = dofNearBlur;
    ent->client->ps.dofFarBlur = dofFarBlur;
}

void __cdecl GScr_SetViewModelDepthOfField(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-Ch]
    float dofEnd; // [esp+4h] [ebp-8h]
    float dofStart; // [esp+8h] [ebp-4h]

    ent = GetPlayerEntity(entref);
    dofStart = Scr_GetFloat(0);
    dofEnd = Scr_GetFloat(1);
    if (dofStart < 0.0)
        Scr_ParamError(0, "start must be >= 0");
    if (dofEnd < 0.0)
        Scr_ParamError(1u, "end must be >= 0");
    if (dofEnd <= (double)dofStart)
    {
        dofStart = 0.0;
        dofEnd = 0.0;
    }
    ent->client->ps.dofViewmodelStart = dofStart;
    ent->client->ps.dofViewmodelEnd = dofEnd;
}

void __cdecl GScr_ViewKick(scr_entref_t entref)
{
    double Float; // st7
    const char *v2; // eax
    float origin[3]; // [esp+8h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+14h] [ebp-4h]

    ent = GetPlayerEntity(entref);
    if (Scr_GetNumParam() != 2)
        Scr_Error("USAGE: <player> viewkick <force 0-127> <source position>\n");
    ent->client->damage_blood = (ent->maxHealth * Scr_GetInt(0) + 50) / 100;
    if (ent->client->damage_blood < 0)
    {
        Float = Scr_GetFloat(0);
        v2 = va("viewkick: damage %g < 0\n", Float);
        Scr_Error(v2);
    }
    Scr_GetVector(1u, origin);
    Vec3Sub(ent->client->ps.origin, origin, ent->client->damage_from);
}

void __cdecl GScr_LocalToWorldCoords(scr_entref_t entref)
{
    float vLocal[3]; // [esp+0h] [ebp-40h] BYREF
    float vWorld[3]; // [esp+Ch] [ebp-34h] BYREF
    gentity_s *ent; // [esp+18h] [ebp-28h]
    float axis[3][3]; // [esp+1Ch] [ebp-24h] BYREF

    ent = GetEntity(entref);
    Scr_GetVector(0, vLocal);
    AnglesToAxis(ent->r.currentAngles, axis);
    MatrixTransformVector(vLocal, axis, vWorld);
    Vec3Add(vWorld, ent->r.currentOrigin, vWorld);
    Scr_AddVector(vWorld);
}

void __cdecl GScr_SetRightArc(scr_entref_t entref)
{
    turretInfo_s *pTurretInfo; // [esp+0h] [ebp-8h]

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    pTurretInfo->arcmin[1] = -Scr_GetFloat(0);
    if (pTurretInfo->arcmin[1] > 0.0)
        pTurretInfo->arcmin[1] = 0.0;
}

void __cdecl GScr_SetLeftArc(scr_entref_t entref)
{
    turretInfo_s *pTurretInfo; // [esp+0h] [ebp-8h]

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    pTurretInfo->arcmax[1] = Scr_GetFloat(0);
    if (pTurretInfo->arcmax[1] < 0.0)
        pTurretInfo->arcmax[1] = 0.0;
}

void __cdecl GScr_SetTopArc(scr_entref_t entref)
{
    turretInfo_s *pTurretInfo; // [esp+0h] [ebp-8h]

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    pTurretInfo->arcmin[0] = -Scr_GetFloat(0);
    if (pTurretInfo->arcmin[0] > 0.0)
        pTurretInfo->arcmin[0] = 0.0;
}

void __cdecl GScr_SetBottomArc(scr_entref_t entref)
{
    turretInfo_s *pTurretInfo; // [esp+0h] [ebp-8h]

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    pTurretInfo->arcmax[0] = Scr_GetFloat(0);
    if (pTurretInfo->arcmax[0] < 0.0)
        pTurretInfo->arcmax[0] = 0.0;
}

void __cdecl GScr_PlaceSpawnPoint(scr_entref_t entref)
{
    uint16_t EntityHitId; // ax
    float vEnd[3]; // [esp+10h] [ebp-48h] BYREF
    trace_t trace; // [esp+1Ch] [ebp-3Ch] BYREF
    gentity_s *pEnt; // [esp+48h] [ebp-10h]
    float vStart[3]; // [esp+4Ch] [ebp-Ch] BYREF

    pEnt = GetEntity(entref);
    vStart[0] = pEnt->r.currentOrigin[0];
    vStart[1] = pEnt->r.currentOrigin[1];
    vStart[2] = pEnt->r.currentOrigin[2];
    vEnd[0] = pEnt->r.currentOrigin[0];
    vEnd[1] = pEnt->r.currentOrigin[1];
    vEnd[2] = pEnt->r.currentOrigin[2];
    vEnd[2] = vEnd[2] + 128.0;
    G_TraceCapsule(
        &trace,
        vStart,
        (float *)playerMins,
        (float *)playerMaxs,
        vEnd,
        pEnt->s.number,
        0x2810011);
    Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
    vEnd[0] = vStart[0];
    vEnd[1] = vStart[1];
    vEnd[2] = vStart[2] - 262144.0;
    G_TraceCapsule(
        &trace,
        vStart,
        (float *)playerMins,
        (float *)playerMaxs,
        vEnd,
        pEnt->s.number,
        0x2810011);
    EntityHitId = Trace_GetEntityHitId(&trace);
    pEnt->s.groundEntityNum = EntityHitId;
    g_entities[pEnt->s.groundEntityNum].flags |= FL_GROUND_ENT;
    Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
    G_TraceCapsule(
        &trace,
        vStart,
        (float *)playerMins,
        (float *)playerMaxs,
        vStart,
        pEnt->s.number,
        0x2810011);
    if (trace.allsolid)
        Com_PrintWarning(
            23,
            "WARNING: Spawn point entity %i is in solid at (%i, %i, %i)\n",
            pEnt->s.number,
            (int)pEnt->r.currentOrigin[0],
            (int)pEnt->r.currentOrigin[1],
            (int)pEnt->r.currentOrigin[2]);
    G_SetOrigin(pEnt, vStart);
}

void GScr_MapRestart()
{
    if (level.finished)
    {
        if (level.finished == 1)
            Scr_Error("map_restart already called");
        else
            Scr_Error("exitlevel already called");
    }
    level.finished = 1;
    level.savepersist = 0;
    if (Scr_GetNumParam())
        level.savepersist = Scr_GetInt(0);
    Cbuf_AddText(0, "fast_restart\n");
}

void GScr_LoadMap()
{
    const char *mapname; // [esp+4h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        mapname = Scr_GetString(0);
        if (SV_MapExists((char*)mapname))
        {
            if (level.finished)
            {
                if (level.finished == 2)
                    Scr_Error("map already called");
                else
                    Scr_Error("exitlevel already called");
            }
            level.finished = 2;
            level.savepersist = 0;
            if (Scr_GetNumParam() > 1)
                level.savepersist = Scr_GetInt(1);
            Cbuf_AddText(0, va("map %s\n", mapname));
        }
    }
}

void GScr_ExitLevel()
{
    if (level.finished)
    {
        if (level.finished == 1)
            Scr_Error("map_restart already called");
        else
            Scr_Error("exitlevel already called");
    }
    level.finished = 3;
    level.savepersist = 0;
    if (Scr_GetNumParam())
        level.savepersist = Scr_GetInt(0);
    KISAK_NULLSUB();
    ExitLevel();
}

void GScr_AddTestClient()
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = SV_AddTestClient();
    if (ent)
        Scr_AddEntity(ent);
}

void GScr_MakeDvarServerInfo()
{
    uint32_t  NumParam; // eax
    char v1; // al
    char string[1028]; // [esp+10h] [ebp-828h] BYREF
    int32_t len; // [esp+414h] [ebp-424h]
    char outString[1024]; // [esp+418h] [ebp-420h] BYREF
    const char *dvarName; // [esp+820h] [ebp-18h]
    int32_t type; // [esp+824h] [ebp-14h]
    int32_t i; // [esp+828h] [ebp-10h]
    const dvar_s *dvar; // [esp+82Ch] [ebp-Ch]
    const char *dvarValue; // [esp+830h] [ebp-8h]
    char *pCh; // [esp+834h] [ebp-4h]

    dvarName = Scr_GetString(0);
    dvar = Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_AddFlags(dvar, 256);
    }
    else
    {
        type = Scr_GetType(1);
        if (type == 3)
        {
            NumParam = Scr_GetNumParam();
            Scr_ConstructMessageString(1, NumParam - 1, "Dvar Value", string, 0x400u);
            dvarValue = string;
        }
        else
        {
            dvarValue = Scr_GetString(1);
        }
        len = strlen(dvarValue);
        pCh = outString;
        i = 0;
        while (i < 1023 && dvarValue[i])
        {
            v1 = I_CleanChar(dvarValue[i]);
            *pCh = v1;
            if (*pCh == 34)
                *pCh = 39;
            ++i;
            ++pCh;
        }
        *pCh = 0;
        Dvar_RegisterString(dvarName, (char *)dvarValue, DVAR_TEMP | DVAR_EXTERNAL, "Script defined user info dvar");
    }
}

void GScr_AllClientsPrint()
{
    if (Scr_GetNumParam())
    {
        SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, va("%c \"%s\"", 101, Scr_GetString(0)));
    }
}

void GScr_MapExists()
{
    const char *mapname; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        mapname = Scr_GetString(0);
        if (SV_MapExists((char*)mapname))
            Scr_AddInt(1);
        else
            Scr_AddInt(0);
    }
}

void GScr_IsValidGameType()
{
    const char *gametype; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        gametype = Scr_GetString(0);
        if (Scr_IsValidGameType(gametype))
            Scr_AddInt(1);
        else
            Scr_AddInt(0);
    }
}

void GScr_SetVoteString()
{
    if (Scr_GetNumParam())
    {
        SV_SetConfigstring(14, Scr_GetString(0));
        SV_SetConfigstring(13, va("%i %i", level.voteTime, Dvar_GetInt("sv_serverId")));
        SV_SetConfigstring(15, va("%i", level.voteYes));
        SV_SetConfigstring(16, va("%i", level.voteNo));
    }
}

void GScr_SetVoteTime()
{
    int32_t time; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        time = Scr_GetInt(0);
        SV_SetConfigstring(13, va("%i %i", time, Dvar_GetInt("sv_serverId")));
        SV_SetConfigstring(15, va("%i", level.voteYes));
        SV_SetConfigstring(16, va("%i", level.voteNo));
    }
}

void GScr_SetVoteYesCount()
{
    int32_t yes; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        yes = Scr_GetInt(0);
        SV_SetConfigstring(15, va("%i", yes));
        SV_SetConfigstring(16, va("%i", level.voteNo));
    }
}

void GScr_SetVoteNoCount()
{
    int32_t no; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        no = Scr_GetInt(0);
        SV_SetConfigstring(16, va("%i", no));
    }
}

void GScr_KickPlayer()
{
    int32_t playernum; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        playernum = Scr_GetInt(0);
        Cbuf_AddText(0, va("tempBanClient %i\n", playernum));
    }
}

void GScr_BanPlayer()
{
    int32_t playernum; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        playernum = Scr_GetInt(0);
        Cbuf_AddText(0, va("banClient %i\n", playernum));
    }
}

void GScr_ClientPrint()
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        ent = Scr_GetEntity(0);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, va("%c \"%s\"", 101, Scr_GetString(1)));
    }
}

void GScr_OpenFile()
{
    char *fullpathname; // [esp+3Ch] [ebp-20h]
    int32_t filesize; // [esp+40h] [ebp-1Ch]
    const char *filename; // [esp+44h] [ebp-18h]
    int32_t tempFile; // [esp+48h] [ebp-14h] BYREF
    int32_t *f; // [esp+4Ch] [ebp-10h]
    const char *mode; // [esp+50h] [ebp-Ch]
    int32_t filenum; // [esp+58h] [ebp-4h]

    f = 0;
    if (Scr_GetNumParam() > 1)
    {
        filename = Scr_GetString(0);
        mode = Scr_GetString(1);
        for (filenum = 0; filenum < 1; ++filenum)
        {
            if (!level.openScriptIOFileHandles[filenum])
            {
                //f = (int32_t *)(4 * filenum + 23808484);
                f = &level.openScriptIOFileHandles[filenum];
                break;
            }
        }
        if (!f)
        {
            Com_Printf(23, "OpenFile failed.  %i files already open\n", 1);
            Scr_AddInt(-1);
            return;
        }
        if (!strcmp(mode, "read"))
        {
            fullpathname = va("%s/%s", "scriptdata", filename);
            filesize = FS_FOpenFileByMode(fullpathname, &tempFile, FS_READ);
            if (filesize >= 0)
            {
                level.openScriptIOFileBuffers[filenum] = Z_VirtualAlloc(filesize + 1, "GScr_OpenFile", 10);
                FS_Read((uint8_t *)level.openScriptIOFileBuffers[filenum], filesize, tempFile);
                FS_FCloseFile(tempFile);
                level.openScriptIOFileBuffers[filenum][filesize] = 0;
                Com_BeginParseSession(filename);
                Com_SetCSV(1);
                level.currentScriptIOLineMark[filenum].lines = 0;
                Scr_AddInt(filenum);
            }
            else
            {
                Scr_AddInt(-1);
            }
            return;
        }
        if (!strcmp(mode, "write"))
        {
            *f = FS_FOpenTextFileWrite(va("%s/%s", "scriptdata", filename));
            if (!*f)
                goto LABEL_15;
        }
        else
        {
            if (strcmp(mode, "append"))
            {
                Com_Printf(23, "Valid openfile modes are 'write', 'read', and 'append'\n");
                Scr_AddInt(-1);
                return;
            }
            if ((FS_FOpenFileByMode(va("%s/%s", "scriptdata", filename), f, FS_APPEND) & 0x80000000) != 0)
            {
            LABEL_15:
                Scr_AddInt(-1);
                return;
            }
        }
        Scr_AddInt(filenum);
    }
}

void GScr_CloseFile()
{
    int32_t filenum; // [esp+0h] [ebp-4h]

    if (Scr_GetNumParam())
    {
        filenum = Scr_GetInt(0);
        if ((uint32_t )filenum >= 2)
        {
            Com_Printf(23, "CloseFile failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1);
            return;
        }
        if (level.openScriptIOFileHandles[filenum])
        {
            if (level.openScriptIOFileBuffers[filenum])
                MyAssertHandler(
                    ".\\game_mp\\g_scr_main_mp.cpp",
                    6112,
                    0,
                    "%s",
                    "!((level.openScriptIOFileHandles[filenum] != 0) && (level.openScriptIOFileBuffers[filenum] != NULL))");
        }
        if (level.openScriptIOFileHandles[filenum])
        {
            FS_FCloseFile(level.openScriptIOFileHandles[filenum]);
            level.openScriptIOFileHandles[filenum] = 0;
        }
        else
        {
            if (!level.openScriptIOFileBuffers[filenum])
            {
                Com_Printf(23, "CloseFile failed, file number %i was not open\n", filenum);
                Scr_AddInt(-1);
                return;
            }
            Com_EndParseSession();
            Z_VirtualFree(level.openScriptIOFileBuffers[filenum]);
            level.openScriptIOFileBuffers[filenum] = 0;
        }
        Scr_AddInt(1);
    }
}

void GScr_FPrintln()
{
    Scr_FPrint_internal(0);
}

void __cdecl Scr_FPrint_internal(bool commaBetweenFields)
{
    uint32_t  NumParam; // eax
    const char *s; // [esp+10h] [ebp-Ch]
    uint32_t  arg; // [esp+14h] [ebp-8h]
    int32_t filenum; // [esp+18h] [ebp-4h]

    if (Scr_GetNumParam() > 1)
    {
        filenum = Scr_GetInt(0);
        if ((uint32_t )filenum < 2)
        {
            if (level.openScriptIOFileHandles[filenum])
            {
                for (arg = 1; arg < Scr_GetNumParam(); ++arg)
                {
                    s = Scr_GetString(arg);
                    FS_Write(s, strlen(s), level.openScriptIOFileHandles[filenum]);
                    if (commaBetweenFields)
                        FS_Write((char*)",", 1u, level.openScriptIOFileHandles[filenum]);
                }
                FS_Write((char *)"\n", 1u, level.openScriptIOFileHandles[filenum]);
                NumParam = Scr_GetNumParam();
                Scr_AddInt(NumParam - 1);
            }
            else
            {
                Com_Printf(23, "FPrintln failed, file number %i was not open for writing\n", filenum);
                Scr_AddInt(-1);
            }
        }
        else
        {
            Com_Printf(23, "FPrintln failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1);
        }
    }
    else
    {
        Com_Printf(23, "fprintln requires at least 2 parameters (file, output)\n");
        Scr_AddInt(-1);
    }
}

void GScr_FPrintFields()
{
    Scr_FPrint_internal(1);
}

void GScr_FReadLn()
{
    int32_t v0; // eax
    int32_t ArgCountOnLine; // eax
    bool eof; // [esp+0h] [ebp-10h]
    const char *buf; // [esp+4h] [ebp-Ch] BYREF
    const char *token; // [esp+8h] [ebp-8h]
    int32_t filenum; // [esp+Ch] [ebp-4h]

    if (Scr_GetNumParam())
    {
        filenum = Scr_GetInt(0);
        if ((uint32_t )filenum < 2)
        {
            if (level.openScriptIOFileBuffers[filenum])
            {
                buf = level.openScriptIOFileBuffers[filenum];
                if (level.currentScriptIOLineMark[filenum].lines)
                {
                    Com_ParseReturnToMark(&buf, &level.currentScriptIOLineMark[filenum]);
                    Com_SkipRestOfLine(&buf);
                    Com_ParseSetMark(&buf, &level.currentScriptIOLineMark[filenum]);
                    token = (const char *)Com_Parse(&buf);
                    eof = *token == 0;
                    Com_ParseReturnToMark(&buf, &level.currentScriptIOLineMark[filenum]);
                    if (eof)
                    {
                        Scr_AddInt(-1);
                    }
                    else
                    {
                        ArgCountOnLine = Com_GetArgCountOnLine(&buf);
                        Scr_AddInt(ArgCountOnLine);
                    }
                }
                else
                {
                    Com_ParseSetMark(&buf, &level.currentScriptIOLineMark[filenum]);
                    v0 = Com_GetArgCountOnLine(&buf);
                    Scr_AddInt(v0);
                }
            }
            else
            {
                Com_Printf(23, "freadln failed, file number %i was not open for reading\n", filenum);
                Scr_AddInt(-1);
            }
        }
        else
        {
            Com_Printf(23, "freadln failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1);
        }
    }
    else
    {
        Com_Printf(23, "freadln requires a parameter - the file to read from\n");
        Scr_AddInt(-1);
    }
}

void GScr_FGetArg()
{
    const char *buf; // [esp+0h] [ebp-14h] BYREF
    int32_t arg; // [esp+4h] [ebp-10h]
    int32_t i; // [esp+8h] [ebp-Ch]
    const char *token; // [esp+Ch] [ebp-8h]
    int32_t filenum; // [esp+10h] [ebp-4h]

    if (Scr_GetNumParam() > 1)
    {
        filenum = Scr_GetInt(0);
        arg = Scr_GetInt(1);
        if ((uint32_t )filenum < 2)
        {
            if (arg >= 0)
            {
                if (level.openScriptIOFileBuffers[filenum])
                {
                    token = 0;
                    buf = level.openScriptIOFileBuffers[filenum];
                    Com_ParseReturnToMark(&buf, &level.currentScriptIOLineMark[filenum]);
                    for (i = 0; i <= arg; ++i)
                    {
                        token = (const char *)Com_ParseOnLine(&buf);
                        if (!*token)
                        {
                            Com_Printf(
                                23,
                                "freadline failed, there aren't %i arguments on this line, there are only %i arguments\n",
                                arg + 1,
                                i);
                            Scr_AddString((char *)"");
                            return;
                        }
                    }
                    Scr_AddString((char *)token);
                }
                else
                {
                    Com_Printf(23, "freadline failed, file number %i was not open for reading\n", filenum);
                    Scr_AddString((char *)"");
                }
            }
            else
            {
                Com_Printf(23, "freadline failed, invalid argument number %i\n", arg);
                Scr_AddString((char *)"");
            }
        }
        else
        {
            Com_Printf(23, "freadline failed, invalid file number %i\n", filenum);
            Scr_AddString((char *)"");
        }
    }
    else
    {
        Com_Printf(23, "freadline requires at least 2 parameters (file, string)\n");
        Scr_AddString((char *)"");
    }
}

void __cdecl GScr_GetStat(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *playerEnt; // [esp+0h] [ebp-Ch]
    int32_t index; // [esp+4h] [ebp-8h]
    int32_t value; // [esp+8h] [ebp-4h]

    playerEnt = GetEntity(entref);
    index = Scr_GetInt(0);
    if (!playerEnt->client)
        Scr_Error("getstat: entity must be a player entity");
    if ((uint32_t )index >= 0xDAC)
    {
        v1 = va("getstat: invalid index %i", index);
        Scr_Error(v1);
    }
    if (playerEnt->client->sess.connected <= CON_DISCONNECTED)
        Scr_Error("getstat: called on a disconnected player");
    value = SV_GetClientStat(playerEnt->s.number, index);
    Scr_AddInt(value);
}

void __cdecl GScr_SetStat(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    gentity_s *playerEnt; // [esp+4h] [ebp-10h]
    VariableUnion index; // [esp+8h] [ebp-Ch]
    uint32_t  value; // [esp+Ch] [ebp-8h]
    int32_t argc; // [esp+10h] [ebp-4h]

    playerEnt = GetEntity(entref);
    if (!playerEnt->client)
        Scr_Error("setstat: entity must be a player entity");
    argc = Scr_GetNumParam();
    index.intValue = 0;
    value = 0;
    if (argc == 2)
    {
        index.intValue = Scr_GetInt(0);
        if (index.intValue >= 0xDACu)
        {
            v1 = va("setstat: invalid index %i", index.intValue);
            Scr_ParamError(0, v1);
        }
        value = Scr_GetInt(1);
        if (index.intValue < 2000 && value >= 0x100)
        {
            v2 = va("setstat: index %i is a byte value, and you're trying to set it to %i", index.intValue, value);
            Scr_ParamError(1u, v2);
        }
    }
    else
    {
        v3 = va("setstat: takes 2 arguments, got %i.\n", argc);
        Scr_Error(v3);
    }
    SV_SetClientStat(playerEnt->s.number, index.intValue, value);
}

void __cdecl GScr_SetTeamForTrigger(scr_entref_t entref)
{
    uint16_t team; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->classname != scr_const.trigger_use && ent->classname != scr_const.trigger_use_touch)
    {
        Scr_Error(va("setteamfortrigger: trigger entity must be of type %s or %s", SL_ConvertToString(scr_const.trigger_use), SL_ConvertToString(scr_const.trigger_use_touch)));
    }
    team = Scr_GetConstString(0);
    if (team == scr_const.allies)
    {
        ent->team = 2;
    }
    else if (team == scr_const.axis)
    {
        ent->team = 1;
    }
    else if (team == scr_const.none)
    {
        ent->team = 0;
    }
    else
    {
        Scr_Error(va("setteamfortrigger: invalid team used must be %s, %s or %s", SL_ConvertToString(scr_const.allies), SL_ConvertToString(scr_const.axis), SL_ConvertToString(scr_const.none)));
    }
}

void __cdecl GScr_ClientClaimTrigger(scr_entref_t entref)
{
    gentity_s *clientEnt; // [esp+0h] [ebp-8h]
    gentity_s *triggerEnt; // [esp+4h] [ebp-4h]

    clientEnt = GetEntity(entref);
    if (!clientEnt->client)
        Scr_Error("clientclaimtrigger: claimer must be a client.");
    triggerEnt = Scr_GetEntity(0);
    if (triggerEnt->classname != scr_const.trigger_use && triggerEnt->classname != scr_const.trigger_use_touch)
    {
        Scr_Error(va("clientclaimtrigger: trigger entity must be of type %s or %s", SL_ConvertToString(scr_const.trigger_use), SL_ConvertToString(scr_const.trigger_use_touch)));
    }
    if (triggerEnt->item[1].ammoCount == ENTITYNUM_NONE || triggerEnt->item[1].ammoCount == clientEnt->client->ps.clientNum)
        triggerEnt->item[1].ammoCount = clientEnt->client->ps.clientNum;
}

void __cdecl GScr_ClientReleaseTrigger(scr_entref_t entref)
{
    gentity_s *clientEnt; // [esp+0h] [ebp-8h]
    gentity_s *triggerEnt; // [esp+4h] [ebp-4h]

    clientEnt = GetEntity(entref);
    if (!clientEnt->client)
        Scr_Error("clientreleasetrigger: releaser must be a client.");
    triggerEnt = Scr_GetEntity(0);
    if (triggerEnt->classname != scr_const.trigger_use && triggerEnt->classname != scr_const.trigger_use_touch)
    {
        Scr_Error(va("clientreleasetrigger: trigger entity must be of type %s or %s", SL_ConvertToString(scr_const.trigger_use), SL_ConvertToString(scr_const.trigger_use_touch)));
    }
    if (triggerEnt->item[1].ammoCount == clientEnt->client->ps.clientNum)
        triggerEnt->item[1].ammoCount = ENTITYNUM_NONE;
}

void __cdecl GScr_ReleaseClaimedTrigger(scr_entref_t entref)
{
    gentity_s *triggerEnt; // [esp+0h] [ebp-4h]

    triggerEnt = GetEntity(entref);
    if (triggerEnt->classname != scr_const.trigger_use && triggerEnt->classname != scr_const.trigger_use_touch)
    {
        Scr_Error(va("releaseclaimedtrigger: trigger entity must be of type %s or %s", SL_ConvertToString(scr_const.trigger_use), SL_ConvertToString(scr_const.trigger_use_touch)));
    }
    triggerEnt->item[1].ammoCount = ENTITYNUM_NONE;
}

void GScr_SetMapCenter()
{
    float mapCenter[3]; // [esp+0h] [ebp-Ch] BYREF

    if (Scr_GetNumParam() != 1)
        Scr_Error("Expected 1 argument to setMapCenter()");
    Scr_GetVector(0, mapCenter);
    SV_SetMapCenter(mapCenter);
}

void GScr_SetGameEndTime()
{
    VariableUnion v0; // eax

    if (Scr_GetNumParam() != 1)
        Scr_Error("Expected 1 argument to setGameEndTime()");
    v0.intValue = Scr_GetInt(0);
    SV_SetGameEndTime(v0.intValue);
}

void GScr_SetMiniMap()
{
    float v1; // [esp+20h] [ebp-58h]
    float v2; // [esp+2Ch] [ebp-4Ch]
    float diff; // [esp+30h] [ebp-48h]
    float diff_4; // [esp+34h] [ebp-44h]
    float upperLeft; // [esp+38h] [ebp-40h]
    float upperLeft_4; // [esp+3Ch] [ebp-3Ch]
    const char *material; // [esp+40h] [ebp-38h]
    char northYawString[32]; // [esp+44h] [ebp-34h] BYREF
    float north[2]; // [esp+68h] [ebp-10h]
    float lowerRight[2]; // [esp+70h] [ebp-8h]

    if (Scr_GetNumParam() != 5)
        Scr_Error("Expecting 5 arguments");
    material = Scr_GetString(0);
    upperLeft = Scr_GetFloat(1);
    upperLeft_4 = Scr_GetFloat(2);
    lowerRight[0] = Scr_GetFloat(3);
    lowerRight[1] = Scr_GetFloat(4);
    SV_GetConfigstring(0x336u, northYawString, 32);
    v1 = atof(northYawString);
    v2 = v1 * 0.01745329238474369;
    north[0] = cos(v2);
    north[1] = sin(v2);
    diff = lowerRight[0] - upperLeft;
    diff_4 = lowerRight[1] - upperLeft_4;
    level.compassMapWorldSize[0] = diff * north[1] - diff_4 * north[0];
    level.compassMapWorldSize[1] = -diff * north[0] - diff_4 * north[1];
    if (level.compassMapWorldSize[0] < 0.0 || level.compassMapWorldSize[1] < 0.0)
        Scr_Error(
            "lower-right X and Y coordinates must be both south and east of upper-left X and Y coordinates in terms of the northyaw");
    level.compassMapUpperLeft[0] = upperLeft;
    level.compassMapUpperLeft[1] = upperLeft_4;
    SV_SetConfigstring(823, va("\"%s\" %f %f %f %f", material, upperLeft, upperLeft_4, lowerRight[0], lowerRight[1]));
}

bool GScr_SetTeamRadar()
{
    bool result; // al
    bool radarAvailable; // [esp+3h] [ebp-5h]
    uint16_t team; // [esp+4h] [ebp-4h]

    team = Scr_GetConstString(0);
    if (team != scr_const.allies && team != scr_const.axis && team != scr_const.none)
    {
        Scr_ParamError(0, va("Illegal team string '%s'. Must be allies, axis, or none.", SL_ConvertToString(team)));
    }
    radarAvailable = Scr_GetInt(1) != 0;
    if (team == scr_const.allies)
    {
        result = radarAvailable;
        level.teamHasRadar[2] = radarAvailable;
    }
    else if (team == scr_const.axis)
    {
        result = radarAvailable;
        level.teamHasRadar[1] = radarAvailable;
    }
    else
    {
        iassert(team == scr_const.none);
        result = radarAvailable;
        level.teamHasRadar[0] = radarAvailable;
    }
    return result;
}

void GScr_GetTeamRadar()
{
    bool radarAvailable; // [esp+3h] [ebp-5h]
    uint16_t team; // [esp+4h] [ebp-4h]

    team = Scr_GetConstString(0);
    if (team == scr_const.allies)
    {
        radarAvailable = level.teamHasRadar[2];
    }
    else if (team == scr_const.axis)
    {
        radarAvailable = level.teamHasRadar[1];
    }
    else
    {
        if (team != scr_const.none)
        {
            Scr_ParamError(0, va("Illegal team string '%s'. Must be allies, axis, or none.", SL_ConvertToString(team)));
            return;
        }
        radarAvailable = level.teamHasRadar[0];
    }
    Scr_AddInt(radarAvailable);
}

void GScr_GetArrayKeys()
{
    if (Scr_GetPointerType(0) != VAR_ARRAY)
    {
        Scr_ParamError(0, va("Parameter (%s) must be an array", Scr_GetTypeName(0)));
    }
    Scr_AddArrayKeys(Scr_GetObject(0));
}

void __cdecl GScr_IsOnLadder(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (!ent->client)
        Scr_Error("setstat: entity must be a player entity");
    Scr_AddBool((ent->client->ps.pm_flags & PMF_LADDER) != 0);
}

void __cdecl GScr_IsMantling(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GetEntity(entref);
    if (!ent->client)
        Scr_Error("setstat: entity must be a player entity");
    Scr_AddBool((ent->client->ps.pm_flags & PMF_MANTLE) != 0);
}

void __cdecl GScr_StartRagdoll(scr_entref_t entref)
{
    trType_t v1; // [esp+0h] [ebp-10h]
    trType_t trType; // [esp+4h] [ebp-Ch]
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    ent = GetEntity(entref);
    if (Scr_GetNumParam())
        Scr_GetInt(0);
    trType = ent->s.lerp.pos.trType;
    if (trType == TR_INTERPOLATE)
    {
        ent->s.lerp.pos.trType = TR_RAGDOLL_INTERPOLATE;
    }
    else if (trType == TR_GRAVITY)
    {
        ent->s.lerp.pos.trType = TR_RAGDOLL_GRAVITY;
    }
    else
    {
        ent->s.lerp.pos.trType = TR_FIRST_RAGDOLL;
    }
    v1 = ent->s.lerp.apos.trType;
    if (v1 == TR_INTERPOLATE)
    {
        ent->s.lerp.apos.trType = TR_RAGDOLL_INTERPOLATE;
    }
    else if (v1 == TR_GRAVITY)
    {
        ent->s.lerp.apos.trType = TR_RAGDOLL_GRAVITY;
    }
    else
    {
        ent->s.lerp.apos.trType = TR_FIRST_RAGDOLL;
    }
}

void __cdecl GScr_IsRagdoll(scr_entref_t entref)
{
    bool isRagdoll; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GetEntity(entref);
    isRagdoll = Com_IsRagdollTrajectory(&ent->s.lerp.pos);
    Scr_AddInt(isRagdoll);
}

void __cdecl GScr_GetCorpseAnim(scr_entref_t entref)
{
    const char *v1; // eax
    XAnim_s *treeAnims; // [esp+0h] [ebp-14h]
    gentity_s *ent; // [esp+8h] [ebp-Ch]
    scr_anim_s anim; // [esp+Ch] [ebp-8h]
    corpseInfo_t *corpseInfo; // [esp+10h] [ebp-4h]

    ent = GetEntity(entref);
    if (ent->s.eType == ET_PLAYER_CORPSE)
    {
        corpseInfo = &g_scr_data.playerCorpseInfo[G_GetPlayerCorpseIndex(ent)];
        anim.index = ent->s.legsAnim & 0xFDFF;
        treeAnims = XAnimGetAnims(corpseInfo->tree);
        anim.tree = Scr_GetAnimsIndex(treeAnims);
        Scr_AddAnim(anim);
    }
    else
    {
        v1 = va("Only valid on player corpses");
        Scr_Error(v1);
    }
}

void __cdecl ScrCmd_ItemWeaponSetAmmo(scr_entref_t entref)
{
    const char *v1; // eax
    int32_t v2; // edx
    VariableUnion v3; // [esp+0h] [ebp-24h]
    int32_t reserveAmmo; // [esp+Ch] [ebp-18h]
    int32_t clipAmmo; // [esp+10h] [ebp-14h]
    uint32_t  altIndex; // [esp+14h] [ebp-10h]
    WeaponDef *weapDef; // [esp+1Ch] [ebp-8h]
    gentity_s *itemEnt; // [esp+20h] [ebp-4h]

    itemEnt = GetEntity(entref);
    if (itemEnt->s.eType != ET_ITEM)
        Scr_Error("Entity is not an item.");
    if (bg_itemlist[itemEnt->s.index.brushmodel].giType != IT_WEAPON)
        Scr_Error("Item entity is not a weapon.");
    clipAmmo = Scr_GetInt(0);
    if (clipAmmo < 0)
        Scr_ParamError(0, "Ammo count must not be negative");
    reserveAmmo = Scr_GetInt(1);
    if (reserveAmmo < 0)
        Scr_ParamError(1u, "Ammo count must not be negative");
    altIndex = 0;
    if (Scr_GetNumParam() > 2)
    {
        altIndex = Scr_GetInt(2);
        if (altIndex >= 2)
        {
            v1 = va("Value out of range.  Allowed values: 0 to %i", 2);
            Scr_ParamError(2u, v1);
        }
    }
    v2 = itemEnt->item[altIndex].index % 128;
    if (v2 > 0)
    {
        weapDef = BG_GetWeaponDef(v2);
        if (weapDef->iClipSize < 0)
            MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 7335, 0, "%s", "weapDef->iClipSize >= 0");
        if (weapDef->iClipSize < clipAmmo)
            v3.intValue = weapDef->iClipSize;
        else
            v3.intValue = clipAmmo;
        itemEnt->item[altIndex].ammoCount = reserveAmmo;
        itemEnt->item[altIndex].clipAmmoCount = v3.intValue;
    }
}

void __cdecl Scr_SetOrigin(gentity_s *ent, int32_t i)
{
    float org[3]; // [esp+0h] [ebp-Ch] BYREF

    Scr_GetVector(0, org);
    G_SetOrigin(ent, org);
    if (ent->r.linked)
        SV_LinkEntity(ent);
}

void __cdecl Scr_SetAngles(gentity_s *ent, int32_t i)
{
    float angles[3]; // [esp+0h] [ebp-Ch] BYREF

    Scr_GetVector(0, angles);
    G_SetAngle(ent, angles);
}

void __cdecl Scr_SetHealth(gentity_s *ent, int32_t i)
{
    int32_t health; // [esp+0h] [ebp-4h]

    health = Scr_GetInt(0);
    if (ent->client)
    {
        ent->health = health;
        ent->client->ps.stats[0] = health;
    }
    else
    {
        ent->maxHealth = health;
        ent->health = health;
    }
}

void __cdecl GScr_AddEntity(gentity_s *pEnt)
{
    if (pEnt)
        Scr_AddEntity(pEnt);
    else
        Scr_AddUndefined();
}

int32_t Scr_ParseGameTypeList_LoadObj()
{
    const char *v0; // eax
    const char *v1; // eax
    int32_t result; // eax
    char *qpath; // [esp+10h] [ebp-1430h]
    char *src; // [esp+14h] [ebp-142Ch]
    uint8_t buffer[1024]; // [esp+18h] [ebp-1428h] BYREF
    char *data_p; // [esp+418h] [ebp-1028h] BYREF
    char *s0; // [esp+41Ch] [ebp-1024h]
    char listbuf[4096]; // [esp+420h] [ebp-1020h] BYREF
    int32_t f; // [esp+1424h] [ebp-1Ch] BYREF
    uint32_t  v10; // [esp+1428h] [ebp-18h]
    int32_t v11; // [esp+142Ch] [ebp-14h]
    int32_t len; // [esp+1430h] [ebp-10h]
    int32_t i; // [esp+1434h] [ebp-Ch]
    char *dest; // [esp+1438h] [ebp-8h]
    int32_t FileList; // [esp+143Ch] [ebp-4h]

    memset((uint8_t *)g_scr_data.gametype.list, 0, sizeof(g_scr_data.gametype.list));
    v11 = 0;
    FileList = FS_GetFileList("maps/mp/gametypes", "gsc", FS_LIST_PURE_ONLY, listbuf, 4096);
    src = listbuf;
    for (i = 0; i < FileList; ++i)
    {
        v10 = strlen(src);
        if (*src == 95)
        {
            src += v10 + 1;
        }
        else
        {
            if (!I_stricmp(&src[v10 - 4], ".gsc"))
                src[v10 - 4] = 0;
            if (v11 == 32)
            {
                Com_Printf(23, "Too many game type scripts found! Only loading the first %i\n", 31);
                break;
            }
            dest = g_scr_data.gametype.list[v11].pszScript;
            I_strncpyz(dest, src, 64);
            //strlwr(dest);
            _strlwr(dest);
            qpath = va("maps/mp/gametypes/%s.txt", src);
            len = FS_FOpenFileByMode(qpath, &f, FS_READ);
            if (len > 0 && len < 1024)
            {
                FS_Read(buffer, len, f);
                data_p = (char *)buffer;
                s0 = (char *)Com_Parse((const char **)&data_p);
                I_strncpyz(dest + 64, s0, 64);
                s0 = (char *)Com_Parse((const char **)&data_p);
                *((_DWORD *)dest + 32) = s0 && !I_stricmp(s0, "team");
            }
            else
            {
                if (len > 0)
                {
                    v1 = va("maps/mp/gametypes/%s.txt", src);
                    Com_PrintWarning(23, "WARNING: GameType description file %s is too big to load.\n", v1);
                }
                else
                {
                    v0 = va("maps/mp/gametypes/%s.txt", src);
                    Com_PrintWarning(23, "WARNING: Could not load GameType description file %s for gametype %s\n", v0, src);
                }
                I_strncpyz(dest + 64, dest, 64);
                *((_DWORD *)dest + 32) = 0;
            }
            ++v11;
            if (len > 0)
                FS_FCloseFile(f);
            src += v10 + 1;
        }
    }
    result = v11;
    g_scr_data.gametype.iNumGameTypes = v11;
    return result;
}

void __cdecl Scr_ParseGameTypeList()
{
    if (IsFastFileLoad())
        Scr_ParseGameTypeList_FastFile();
    else
        Scr_ParseGameTypeList_LoadObj();
}

XAssetHeader Scr_ParseGameTypeList_FastFile()
{
    XAssetHeader result; // eax
    const char *v1; // eax
    const char *v2; // eax
    int32_t v3; // [esp+0h] [ebp-44h]
    char *fullname; // [esp+1Ch] [ebp-28h]
    RawFile *rawfile; // [esp+20h] [ebp-24h]
    const char *pszFileName; // [esp+24h] [ebp-20h]
    const char *pBuffParse; // [esp+28h] [ebp-1Ch] BYREF
    const char *pToken; // [esp+2Ch] [ebp-18h]
    int32_t iNumGameTypes; // [esp+30h] [ebp-14h]
    int32_t iFileLength; // [esp+34h] [ebp-10h]
    RawFile *gametypesFile; // [esp+38h] [ebp-Ch]
    const char *gametypesBuf; // [esp+3Ch] [ebp-8h] BYREF
    gameTypeScript_t *pGameType; // [esp+40h] [ebp-4h]

    memset((uint8_t *)g_scr_data.gametype.list, 0, sizeof(g_scr_data.gametype.list));
    iNumGameTypes = 0;
    result.xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "maps/mp/gametypes/_gametypes.txt").xmodelPieces;
    gametypesFile = (RawFile *)result.xmodelPieces;
    if (result.xmodelPieces)
    {
        gametypesBuf = gametypesFile->buffer;
        while (1)
        {
            result.xmodelPieces = (XModelPieces *)Com_Parse(&gametypesBuf);
            pszFileName = (const char *)result.xmodelPieces;
            if (!gametypesBuf)
                break;
            if (iNumGameTypes == 32)
            {
                Com_Printf(23, "Too many game type scripts found! Only loading the first %i\n", 31);
                break;
            }
            pGameType = &g_scr_data.gametype.list[iNumGameTypes];
            I_strncpyz(pGameType->pszScript, (char *)result.xmodelPieces, 64);
            I_strlwr(pGameType->pszScript);
            fullname = va("maps/mp/gametypes/%s.txt", pszFileName);
            rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fullname).rawfile;
            if (rawfile)
                v3 = strlen(rawfile->buffer);
            else
                v3 = 0;
            iFileLength = v3;
            if (v3 > 0 && iFileLength < 1024)
            {
                if (!rawfile)
                    MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 8047, 0, "%s", "rawfile");
                pBuffParse = rawfile->buffer;
                pToken = (const char *)Com_Parse(&pBuffParse);
                I_strncpyz(pGameType->pszName, (char *)pToken, 64);
                pToken = (const char *)Com_Parse(&pBuffParse);
                pGameType->bTeamBased = pToken && !I_stricmp(pToken, "team");
            }
            else
            {
                if (iFileLength > 0)
                {
                    v2 = va("maps/mp/gametypes/%s.txt", pszFileName);
                    Com_PrintWarning(23, "WARNING: GameType description file %s is too big to load.\n", v2);
                }
                else
                {
                    v1 = va("maps/mp/gametypes/%s.txt", pszFileName);
                    Com_PrintWarning(
                        23,
                        "WARNING: Could not load GameType description file %s for gametype %s\n",
                        v1,
                        pszFileName);
                }
                I_strncpyz(pGameType->pszName, pGameType->pszScript, 64);
                pGameType->bTeamBased = 0;
            }
            ++iNumGameTypes;
        }
    }
    g_scr_data.gametype.iNumGameTypes = iNumGameTypes;
    return result;
}

char *__cdecl Scr_GetGameTypeNameForScript(const char *pszGameTypeScript)
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_scr_data.gametype.iNumGameTypes; ++i)
    {
        if (!I_stricmp(g_scr_data.gametype.list[i].pszScript, pszGameTypeScript))
            return g_scr_data.gametype.list[i].pszName;
    }
    return 0;
}

bool __cdecl Scr_IsValidGameType(const char *pszGameType)
{
    return Scr_GetGameTypeNameForScript(pszGameType) != 0;
}

void __cdecl Scr_LoadGameType()
{
    uint16_t t; // [esp+0h] [ebp-4h]

    if (!g_scr_data.gametype.main)
        MyAssertHandler(".\\game_mp\\g_scr_main_mp.cpp", 8108, 0, "%s", "g_scr_data.gametype.main");
    t = Scr_ExecThread(g_scr_data.gametype.main, 0);
    Scr_FreeThread(t);
}

void __cdecl Scr_StartupGameType()
{
    uint16_t callback; // [esp+0h] [ebp-4h]

    callback = Scr_ExecThread(g_scr_data.gametype.startupgametype, 0);
    Scr_FreeThread(callback);
}

void __cdecl Scr_PlayerConnect(gentity_s *self)
{
    uint16_t callback; // [esp+0h] [ebp-4h]

    callback = Scr_ExecEntThread(self, g_scr_data.gametype.playerconnect, 0);
    Scr_FreeThread(callback);
}

void __cdecl Scr_PlayerDisconnect(gentity_s *self)
{
    uint16_t callback; // [esp+0h] [ebp-4h]

    callback = Scr_ExecEntThread(self, g_scr_data.gametype.playerdisconnect, 0);
    Scr_FreeThread(callback);
}

void __cdecl Scr_PlayerDamage(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int32_t damage,
    int32_t dflags,
    uint32_t  meansOfDeath,
    uint32_t  iWeapon,
    const float *vPoint,
    const float *vDir,
    hitLocation_t hitLoc,
    int32_t timeOffset)
{
    uint16_t HitLocationString; // ax
    WeaponDef *WeaponDef; // eax
    uint16_t callback; // [esp+0h] [ebp-4h]

    Scr_AddInt(timeOffset);
    HitLocationString = G_GetHitLocationString(hitLoc);
    Scr_AddConstString(HitLocationString);
    GScr_AddVector(vDir);
    GScr_AddVector(vPoint);
    WeaponDef = BG_GetWeaponDef(iWeapon);
    Scr_AddString((char *)WeaponDef->szInternalName);
    if (meansOfDeath < 0x10)
        Scr_AddConstString(*modNames[meansOfDeath]);
    else
        Scr_AddString("badMOD");
    Scr_AddInt(dflags);
    Scr_AddInt(damage);
    GScr_AddEntity(attacker);
    GScr_AddEntity(inflictor);
    callback = Scr_ExecEntThread(self, g_scr_data.gametype.playerdamage, 0xAu);
    Scr_FreeThread(callback);
}

void __cdecl Scr_PlayerKilled(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int32_t damage,
    uint32_t  meansOfDeath,
    uint32_t  iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int32_t psTimeOffset,
    int32_t deathAnimDuration)
{
    uint16_t HitLocationString; // ax
    WeaponDef *WeaponDef; // eax
    uint16_t callback; // [esp+0h] [ebp-4h]

    Scr_AddInt(deathAnimDuration);
    Scr_AddInt(psTimeOffset);
    HitLocationString = G_GetHitLocationString(hitLoc);
    Scr_AddConstString(HitLocationString);
    GScr_AddVector(vDir);
    WeaponDef = BG_GetWeaponDef(iWeapon);
    Scr_AddString((char *)WeaponDef->szInternalName);
    if (meansOfDeath < 0x10)
        Scr_AddConstString(*modNames[meansOfDeath]);
    else
        Scr_AddString("badMOD");
    Scr_AddInt(damage);
    GScr_AddEntity(attacker);
    GScr_AddEntity(inflictor);
    callback = Scr_ExecEntThread(self, g_scr_data.gametype.playerkilled, 9u);
    Scr_FreeThread(callback);
}

void __cdecl Scr_PlayerLastStand(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int32_t damage,
    uint32_t  meansOfDeath,
    uint32_t  iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int32_t psTimeOffset)
{
    uint16_t HitLocationString; // ax
    WeaponDef *WeaponDef; // eax
    uint16_t callback; // [esp+0h] [ebp-4h]

    Scr_AddInt(0);
    Scr_AddInt(psTimeOffset);
    HitLocationString = G_GetHitLocationString(hitLoc);
    Scr_AddConstString(HitLocationString);
    GScr_AddVector(vDir);
    WeaponDef = BG_GetWeaponDef(iWeapon);
    Scr_AddString((char *)WeaponDef->szInternalName);
    if (meansOfDeath < 0x10)
        Scr_AddConstString(*modNames[meansOfDeath]);
    else
        Scr_AddString("badMOD");
    Scr_AddInt(damage);
    GScr_AddEntity(attacker);
    GScr_AddEntity(inflictor);
    callback = Scr_ExecEntThread(self, g_scr_data.gametype.playerlaststand, 9u);
    Scr_FreeThread(callback);
}

void __cdecl Scr_VoteCalled(gentity_s *self, char *command, char *param1, char *param2)
{
    Scr_AddString(param2);
    Scr_AddString(param1);
    Scr_AddString(command);
    Scr_Notify(self, scr_const.call_vote, 3u);
}

void __cdecl Scr_PlayerVote(gentity_s *self, char *option)
{
    Scr_AddString(option);
    Scr_Notify(self, scr_const.vote, 1u);
}

void GScr_GetAnimLength()
{
    float value; // [esp+0h] [ebp-10h]
    scr_anim_s anim; // [esp+8h] [ebp-8h]
    XAnim_s *anims; // [esp+Ch] [ebp-4h]

    anim = Scr_GetAnim(0, 0);
    anims = Scr_GetAnims(anim.tree);
    if (!XAnimIsPrimitive(anims, anim.index))
        Scr_ParamError(0, "non-primitive animation has no concept of length");
    value = XAnimGetLength(anims, anim.index);
    Scr_AddFloat(value);
}

void(__cdecl *__cdecl Scr_GetFunction(const char **pName, int *type))()
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0xCD; ++i)
    {
        if (!strcmp(*pName, functions[i].actionString))
        {
            *pName = functions[i].actionString;
            *type = functions[i].type;
            return functions[i].actionFunc;
        }
    }
    return 0;
}

void(__cdecl *__cdecl BuiltIn_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x52; ++i)
    {
        if (!strcmp(*pName, methods_2[i].actionString))
        {
            *pName = methods_2[i].actionString;
            *type = methods_2[i].type;
            return methods_2[i].actionFunc;
        }
    }
    return 0;
}

void(__cdecl *__cdecl Scr_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    void(__cdecl * method)(scr_entref_t); // [esp+0h] [ebp-4h]

    *type = 0;

    method = Player_GetMethod(pName);
    if (method)
        return method;
    method = ScriptEnt_GetMethod(pName);
    if (method)
        return method;

    method = HudElem_GetMethod(pName);
    if (method)
        return method;

    method = Helicopter_GetMethod(pName);
    if (method)
        return method;

    return BuiltIn_GetMethod(pName, type);
}

void __cdecl GScr_Shutdown()
{
    if (level.cachedTagMat.name)
        Scr_SetString(&level.cachedTagMat.name, 0);
}
