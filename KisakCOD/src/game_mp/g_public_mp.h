#pragma once
#include <xanim/xanim.h>
#include <qcommon/msg_mp.h>
#include <bgame/bg_public.h>
#include <script/scr_variable.h>
#include "g_main_mp.h"
#include <game/game_public.h>
#include <script/scr_const.h>


struct BuiltinFunctionDef // sizeof=0xC
{
    const char *actionString;
    void(__cdecl *actionFunc)();
    int type;
};

struct BuiltinMethodDef // sizeof=0xC
{                                       // ...
    const char *actionString;           // ...
    void(__cdecl *actionFunc)(scr_entref_t); // ...
    int type;                           // ...
};

BuiltinMethodDef methods_2[82] =
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
  { "playrumbleonentity", (void(__cdecl *)(scr_entref_t))&CL_ResetStats_f, 0},
  { "playrumblelooponentity", (void(__cdecl *)(scr_entref_t)) &CL_ResetStats_f, 0 },
  { "stoprumble", (void(__cdecl *)(scr_entref_t)) &CL_ResetStats_f, 0 },
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
  { "sendleaderboards", (void(__cdecl *)(scr_entref_t)) &CL_ResetStats_f, 0 },
  { "setmovespeedscale", &ScrCmd_SetMoveSpeedScale, 0 },
  { "logstring", (void(__cdecl *)(scr_entref_t)) &CL_ResetStats_f, 0 },
  { "missile_settarget", &GScr_MissileSetTarget, 0 },
  { "isonladder", &GScr_IsOnLadder, 0 },
  { "ismantling", &GScr_IsMantling, 0 },
  { "startragdoll", &GScr_StartRagdoll, 0 },
  { "isragdoll", &GScr_IsRagdoll, 0 },
  { "getcorpseanim", &GScr_GetCorpseAnim, 0 },
  { "itemweaponsetammo", &ScrCmd_ItemWeaponSetAmmo, 0 }
}; // idb

static unsigned __int16 *modNames[16] =
{
    &scr_const.mod_unknown,
    &scr_const.mod_pistol_bullet,
    &scr_const.mod_rifle_bullet,
    &scr_const.mod_grenade,
    &scr_const.mod_grenade_splash,
    &scr_const.mod_projectile,
    &scr_const.mod_projectile_splash,
    &scr_const.mod_melee,
    &scr_const.mod_head_shot,
    &scr_const.mod_crush,
    &scr_const.mod_telefrag,
    &scr_const.mod_falling,
    &scr_const.mod_suicide,
    &scr_const.mod_trigger_hurt,
    &scr_const.mod_explosive,
    &scr_const.mod_impact
};

struct gameTypeScript_t // sizeof=0x84
{                                       // ...
    char pszScript[64];
    char pszName[64];                   // ...
    int bTeamBased;
};

struct scr_data_t_s // sizeof=0x10A8
{                                       // ...
    int main;                           // ...
    int startupgametype;                // ...
    int playerconnect;                  // ...
    int playerdisconnect;               // ...
    int playerdamage;                   // ...
    int playerkilled;                   // ...
    int votecalled;
    int playervote;
    int playerlaststand;                // ...
    int iNumGameTypes;                  // ...
    gameTypeScript_t list[32];          // ...
};
struct __declspec(align(4)) corpseInfo_t // sizeof=0x4DC
{                                       // ...
    XAnimTree_s *tree;                  // ...
    int entnum;                         // ...
    int time;
    clientInfo_t ci;
    bool falling;
    // padding byte
    // padding byte
    // padding byte
};
struct scr_data_t // sizeof=0x379C
{                                       // ...
    int levelscript;                    // ...
    int gametypescript;
    scr_data_t_s gametype; // ...
    int delete_;                        // ...
    int initstructs;                    // ...
    int createstruct;                   // ...
    corpseInfo_t playerCorpseInfo[8];   // ...
};


// g_active_mp
void __cdecl P_DamageFeedback(gentity_s *player);
void __cdecl G_SetClientSound(gentity_s *ent);
void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm);
void __cdecl G_TouchTriggers(gentity_s *ent);
void __cdecl SpectatorThink(gentity_s *ent, usercmd_s *ucmd);
int __cdecl ClientInactivityTimer(gclient_s *client);
void __cdecl ClientIntermissionThink(gentity_s *ent);
void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex);
void __cdecl HandleClientEvent(gclient_s *client, gentity_s *ent, int event, int eventParm);
void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt);
bool __cdecl IsLiveGrenade(gentity_s *ent);
void __cdecl ClientEvents(gentity_s *ent, int oldEventSequence);
void __cdecl G_SetLastServerTime(int clientNum, int lastServerTime);
void __cdecl G_SetClientContents(gentity_s *pEnt);
void __cdecl ClientThink_real(gentity_s *ent, usercmd_s *ucmd);
void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap);
void __cdecl G_AddPlayerMantleBlockage(float *endPos, int duration, pmove_t *pm);
void __cdecl ClientThink(int clientNum);
void __cdecl G_RunClient(gentity_s *ent);
void __cdecl IntermissionClientEndFrame(gentity_s *ent);
void __cdecl SpectatorClientEndFrame(gentity_s *ent);
bool __cdecl G_ClientCanSpectateTeam(gclient_s *client, team_t team);
int __cdecl GetFollowPlayerState(int clientNum, playerState_s *ps);
int __cdecl StuckInClient(gentity_s *self);
void __cdecl G_PlayerController(const gentity_s *self, int *partBits);
void __cdecl ClientEndFrame(gentity_s *ent);
int __cdecl G_UpdateClientInfo(gentity_s *ent);
void __cdecl G_PlayerEvent(int clientNum, int event);
int __cdecl GetFollowPlayerState(int clientNum, playerState_s *ps);



// g_client_mp
void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle);
void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin);
void __cdecl ClientUserinfoChanged(unsigned int clientNum);
void __cdecl ClientCleanName(const char *in, char *out, int outSize);
char *__cdecl ClientConnect(unsigned int clientNum, unsigned __int16 scriptPersId);
void __cdecl ClientClearFields(gclient_s *client);
void __cdecl ClientBegin(int clientNum);
void __cdecl ClientSpawn(gentity_s *ent, const float *spawn_origin, const float *spawn_angles);
void __cdecl ClientDisconnect(int clientNum);
unsigned int __cdecl G_GetNonPVSPlayerInfo(gentity_s *pSelf, float *vPosition, int iLastUpdateEnt);



// g_client_script_cmd_mp
enum meansOfDeath_t : __int32
{                                       // ...
    MOD_UNKNOWN = 0x0,
    MOD_PISTOL_BULLET = 0x1,
    MOD_RIFLE_BULLET = 0x2,
    MOD_GRENADE = 0x3,
    MOD_GRENADE_SPLASH = 0x4,
    MOD_PROJECTILE = 0x5,
    MOD_PROJECTILE_SPLASH = 0x6,
    MOD_MELEE = 0x7,
    MOD_HEAD_SHOT = 0x8,
    MOD_CRUSH = 0x9,
    MOD_TELEFRAG = 0xA,
    MOD_FALLING = 0xB,
    MOD_SUICIDE = 0xC,
    MOD_TRIGGER_HURT = 0xD,
    MOD_EXPLOSIVE = 0xE,
    MOD_IMPACT = 0xF,
    MOD_NUM = 0x10,
};
void __cdecl PlayerCmd_giveWeapon(scr_entref_t entref);
void __cdecl G_InitializeAmmo(gentity_s *pSelf, int weaponIndex, unsigned __int8 weaponModel, int hadWeapon);
int __cdecl G_GetNeededStartAmmo(gentity_s *pSelf, WeaponDef *weapDef);
void __cdecl PlayerCmd_takeWeapon(scr_entref_t entref);
void __cdecl PlayerCmd_takeAllWeapons(scr_entref_t entref);
void __cdecl PlayerCmd_getCurrentWeapon(scr_entref_t entref);
bool __cdecl ClientPlaying(gentity_s *pSelf);
void __cdecl PlayerCmd_getCurrentOffhand(scr_entref_t entref);
void __cdecl PlayerCmd_setOffhandSecondaryClass(scr_entref_t entref);
void __cdecl PlayerCmd_getOffhandSecondaryClass(scr_entref_t entref);
void __cdecl PlayerCmd_hasWeapon(scr_entref_t entref);
void __cdecl PlayerCmd_switchToWeapon(scr_entref_t entref);
void __cdecl PlayerCmd_switchToOffhand(scr_entref_t entref);
void __cdecl PlayerCmd_giveStartAmmo(scr_entref_t entref);
void __cdecl PlayerCmd_giveMaxAmmo(scr_entref_t entref);
void __cdecl PlayerCmd_getFractionStartAmmo(scr_entref_t entref);
void __cdecl PlayerCmd_getFractionMaxAmmo(scr_entref_t entref);
void __cdecl PlayerCmd_setOrigin(scr_entref_t entref);
void __cdecl PlayerCmd_GetVelocity(scr_entref_t entref);
void __cdecl PlayerCmd_setAngles(scr_entref_t entref);
void __cdecl PlayerCmd_getAngles(scr_entref_t entref);
void __cdecl PlayerCmd_useButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_attackButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_adsButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_meleeButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_fragButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref);
void __cdecl PlayerCmd_buttonPressedDEVONLY(scr_entref_t entref);
void __cdecl PlayerCmd_playerADS(scr_entref_t entref);
void __cdecl PlayerCmd_isOnGround(scr_entref_t entref);
void __cdecl PlayerCmd_pingPlayer(scr_entref_t entref);
void __cdecl PlayerCmd_SetViewmodel(scr_entref_t entref);
void __cdecl PlayerCmd_GetViewmodel(scr_entref_t entref);
void __cdecl PlayerCmd_showScoreboard(scr_entref_t entref);
void __cdecl PlayerCmd_setSpawnWeapon(scr_entref_t entref);
void __cdecl PlayerCmd_dropItem(scr_entref_t entref);
void __cdecl PlayerCmd_finishPlayerDamage(scr_entref_t entref);
bool __cdecl IsBulletImpactMOD(meansOfDeath_t mod);
void __cdecl PlayerCmd_Suicide(scr_entref_t entref);
void __cdecl PlayerCmd_OpenMenu(scr_entref_t entref);
void __cdecl PlayerCmd_OpenMenuNoMouse(scr_entref_t entref);
void __cdecl PlayerCmd_CloseMenu(scr_entref_t entref);
void __cdecl PlayerCmd_CloseInGameMenu(scr_entref_t entref);
void __cdecl PlayerCmd_SetWeaponAmmoClip(scr_entref_t entref);
void __cdecl PlayerCmd_SetWeaponAmmoStock(scr_entref_t entref);
void __cdecl PlayerCmd_GetWeaponAmmoClip(scr_entref_t entref);
void __cdecl PlayerCmd_GetWeaponAmmoStock(scr_entref_t entref);
void __cdecl PlayerCmd_AnyAmmoForWeaponModes(scr_entref_t entref);
void __cdecl iclientprintln(scr_entref_t entref);
void __cdecl iclientprintlnbold(scr_entref_t entref);
void __cdecl PlayerCmd_spawn(scr_entref_t entref);
void __cdecl PlayerCmd_setEnterTime(scr_entref_t entref);
void __cdecl BodyEnd(gentity_s *ent);
void __cdecl PlayerCmd_ClonePlayer(scr_entref_t entref);
void __cdecl PlayerCmd_SetClientDvar(scr_entref_t entref);
void __cdecl PlayerCmd_SetClientDvars(scr_entref_t entref);
void __cdecl PlayerCmd_IsTalking(scr_entref_t entref);
void __cdecl PlayerCmd_FreezeControls(scr_entref_t entref);
void __cdecl PlayerCmd_DisableWeapons(scr_entref_t entref);
void __cdecl PlayerCmd_EnableWeapons(scr_entref_t entref);
void __cdecl PlayerCmd_SetReverb(scr_entref_t entref);
void __cdecl PlayerCmd_DeactivateReverb(scr_entref_t entref);
void __cdecl PlayerCmd_SetChannelVolumes(scr_entref_t entref);
void __cdecl PlayerCmd_DeactivateChannelVolumes(scr_entref_t entref);
void __cdecl ScrCmd_PlayLocalSound(scr_entref_t entref);
void __cdecl ScrCmd_StopLocalSound(scr_entref_t entref);
void __cdecl PlayerCmd_SayAll(scr_entref_t entref);
void __cdecl PlayerCmd_SayTeam(scr_entref_t entref);
void __cdecl PlayerCmd_AllowADS(scr_entref_t entref);
void __cdecl PlayerCmd_AllowJump(scr_entref_t entref);
void __cdecl PlayerCmd_AllowSprint(scr_entref_t entref);
void __cdecl PlayerCmd_SetSpreadOverride(scr_entref_t entref);
void __cdecl PlayerCmd_ResetSpreadOverride(scr_entref_t entref);
void __cdecl PlayerCmd_AllowSpectateTeam(scr_entref_t entref);
void __cdecl PlayerCmd_GetGuid(scr_entref_t entref);
void __cdecl PlayerCmd_GetXuid(scr_entref_t entref);
void __cdecl PlayerCmd_BeginLocationSelection(scr_entref_t entref);
void __cdecl PlayerCmd_EndLocationSelection(scr_entref_t entref);
void __cdecl PlayerCmd_SetActionSlot(scr_entref_t entref);
void __cdecl PlayerCmd_GetWeaponsList(scr_entref_t entref);
void __cdecl PlayerCmd_GetWeaponsListPrimaries(scr_entref_t entref);
void __cdecl PlayerCmd_SetPerk(scr_entref_t entref);
void __cdecl BG_SetPerk(int *perks, unsigned int perkIndex);
void __cdecl PlayerCmd_HasPerk(scr_entref_t entref);
void __cdecl PlayerCmd_UnsetPerk(scr_entref_t entref);
void __cdecl BG_UnsetPerk(int *perks, unsigned int perkIndex);
void __cdecl PlayerCmd_ClearPerks(scr_entref_t entref);
void __cdecl PlayerCmd_UpdateScores(scr_entref_t entref);
void __cdecl PlayerCmd_UpdateDMScores(scr_entref_t entref);
void __cdecl PlayerCmd_SetRank(scr_entref_t entref);
void(__cdecl *__cdecl Player_GetMethod(const char **pName))(scr_entref_t);



// g_cmds_mp
void __cdecl SendScoreboard(gentity_s *ent);
void __cdecl Cmd_Score_f(gentity_s *ent);
int __cdecl CheatsOk(gentity_s *ent);
char *__cdecl ConcatArgs(int start);
int __cdecl SV_Cmd_Argc();
void __cdecl G_setfog(const char *fogstring);
void __cdecl Cmd_Give_f(gentity_s *ent);
void __cdecl Cmd_Take_f(gentity_s *ent);
void __cdecl Cmd_God_f(gentity_s *ent);
void __cdecl Cmd_DemiGod_f(gentity_s *ent);
void __cdecl Cmd_Notarget_f(gentity_s *ent);
void __cdecl Cmd_Noclip_f(gentity_s *ent);
void __cdecl Cmd_UFO_f(gentity_s *ent);
void __cdecl Cmd_Kill_f(gentity_s *ent);
void __cdecl StopFollowing(gentity_s *ent);
int __cdecl Cmd_FollowCycle_f(gentity_s *ent, int dir);
bool __cdecl G_IsPlaying(gentity_s *ent);
void __cdecl G_Say(gentity_s *ent, gentity_s *target, int mode, char *chatText);
void __cdecl G_SayTo(
    gentity_s *ent,
    gentity_s *other,
    int mode,
    int color,
    const char *teamString,
    const char *cleanname,
    const char *message);
void __cdecl Cmd_Where_f(gentity_s *ent);
void __cdecl Cmd_CallVote_f(gentity_s *ent);
void __cdecl Cmd_Vote_f(gentity_s *ent);
void __cdecl Cmd_SetViewpos_f(gentity_s *ent);
void __cdecl Cmd_EntityCount_f();
void __cdecl Cmd_MenuResponse_f(gentity_s *pEnt);
void __cdecl ClientCommand(int clientNum);
void __cdecl Cmd_Say_f(gentity_s *ent, int mode, int arg0);
void Cmd_PrintEntities_f();
void Cmd_VisionSetNaked_f();
const char *__cdecl SV_Cmd_Argv(int argIndex);
void Cmd_VisionSetNight_f();



// g_combat_mp
void __cdecl TRACK_g_combat();
void __cdecl G_ParseHitLocDmgTable();
void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker);
int __cdecl G_MeansOfDeathFromScriptParam(unsigned int scrParam);
void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int psTimeOffset);
void __cdecl DeathGrenadeDrop(gentity_s *self, int meansOfDeath);
double __cdecl G_GetWeaponHitLocationMultiplier(hitLocation_t hitLoc, unsigned int weapon);
void __cdecl G_DamageClient(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int damage,
    int dflags,
    unsigned int mod,
    unsigned int weapon,
    hitLocation_t hitLoc,
    int timeOffset);
unsigned int __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent);
void __cdecl G_Damage(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    float *dir,
    float *point,
    int damage,
    int dFlags,
    int mod,
    unsigned int weapon,
    hitLocation_t hitLoc,
    unsigned int modelIndex,
    unsigned int partName,
    int timeOffset);
void __cdecl DamageNotify(
    unsigned __int16 notify,
    gentity_s *targ,
    gentity_s *attacker,
    float *dir,
    float *point,
    int damage,
    int mod,
    int dFlags,
    unsigned int modelIndex,
    unsigned int partName);
double __cdecl CanDamage(
    gentity_s *targ,
    gentity_s *inflictor,
    float *centerPos,
    float coneAngleCos,
    float *coneDirection,
    int contentMask);
void __cdecl G_FlashbangBlast(float *origin, float radius_max, float radius_min, gentity_s *attacker, team_t team);
void __cdecl GetEntListForRadius(
    const float *origin,
    float radius_max,
    float radius_min,
    int *entList,
    int *entListCount);
void __cdecl FlashbangBlastEnt(
    gentity_s *ent,
    float *blastOrigin,
    float radius_max,
    float radius_min,
    gentity_s *attacker,
    team_t team);
void __cdecl Vec3NormalizeFast(float *v);
double __cdecl EntDistToPoint(const float *origin, gentity_s *ent);
void __cdecl AddScrTeamName(team_t team);
bool __cdecl G_WithinDamageRadius(const float *damageOrigin, float radiusSquared, gentity_s *ent);
double __cdecl G_GetRadiusDamageDistanceSquared(const float *damageOrigin, gentity_s *ent);
int __cdecl G_RadiusDamage(
    float *origin,
    gentity_s *inflictor,
    gentity_s *attacker,
    float fInnerDamage,
    float fOuterDamage,
    float radius,
    float coneAngleCos,
    float *coneDirection,
    gentity_s *ignore,
    int mod,
    unsigned int weapon);
unsigned __int16 __cdecl G_GetHitLocationString(hitLocation_t hitLoc);
int __cdecl G_GetHitLocationIndexFromString(unsigned __int16 sString);




// g_misc_mp
void __cdecl SP_info_notnull(gentity_s *self);
void __cdecl SP_light(gentity_s *self);
void __cdecl Byte4PackRgba(const float *from, unsigned __int8 *to);
void __cdecl TeleportPlayer(gentity_s *player, float *origin, float *angles);
void __cdecl G_InitTurrets();
void __cdecl G_ClientStopUsingTurret(gentity_s *self);
void __cdecl turret_think_client(gentity_s *self);
void __cdecl turret_track(gentity_s *self, gentity_s *other);
void __cdecl G_PlayerTurretPositionAndBlend(gentity_s *ent, gentity_s *pTurretEnt);
void __cdecl turret_clientaim(gentity_s *self, gentity_s *other);
void __cdecl turret_shoot_internal(gentity_s *self, gentity_s *other);
void __cdecl Fire_Lead(gentity_s *ent, gentity_s *activator);
void __cdecl Turret_FillWeaponParms(gentity_s *ent, gentity_s *activator, weaponParms *wp);
void __cdecl turret_UpdateSound(gentity_s *self);
void __cdecl turret_think(gentity_s *self);
int __cdecl turret_ReturnToDefaultPos(gentity_s *self, int bManned);
int __cdecl turret_UpdateTargetAngles(gentity_s *self, float *desiredAngles, int bManned);
void __cdecl turret_think_init(gentity_s *self);
void __cdecl turret_controller(const gentity_s *self, int *partBits);
void __cdecl G_FreeTurret(gentity_s *self);
bool __cdecl G_IsTurretUsable(gentity_s *self, gentity_s *owner);
bool __cdecl turret_behind(gentity_s *self, gentity_s *other);
void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s *activator);
void __cdecl G_SpawnTurret(gentity_s *self, const char *weaponinfoname);
void __cdecl SP_turret(gentity_s *self);




// g_player_corpse_mp
struct __declspec(align(4)) corpseInfo_t // sizeof=0x4DC
{                                       // ...
    XAnimTree_s *tree;                  // ...
    int entnum;                         // ...
    int time;
    clientInfo_t ci;
    bool falling;
    // padding byte
    // padding byte
    // padding byte
};
int __cdecl G_GetPlayerCorpseIndex(gentity_s *ent);
int __cdecl G_GetFreePlayerCorpseIndex();
void __cdecl PlayerCorpse_Free(gentity_s *ent);
void __cdecl G_RunCorpseMove(gentity_s *ent);
void __cdecl G_BounceCorpse(gentity_s *ent, corpseInfo_t *corpseInfo, trace_t *trace, float *endpos);
char __cdecl G_GetAnimDeltaForCorpse(gentity_s *ent, float *originChange);
void __cdecl G_RunCorpse(gentity_s *ent);
void __cdecl G_RunCorpseAnimate(gentity_s *ent);



// g_scr_helicopter
void __cdecl CMD_Heli_FreeHelicopter(scr_entref_t entref);
void __cdecl CMD_Heli_SetDamageStage(scr_entref_t entref);
void(__cdecl *__cdecl Helicopter_GetMethod(const char **pName))(scr_entref_t);
void __cdecl Helicopter_RegisterDvars();
void __cdecl G_SpawnHelicopter(gentity_s *ent, gentity_s *owner, const char *vehicleInfoName, char *modelName);
void __cdecl Heli_InitFirstThink(gentity_s *pSelf);
void __cdecl Helicopter_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int damage,
    const float *point,
    const int mod,
    const float *dir,
    const hitLocation_t hitLoc,
    const int weaponIdx);
void __cdecl Helicopter_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    const int damage,
    const int mod,
    const int weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int psTimeOffset);
void __cdecl Helicopter_Controller(const gentity_s *pSelf, int *partBits);
void __cdecl Helicopter_Think(gentity_s *ent);




// g_scr_main_mp
HashEntry_unnamed_type_u __cdecl GScr_AllocString(const char *s);
void __cdecl TRACK_g_scr_main();
void __cdecl Scr_LoadLevel();
void __cdecl GScr_LoadGameTypeScript();
int __cdecl GScr_LoadScriptAndLabel(const char *filename, const char *label, int bEnforceExists);
void __cdecl GScr_LoadScripts();
int GScr_LoadLevelScript();
void GScr_PostLoadScripts();
void __cdecl GScr_FreeScripts();
void __cdecl ScrCmd_GetClanId(scr_entref_t entref);
void __cdecl ScrCmd_GetClanName(scr_entref_t entref);
void GScr_CreatePrintChannel();
void GScr_printChannelSet();
void print();
void println();
void __cdecl Scr_LocalizationError(unsigned int iParm, const char *pszErrorMessage);
void __cdecl Scr_ConstructMessageString(
    int firstParmIndex,
    int lastParmIndex,
    const char *errorContext,
    char *string,
    unsigned int stringLimit);
void __cdecl Scr_ValidateLocalizedStringRef(unsigned int parmIndex, const char *token, int tokenLen);
void __cdecl Scr_MakeGameMessage(int iClientNum, const char *pszCmd);
void __cdecl Scr_VerifyWeaponIndex(int weaponIndex, const char *weaponName);
void iprintln();
void iprintlnbold();
void GScr_print3d();
void GScr_line();
void assertCmd();
void assertexCmd();
void assertmsgCmd();
void GScr_IsDefined();
void GScr_IsString();
void GScr_IsAlive();
void GScr_GetDvar();
void GScr_GetDvarInt();
void GScr_GetDvarFloat();
void GScr_SetDvar();
void GScr_GetTime();
void Scr_GetEntByNum();
void Scr_GetWeaponModel();
void __cdecl GScr_GetAmmoCount(scr_entref_t entref);
gentity_s *__cdecl GetPlayerEntity(scr_entref_t entref);
gentity_s *__cdecl GetEntity(scr_entref_t entref);
void GScr_GetAnimLength();
void GScr_AnimHasNotetrack();
void GScr_GetNotetrackTimes();
void GScr_GetBrushModelCenter();
void GScr_Spawn();
void GScr_SpawnPlane();
void GScr_SpawnTurret();
void GScr_SpawnHelicopter();
unsigned int GScr_PrecacheTurret();
void __cdecl ScrCmd_SetMoveSpeedScale(scr_entref_t entref);
void __cdecl ScrCmd_attach(scr_entref_t entref);
void __cdecl ScrCmd_detach(scr_entref_t entref);
void __cdecl ScrCmd_detachAll(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachSize(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachModelName(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachTagName(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachIgnoreCollision(scr_entref_t entref);
void __cdecl G_EntityStateSetPartBits(gentity_s *ent, const unsigned int *partBits);
void __cdecl G_EntityStateGetPartBits(const gentity_s *ent, unsigned int *partBits);
void __cdecl ScrCmd_hidepart(scr_entref_t entref);
void __cdecl ScrCmd_showpart(scr_entref_t entref);
void __cdecl ScrCmd_showallparts(scr_entref_t entref);
void __cdecl ScrCmd_LinkTo(scr_entref_t entref);
void __cdecl ScrCmd_Unlink(scr_entref_t entref);
void __cdecl ScrCmd_EnableLinkTo(scr_entref_t entref);
void __cdecl ScrCmd_GetOrigin(scr_entref_t entref);
void __cdecl ScrCmd_GetEye(scr_entref_t entref);
void __cdecl ScrCmd_UseBy(scr_entref_t entref);
void __cdecl ScrCmd_IsTouching(scr_entref_t entref);
void ScrCmd_SoundExists();
void __cdecl ScrCmd_PlaySound(scr_entref_t entref);
gentity_s *__cdecl StartScriptPlaySoundOnEnt(scr_entref_t entref);
void __cdecl ScrCmd_PlaySoundToTeam(scr_entref_t entref);
void __cdecl ScrCmd_PlaySoundToPlayer(scr_entref_t entref);
void __cdecl ScrCmd_PlaySoundAsMaster(scr_entref_t entref);
void __cdecl ScrCmd_PlayLoopSound(scr_entref_t entref);
void __cdecl ScrCmd_StopLoopSound(scr_entref_t entref);
void __cdecl ScrCmd_Delete(scr_entref_t entref);
void __cdecl ScrCmd_SetModel(scr_entref_t entref);
void __cdecl ScrCmd_GetNormalHealth(scr_entref_t entref);
void __cdecl ScrCmd_SetNormalHealth(scr_entref_t entref);
void __cdecl ScrCmd_Show(scr_entref_t entref);
void __cdecl ScrCmd_Hide(scr_entref_t entref);
void __cdecl ScrCmd_LaserOn(scr_entref_t entref);
void __cdecl ScrCmd_LaserOff(scr_entref_t entref);
void __cdecl ScrCmd_ShowToPlayer(scr_entref_t entref);
void __cdecl ScrCmd_SetContents(scr_entref_t entref);
void __cdecl ScrCmd_GetStance(scr_entref_t entref);
void __cdecl Scr_SetStableMissile(scr_entref_t entref);
void __cdecl GScr_SetCursorHint(scr_entref_t entref);
int __cdecl G_GetHintStringIndex(int *piIndex, char *pszString);
void __cdecl GScr_SetHintString(scr_entref_t entref);
void __cdecl GScr_UseTriggerRequireLookAt(scr_entref_t entref);
void __cdecl GScr_GetEntityNumber(scr_entref_t entref);
void __cdecl GScr_EnableGrenadeTouchDamage(scr_entref_t entref);
void __cdecl GScr_DisableGrenadeTouchDamage(scr_entref_t entref);
void __cdecl GScr_MissileSetTarget(scr_entref_t entref);
void __cdecl GScr_EnableAimAssist(scr_entref_t entref);
void __cdecl GScr_DisableAimAssist(scr_entref_t entref);
void __cdecl G_InitObjectives();
int __cdecl ObjectiveStateIndexFromString(objectiveState_t *piStateIndex, unsigned int stateString);
void __cdecl ClearObjective(objective_t *obj);
void Scr_Objective_Add();
void __cdecl ClearObjective_OnEntity(objective_t *obj);
void __cdecl SetObjectiveIcon(objective_t *obj, unsigned int paramNum);
void Scr_Objective_Delete();
void Scr_Objective_State();
void Scr_Objective_Icon();
void Scr_Objective_Position();
objective_t *Scr_Objective_OnEntity();
void Scr_Objective_Current();
void GScr_Objective_Team();
void GScr_LogPrint();
void GScr_WorldEntNumber();
int GScr_Obituary();
void GScr_positionWouldTelefrag();
void GScr_getStartTime();
void GScr_PrecacheMenu();
int __cdecl GScr_GetScriptMenuIndex(const char *pszMenu);
void GScr_PrecacheStatusIcon();
int __cdecl GScr_GetStatusIconIndex(const char *pszIcon);
void GScr_PrecacheHeadIcon();
int __cdecl GScr_GetHeadIconIndex(const char *pszIcon);
void GScr_WeaponClipSize();
void GScr_WeaponIsSemiAuto();
void GScr_WeaponIsBoltAction();
void GScr_WeaponType();
void GScr_WeaponClass();
void GScr_WeaponInventoryType();
void GScr_WeaponStartAmmo();
void GScr_WeaponMaxAmmo();
void GScr_WeaponAltWeaponName();
void GScr_WeaponFireTime();
void GScr_IsWeaponClipOnly();
void GScr_IsWeaponDetonationTimed();
void GScr_PrecacheLocationSelector();
int __cdecl GScr_GetLocSelIndex(const char *mtlName);
void Scr_BulletTrace();
void Scr_BulletTracePassed();
void __cdecl Scr_SightTracePassed();
void Scr_PhysicsTrace();
void Scr_PlayerPhysicsTrace();
void Scr_RandomInt();
void Scr_RandomFloat();
void Scr_RandomIntRange();
void Scr_RandomFloatRange();
void GScr_sin();
void GScr_cos();
void GScr_tan();
void GScr_asin();
void GScr_acos();
void GScr_atan();
void GScr_abs();
void GScr_min();
void GScr_max();
void GScr_floor();
void GScr_ceil();
void GScr_sqrt();
void GScr_CastInt();
void GScr_VectorFromLineToPoint();
void GScr_PointOnSegmentNearestToPoint();
void Scr_Distance();
void Scr_Distance2D();
void Scr_DistanceSquared();
void Scr_Length();
void Scr_LengthSquared();
void Scr_Closer();
void Scr_VectorDot();
void Scr_VectorNormalize();
void Scr_VectorToAngles();
void Scr_VectorLerp();
void Scr_AnglesToUp();
void Scr_AnglesToRight();
void Scr_AnglesToForward();
void Scr_CombineAngles();
void Scr_IsSubStr();
void Scr_GetSubStr();
void Scr_ToLower();
void Scr_StrTok();
void Scr_MusicPlay();
void Scr_MusicStop();
void Scr_SoundFade();
int Scr_PrecacheModel();
void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, const char *assetName);
void Scr_PrecacheShellShock();
void Scr_PrecacheItem();
int Scr_PrecacheShader();
char *Scr_PrecacheString();
void Scr_AmbientPlay();
void Scr_AmbientStop();
void Scr_GrenadeExplosionEffect();
void GScr_RadiusDamage();
void __cdecl GScr_RadiusDamageInternal(gentity_s *inflictor);
void __cdecl GScr_EntityRadiusDamage(scr_entref_t entref);
void __cdecl GScr_Detonate(scr_entref_t entref);
VariableUnion GScr_SetPlayerIgnoreRadiusDamage();
void __cdecl GScr_DamageConeTrace(scr_entref_t entref);
void __cdecl GScr_DamageConeTraceInternal(scr_entref_t entref, int contentMask);
void __cdecl GScr_SightConeTrace(scr_entref_t entref);
void GScr_GetMoveDelta();
void GScr_GetAngleDelta();
void GScr_GetNorthYaw();
void Scr_LoadFX();
void Scr_PlayFX();
void __cdecl Scr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles);
void __cdecl Scr_FxParamError(unsigned int paramIndex, const char *errorString, int fxId);
void Scr_PlayFXOnTag();
void Scr_PlayLoopedFX();
void Scr_SpawnFX();
void Scr_TriggerFX();
void Scr_PhysicsExplosionSphere();
void Scr_PhysicsRadiusJolt();
void Scr_PhysicsRadiusJitter();
void Scr_PhysicsExplosionCylinder();
void Scr_SetExponentialFog();
void __cdecl Scr_SetFog(const char *cmd, float start, float density, float r, float g, float b, float time);
void Scr_VisionSetNaked();
void Scr_VisionSetNight();
void Scr_TableLookup();
void Scr_TableLookupIString();
void GScr_IsPlayer();
void GScr_IsPlayerNumber();
void GScr_SetWinningPlayer();
void GScr_SetWinningTeam();
void GScr_Announcement();
void GScr_ClientAnnouncement();
void GScr_GetTeamScore();
void GScr_SetTeamScore();
void GScr_SetClientNameMode();
void GScr_UpdateClientNames();
void GScr_GetTeamPlayersAlive();
void __cdecl GScr_GetNumParts();
void __cdecl GScr_GetPartName();
gentity_s *GScr_Earthquake();
void __cdecl GScr_ShellShock(scr_entref_t entref);
void __cdecl GScr_StopShellShock(scr_entref_t entref);
void __cdecl GScr_GetTagOrigin(scr_entref_t entref);
int __cdecl GScr_UpdateTagInternal(
    gentity_s *ent,
    unsigned int tagName,
    cached_tag_mat_t *cachedTag,
    int showScriptError);
void __cdecl GScr_GetTagAngles(scr_entref_t entref);
void __cdecl GScr_SetDepthOfField(scr_entref_t entref);
void __cdecl GScr_SetViewModelDepthOfField(scr_entref_t entref);
void __cdecl GScr_ViewKick(scr_entref_t entref);
void __cdecl GScr_LocalToWorldCoords(scr_entref_t entref);
void __cdecl GScr_SetRightArc(scr_entref_t entref);
void __cdecl GScr_SetLeftArc(scr_entref_t entref);
void __cdecl GScr_SetTopArc(scr_entref_t entref);
void __cdecl GScr_SetBottomArc(scr_entref_t entref);
void __cdecl GScr_PlaceSpawnPoint(scr_entref_t entref);
void GScr_MapRestart();
void GScr_LoadMap();
void GScr_ExitLevel();
void GScr_AddTestClient();
void GScr_MakeDvarServerInfo();
void GScr_AllClientsPrint();
void GScr_MapExists();
void GScr_IsValidGameType();
void GScr_SetVoteString();
void GScr_SetVoteTime();
void GScr_SetVoteYesCount();
void GScr_SetVoteNoCount();
void GScr_KickPlayer();
void GScr_BanPlayer();
void GScr_ClientPrint();
void GScr_OpenFile();
void GScr_CloseFile();
void GScr_FPrintln();
void __cdecl Scr_FPrint_internal(bool commaBetweenFields);
void GScr_FPrintFields();
void GScr_FReadLn();
void GScr_FGetArg();
void __cdecl GScr_GetStat(scr_entref_t entref);
void __cdecl GScr_SetStat(scr_entref_t entref);
void __cdecl GScr_SetTeamForTrigger(scr_entref_t entref);
void __cdecl GScr_ClientClaimTrigger(scr_entref_t entref);
void __cdecl GScr_ClientReleaseTrigger(scr_entref_t entref);
void __cdecl GScr_ReleaseClaimedTrigger(scr_entref_t entref);
void GScr_SetMapCenter();
void GScr_SetGameEndTime();
void GScr_SetMiniMap();
bool GScr_SetTeamRadar();
void GScr_GetTeamRadar();
void GScr_GetArrayKeys();
void __cdecl GScr_IsOnLadder(scr_entref_t entref);
void __cdecl GScr_IsMantling(scr_entref_t entref);
void __cdecl GScr_StartRagdoll(scr_entref_t entref);
void __cdecl GScr_IsRagdoll(scr_entref_t entref);
void __cdecl GScr_GetCorpseAnim(scr_entref_t entref);
void __cdecl ScrCmd_ItemWeaponSetAmmo(scr_entref_t entref);
void(__cdecl *__cdecl Scr_GetFunction(const char **pName, int *type))();
void(__cdecl *__cdecl Scr_GetMethod(const char **pName, int *type))(scr_entref_t);
void(__cdecl *__cdecl BuiltIn_GetMethod(const char **pName, int *type))(scr_entref_t);
void __cdecl Scr_SetOrigin(gentity_s *ent, int i);
void __cdecl Scr_SetAngles(gentity_s *ent, int i);
void __cdecl Scr_SetHealth(gentity_s *ent, int i);
void __cdecl GScr_AddVector(const float *vVec);
void __cdecl GScr_AddEntity(gentity_s *pEnt);
void __cdecl Scr_ParseGameTypeList();
XAssetHeader Scr_ParseGameTypeList_FastFile();
char *__cdecl Scr_GetGameTypeNameForScript(const char *pszGameTypeScript);
bool __cdecl Scr_IsValidGameType(const char *pszGameType);
void __cdecl Scr_LoadGameType();
void __cdecl Scr_StartupGameType();
void __cdecl Scr_PlayerConnect(gentity_s *self);
void __cdecl Scr_PlayerDisconnect(gentity_s *self);
void __cdecl Scr_PlayerDamage(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int dflags,
    unsigned int meansOfDeath,
    unsigned int iWeapon,
    const float *vPoint,
    const float *vDir,
    hitLocation_t hitLoc,
    int timeOffset);
void __cdecl Scr_PlayerKilled(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    unsigned int meansOfDeath,
    unsigned int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int psTimeOffset,
    int deathAnimDuration);
void __cdecl Scr_PlayerLastStand(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    unsigned int meansOfDeath,
    unsigned int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int psTimeOffset);
void __cdecl Scr_VoteCalled(gentity_s *self, char *command, char *param1, char *param2);
void __cdecl Scr_PlayerVote(gentity_s *self, char *option);
void __cdecl GScr_Shutdown();





// g_spawn_mp
struct SpawnFuncEntry // sizeof=0x8
{                                       // ...
    const char *classname;
    void(__cdecl *callback)(gentity_s *);
};
int __cdecl G_LevelSpawnString(const char *key, const char *defaultString, const char **out);
int __cdecl G_SpawnFloat(const char *key, const char *defaultString, float *out);
int __cdecl G_SpawnInt(const char *key, const char *defaultString, int *out);
void __cdecl Scr_ReadOnlyField(gentity_s *ent, int i);
int __cdecl G_CallSpawnEntity(gentity_s *ent);
const gitem_s *__cdecl G_GetItemForClassname(const char *classname, unsigned __int8 model);
void(__cdecl *__cdecl G_FindSpawnFunc(
    const char *classname,
    const SpawnFuncEntry *spawnFuncArray,
    int spawnFuncCount))(gentity_s *);
void __cdecl GScr_AddFieldsForEntity();
void __cdecl GScr_AddFieldsForRadiant();
void __cdecl Scr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs);
int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset);
int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs);
void __cdecl Scr_GetObjectField(unsigned int classnum, int entnum, int offset);
void __cdecl Scr_FreeEntityConstStrings(gentity_s *pEnt);
void __cdecl Scr_FreeEntity(gentity_s *ent);
void __cdecl Scr_AddEntity(gentity_s *ent);
gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index);
gentity_s *__cdecl Scr_GetEntity(unsigned int index);
void __cdecl Scr_FreeHudElem(game_hudelem_s *hud);
void __cdecl Scr_AddHudElem(game_hudelem_s *hud);
unsigned __int16 __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount);
void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount);
void __cdecl Scr_GetEnt();
void __cdecl Scr_GetEntArray();
void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index);
void __cdecl SP_worldspawn();
void __cdecl G_SpawnEntitiesFromString();
void G_CallSpawn();
void __cdecl G_ParseEntityFields(gentity_s *ent);
void __cdecl G_ParseEntityField(const char *key, char *value, gentity_s *ent);
void __cdecl G_SetEntityScriptVariable(const char *key, char *value, gentity_s *ent);
unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, char *value);
void __cdecl G_LoadStructs();
int G_SpawnStruct();




// g_trigger_mp
void __cdecl G_Trigger(gentity_s *self, gentity_s *other);
char __cdecl InitTrigger(gentity_s *self);
void __cdecl InitSentientTrigger(gentity_s *self);
void __cdecl multi_trigger(gentity_s *ent);
void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int extra);
void __cdecl SP_trigger_multiple(gentity_s *ent);
void __cdecl InitTriggerWait(gentity_s *ent, int spawnflag);
void __cdecl SP_trigger_radius(gentity_s *ent);
void __cdecl SP_trigger_disk(gentity_s *ent);
void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int extra);
void __cdecl hurt_use(gentity_s *self, gentity_s *other, gentity_s *third);
void __cdecl SP_trigger_hurt(gentity_s *self);
void __cdecl SP_trigger_once(gentity_s *ent);
bool __cdecl Respond_trigger_damage(gentity_s *pEnt, int iMOD);
void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int iDamage, int iMOD);
void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *third);
void __cdecl Pain_trigger_damage(gentity_s *pSelf, gentity_s *pAttacker, int iDamage, const float *vPoint, int iMod, const float *idk, hitLocation_t hit, int swag);
void Die_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc,
    int timeOffset);
void __cdecl SP_trigger_damage(gentity_s *pSelf);
void __cdecl G_CheckHitTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, unsigned int iMOD);
void __cdecl G_GrenadeTouchTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, int iMOD);
void __cdecl SP_trigger_lookat(gentity_s *self);



// g_vehicles_mp
struct vehicle_info_t // sizeof=0x270
{                                       // ...
    char name[64];
    __int16 type;
    // padding byte
    // padding byte
    int steerWheels;
    int texScroll;
    int quadBarrel;
    int bulletDamage;
    int armorPiercingDamage;
    int grenadeDamage;
    int projectileDamage;
    int projectileSplashDamage;
    int heavyExplosiveDamage;
    float texScrollScale;
    float maxSpeed;
    float accel;
    float rotRate;
    float rotAccel;
    float maxBodyPitch;
    float maxBodyRoll;
    float collisionDamage;
    float collisionSpeed;
    float suspensionTravel;
    char turretWeapon[64];
    float turretHorizSpanLeft;
    float turretHorizSpanRight;
    float turretVertSpanUp;
    float turretVertSpanDown;
    float turretRotRate;
    char sndNames[6][64];
    unsigned __int8 sndIndices[6];
    // padding byte
    // padding byte
    float engineSndSpeed;
};
unsigned __int16 __cdecl BG_VehiclesGetSlotTagName(int slotIndex);
void __cdecl CG_VehRegisterDvars();
DObj_s *__cdecl GetVehicleEntDObj(int localClientNum, centity_s *centVeh);
void __cdecl CG_VehGunnerPOV(int localClientNum, float *resultOrigin, float *resultAngles);
clientInfo_t *__cdecl ClientInfoForLocalClient(int localClientNum);
bool __cdecl CG_VehLocalClientUsingVehicle(int localClientNum);
bool __cdecl CG_VehLocalClientDriving(int localClientNum);
bool __cdecl CG_VehEntityUsingVehicle(int localClientNum, unsigned int entNum);
clientInfo_t *__cdecl ClientInfoForEntity(int localClientNum, unsigned int entNum);
int __cdecl CG_VehLocalClientVehicleSlot(int localClientNum);
int __cdecl CG_VehPlayerVehicleSlot(int localClientNum, unsigned int entNum);
void __cdecl CG_VehSeatTransformForPlayer(
    int localClientNum,
    unsigned int entNum,
    float *resultOrigin,
    float *resultAngles);
void __cdecl SeatTransformForClientInfo(int localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles);
void __cdecl SeatTransformForSlot(
    int localClientNum,
    unsigned int vehEntNum,
    unsigned int vehSlotIdx,
    float *resultOrigin,
    float *resultAngles);
void __cdecl CG_VehSeatOriginForLocalClient(int localClientNum, float *result);
double __cdecl Veh_GetTurretBarrelRoll(int localClientNum, centity_s *cent);
int __cdecl CG_GetEntityIndex(int localClientNum, const centity_s *cent);
vehicleEffects *__cdecl VehicleGetFxInfo(int localClientNum, int entityNum);
void __cdecl Veh_IncTurretBarrelRoll(int localClientNum, int entityNum, float rotation);
void __cdecl CG_VehProcessEntity(int localClientNum, centity_s *cent);
void __cdecl SetupPoseControllers(int localClientNum, DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
unsigned __int16 __cdecl CompressUnit(float unit);
void __cdecl VehicleFXTest(int localClientNum, const DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
double __cdecl GetSpeed(int localClientNum, centity_s *cent);
void __cdecl CG_VehSphereCoordsToPos(float sphereDistance, float sphereYaw, float sphereAltitude, float *result);
void __cdecl CG_Veh_Init();
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *vel, const float *angles);
void __cdecl VEH_InitPhysics(gentity_s *ent);
bool __cdecl VEH_SlideMove(gentity_s *ent, int gravity);
void __cdecl VEH_ClipVelocity(float *in, float *normal, float *out);
void __cdecl VEH_BackupPosition(gentity_s *ent);
void __cdecl VEH_TouchEntities(gentity_s *ent);
void __cdecl VEH_PushEntity(gentity_s *ent, gentity_s *target, float *pushDir, float *deltaOrigin, float *deltaAngles);
bool __cdecl AttachedStickyMissile(gentity_s *vehicle, gentity_s *missile);
void __cdecl PushAttachedStickyMissile(gentity_s *vehicle, gentity_s *missile);
void __cdecl G_VehRegisterDvars();
vehicle_info_t *__cdecl VEH_GetVehicleInfo(__int16 index);
int __cdecl G_VehPlayerRideSlot(gentity_s *vehicle, int playerEntNum);
void __cdecl VEH_DebugCapsule(float *pos, float rad, float height, float r, float g, float b);
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *angles);
void __cdecl VEH_JoltBody(gentity_s *ent, const float *dir, float intensity, float speedFrac, float decel);
void __cdecl VEH_UpdateSounds(gentity_s *ent);
void __cdecl G_VehUnlinkPlayer(gentity_s *ent, gentity_s *player);
void __cdecl VehicleClearRideSlotForPlayer(gentity_s *ent, int playerEntNum);
void __cdecl G_VehiclesInit(int restarting);
void __cdecl InitInfos(int restarting);
void __cdecl G_VehiclesSetupSpawnedEnts();
void __cdecl SetupCollisionMap(gentity_s *ent);
gentity_s *__cdecl GetCollisionMap(const char *modelname);
void __cdecl SpawnVehicle(gentity_s *ent, const char *typeName);
int __cdecl VEH_GetVehicleInfoFromName(const char *name);
int __cdecl G_LoadVehicle(const char *name);
int __cdecl VEH_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, int fieldType);
void __cdecl VEH_InitModelAndValidateTags(gentity_s *ent, int *infoIdx);
char __cdecl VEH_DObjHasRequiredTags(gentity_s *ent, int infoIdx);
void __cdecl InitVehicleTags(gentity_s *ent);
void __cdecl InitEntityVehicleVars(gentity_s *ent, scr_vehicle_s *veh, __int16 infoIdx);
void __cdecl InitEntityVars(gentity_s *ent, scr_vehicle_s *veh, int infoIdx);
void __cdecl G_VehFreeEntity(gentity_s *vehEnt);
bool __cdecl G_VehUsable(gentity_s *vehicle, gentity_s *player);
char __cdecl VehicleHasSeatFree(gentity_s *ent);
bool __cdecl G_VehImmuneToDamage(gentity_s *ent, int mod, char damageFlags, unsigned int weapon);
void __cdecl G_VehEntHandler_Think(gentity_s *pSelf);
VehicleTags *__cdecl RideTagFirst(gentity_s *ent);
VehicleTags *__cdecl RideTagNext(gentity_s *ent, VehicleRideSlot_t *inTag);
void __cdecl VEH_DebugBox(float *pos, float width, float r, float g, float b);
void __cdecl InflictDamage(gentity_s *vehEnt, gentity_s *target, float *dir, int damage);
int __cdecl VehicleEntDriver(gentity_s *ent);
void __cdecl UpdateTurret(gentity_s *ent);
int __cdecl VehicleEntGunner(gentity_s *ent);
void __cdecl VEH_UpdateBody(gentity_s *ent, float frameTime);
void __cdecl VEH_UpdateSteering(gentity_s *ent);
void __cdecl VEH_UpdateMaterialTime(gentity_s *ent, float frameTime);
void __cdecl VEH_UpdateWeapon(gentity_s *ent);
void __cdecl FireTurret(gentity_s *ent, gentity_s *player);
void __cdecl FillWeaponParms(gentity_s *vehEnt, gentity_s *player, weaponParms *wp);
void __cdecl VEH_UpdateClients(gentity_s *ent);
void __cdecl VEH_UpdateClientPassenger(gentity_s *ent);
int __cdecl VehicleEntPassenger(gentity_s *ent);
void __cdecl VEH_UpdateClientGunner(gentity_s *ent);
void __cdecl VEH_UpdateClientDriver(gentity_s *ent);
char __cdecl VEH_PlayerRotation(gentity_s *player, vehicle_physic_t *phys);
void __cdecl UpdateSimulation(gentity_s *ent);
void __cdecl IntegratePosAndRot(gentity_s *ent);
void __cdecl DebugDrawVelocity(const float *origin, const float *velocity, const float *color);
void __cdecl GetAccelerationForces(gentity_s *ent, float frameTime, float *resultPosition);
void __cdecl PositionAccelForces(vehicle_physic_t *phys, float driverAccel, float *result);
bool __cdecl CarTravelingForward(vehicle_physic_t *phys);
bool __cdecl DriverBreaking(vehicle_physic_t *phys, float driverAccel);
void __cdecl AdvanceVehiclePosition(gentity_s *ent, float frameTime);
void __cdecl VEH_GroundTrace(gentity_s *ent);
void __cdecl VEH_GroundMove(gentity_s *ent, float frameTime);
void __cdecl VEH_StepSlideMove(gentity_s *ent, int gravity, float frameTime);
bool __cdecl VEH_SlideMove(gentity_s *ent, int gravity, float frameTime);
void __cdecl VEH_AirMove(gentity_s *ent, int gravity, float frameTime);
void __cdecl VEH_GroundPlant(gentity_s *ent, int gravity, float frameTime);
void __cdecl VEH_DebugLine(float *start, float *end, float r, float g, float b);
void __cdecl VEH_GetWheelOrigin(gentity_s *ent, int idx, float *origin);
void __cdecl AdvanceVehicleRotation(gentity_s *ent, float frameTime);
void __cdecl CapMaxMPH(vehicle_physic_t *phys);
void __cdecl InitFirstThink(gentity_s *pSelf);
void __cdecl G_VehEntHandler_Touch(gentity_s *pSelf, gentity_s *pOther, int bTouched);
void __cdecl G_VehEntHandler_Use(gentity_s *pEnt, gentity_s *pOther, gentity_s *pActivator);
void __cdecl LinkPlayerToVehicle(gentity_s *ent, gentity_s *player);
void __cdecl G_VehEntHandler_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    const int damage,
    const int mod,
    const int weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int psTimeOffset);
void __cdecl G_VehEntHandler_Controller(const gentity_s *pSelf, int *partBits);
void __cdecl G_VehSpawner(gentity_s *pSelf);
void __cdecl G_VehCollmapSpawner(gentity_s *pSelf);




// player_use_mp
struct useList_t // sizeof=0x8
{                                       // ...
    gentity_s *ent;                     // ...
    float score;
};
void __cdecl Player_UpdateActivate(gentity_s *ent);
char __cdecl Player_ActivateCmd(gentity_s *ent);
void __cdecl Player_ActivateHoldCmd(gentity_s *ent);
void __cdecl Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt);
void __cdecl Player_UpdateCursorHints(gentity_s *ent);
int __cdecl Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex);
int __cdecl compare_use(float *pe1, float *pe2);
int __cdecl Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt);
void __cdecl Player_SetTurretDropHint(gentity_s *ent);
void __cdecl Player_SetVehicleDropHint(gentity_s *ent);



extern const dvar_t *vehHelicopterTiltFromControllerAxes;
extern const dvar_t *vehHelicopterTiltFromFwdAndYaw;
extern const dvar_t *vehHelicopterJitterJerkyness;
extern const dvar_t *vehHelicopterTiltSpeed;
extern const dvar_t *vehHelicopterInvertUpDown;
extern const dvar_t *vehHelicopterMaxYawAccel;
extern const dvar_t *vehHelicopterLookaheadTime;
extern const dvar_t *vehHelicopterMaxSpeedVertical;
extern const dvar_t *vehHelicopterTiltFromDeceleration;
extern const dvar_t *vehHelicopterRightStickDeadzone;
extern const dvar_t *vehHelicopterSoftCollisions;
extern const dvar_t *vehHelicopterYawOnLeftStick;
extern const dvar_t *vehHelicopterDecelerationFwd;
extern const dvar_t *vehHelicopterMaxAccelVertical;
extern const dvar_t *vehHelicopterMaxPitch;
extern const dvar_t *vehHelicopterTiltFromAcceleration;
extern const dvar_t *vehHelicopterStrafeDeadzone;
extern const dvar_t *vehHelicopterHoverSpeedThreshold;
extern const dvar_t *vehHelicopterMaxYawRate;
extern const dvar_t *vehHelicopterMaxAccel;
extern const dvar_t *vehHelicopterYawAltitudeControls;
extern const dvar_t *vehHelicopterMaxRoll;
extern const dvar_t *vehHelicopterScaleMovement;
extern const dvar_t *vehHelicopterMaxSpeed;
extern const dvar_t *vehHelicopterTiltFromVelocity;
extern const dvar_t *vehHelicopterTiltMomentum;
extern const dvar_t *vehHelicopterHeadSwayDontSwayTheTurret;
extern const dvar_t *vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
extern const dvar_t *vehHelicopterDecelerationSide;


BuiltinFunctionDef functions[205] =
{
  { "createprintchannel", &GScr_CreatePrintChannel, 1 },
  { "setprintchannel", &GScr_printChannelSet, 1 },
  { "print", &print, 1 },
  { "println", &println, 1 },
  { "iprintln", &iprintln, 0 },
  { "iprintlnbold", &iprintlnbold, 0 },
  { "print3d", &GScr_print3d, 1 },
  { "line", &GScr_line, 1 },
  { "logstring", &CL_ResetStats_f, 0 },
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
  { "objective_onentity",  (void(__cdecl *)())&Scr_Objective_OnEntity, 0 },
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
  { "precachemodel", (void(__cdecl *)()) &Scr_PrecacheModel, 0 },
  { "precacheshellshock", &Scr_PrecacheShellShock, 0 },
  { "precacheitem", &Scr_PrecacheItem, 0 },
  { "precacheshader", (void(__cdecl *)()) &Scr_PrecacheShader, 0 },
  { "precachestring", (void(__cdecl *)()) &Scr_PrecacheString, 0 },
  { "precacherumble", &CL_ResetStats_f, 0 },
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
  { "setplayerignoreradiusdamage", (void(__cdecl *)()) &GScr_SetPlayerIgnoreRadiusDamage, 0 },
  { "getnumparts", &GScr_GetNumParts, 0 },
  { "getpartname", &GScr_GetPartName, 0 },
  { "earthquake", (void(__cdecl *)()) &GScr_Earthquake, 0 },
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
  { "obituary",  (void(__cdecl *)()) &GScr_Obituary, 0 },
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
  { "setarchive", &CL_ResetStats_f, 0 },
  { "allclientsprint", &GScr_AllClientsPrint, 0 },
  { "clientprint", &GScr_ClientPrint, 0 },
  { "mapexists", &GScr_MapExists, 0 },
  { "isvalidgametype", &GScr_IsValidGameType, 0 },
  { "matchend", &CL_ResetStats_f, 0 },
  { "setplayerteamrank", &CL_ResetStats_f, 0 },
  { "sendranks", &CL_ResetStats_f, 0 },
  { "endparty", &CL_ResetStats_f, 0 },
  { "setteamradar",  (void(__cdecl *)()) &GScr_SetTeamRadar, 0 },
  { "getteamradar", &GScr_GetTeamRadar, 0 },
  { "getassignedteam", (void(__cdecl *)()) &PlayerCmd_buttonPressedDEVONLY, 0 },
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
  { "playrumbleonposition", &CL_ResetStats_f, 0 },
  { "playrumblelooponposition", &CL_ResetStats_f, 0 },
  { "stopallrumbles", &CL_ResetStats_f, 0 },
  { "soundexists", &ScrCmd_SoundExists, 0 },
  { "issplitscreen", (void(__cdecl *)()) &PlayerCmd_buttonPressedDEVONLY, 0 },
  { "setminimap", &GScr_SetMiniMap, 0 },
  { "setmapcenter", &GScr_SetMapCenter, 0 },
  { "setgameendtime", &GScr_SetGameEndTime, 0 },
  { "getarraykeys", &GScr_GetArrayKeys, 0 },
  { "searchforonlinegames", &CL_ResetStats_f, 0 },
  { "quitlobby", &CL_ResetStats_f, 0 },
  { "quitparty", &CL_ResetStats_f, 0 },
  { "startparty", &CL_ResetStats_f, 0 },
  { "startprivatematch", &CL_ResetStats_f, 0 },
  { "visionsetnaked", &Scr_VisionSetNaked, 0 },
  { "visionsetnight", &Scr_VisionSetNight, 0 },
  { "tablelookup", &Scr_TableLookup, 0 },
  { "tablelookupistring", &Scr_TableLookupIString, 0 },
  { "endlobby", &CL_ResetStats_f, 0 }
}; // idb