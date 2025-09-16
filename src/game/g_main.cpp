#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_main.h"
#include "g_local.h"
#include <universal/q_shared.h>
#include <cgame/cg_public.h>
#include <script/scr_main.h>
#include "g_scr_load_obj.h"
#include <universal/com_files.h>
#include <database/database.h>
#include "g_save.h"
#include "savememory.h"
#include "actor.h"
#include "sentient.h"
#include <server/sv_game.h>
#include "actor_spawner.h"
#include <aim_assist/aim_target.h>
#include <script/scr_vm.h>
#include "actor_threat.h"
#include <script/scr_const.h>
#include "g_vehicle_path.h"
#include "turret.h"
#include "actor_event_listeners.h"
#include "actor_grenade.h"
#include "actor_corpse.h"
#include <universal/profile.h>
#include <qcommon/cmd.h>

const char *g_helicopterYawAltitudeControlsNames[4] =
{
  "Altitude and yaw are both controlled by the thumbstick",
  "Altitude: thumbstick         Yaw: bumper buttons",
  "Altitude: bumper buttons     Yaw: thumbstick",
  NULL
};

const char *g_entinfoNames[7] =
{
  "off",
  "all ents / draw lines / draw info",
  "selected ent / draw lines / draw info",
  "selected ent / draw info",
  "all ents / draw goal lines and radii",
  "selected ent / draw goal lines and radii",
  NULL
};

const char *g_entinfoTypeNames[4] =
{ "all", "AI only", "vehicle only", NULL };

const char *moveOrientModeStrings[8] =
{
  "invalid",
  "dont_change",
  "motion",
  "enemy",
  "enemy_or_motion",
  "enemy_or_motion_sidestep",
  "goal",
  NULL
};

// 0: void(*think)(gentity_s *);
// 1: void(*reached)(gentity_s *);
// 2: void(*blocked)(gentity_s *, gentity_s *);
// 3: void(*touch)(gentity_s *, gentity_s *, int);
// 4: void(*use)(gentity_s *, gentity_s *, gentity_s *);
// 5: void(*pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int);
// 6: void(*die)(gentity_s *, gentity_s *, gentity_s *, const int, const int, const int, const float *, const hitLocation_t);
// 7: void(*entinfo)(gentity_s *, float *);
// 8: void(*controller)(const gentity_s *, int *);
// 9: int methodOfDeath;
// 10: int splashMethodOfDeath;

entityHandler_t entityHandlers[27] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, Actor_Touch, NULL, Actor_Pain, Actor_Die, Actor_EntInfo, actor_controller, 0, 0 },
  { Actor_Think, NULL, NULL, Actor_Touch, NULL, Actor_Pain, Actor_Die, Actor_EntInfo, actor_controller, 0, 0 },
  { Actor_CorpseThink, NULL, NULL, NULL, NULL, NULL, NULL, misc_EntInfo, NULL, 0, 0 },
  { NULL, NULL, NULL, Touch_Multi, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, Touch_FriendlyChain, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, NULL, hurt_use, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, hurt_touch, hurt_use, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, NULL, Use_trigger_damage, Pain_trigger_damage, Die_trigger_damage, NULL, NULL, 0, 0 },
  { Scr_Vehicle_Init, NULL, NULL, Scr_Vehicle_Touch, Scr_Vehicle_Use, Scr_Vehicle_Pain, Scr_Vehicle_Die, misc_EntInfo, Scr_Vehicle_Controller, 0, 0 },
  { Scr_Vehicle_Think, NULL, NULL, Scr_Vehicle_Touch, Scr_Vehicle_Use, Scr_Vehicle_Pain, Scr_Vehicle_Die, Vehicle_EntInfo, Scr_Vehicle_Controller, 0, 0 },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, misc_EntInfo, NULL, 0, 0 },
  { NULL, Reached_ScriptMover, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, Reached_ScriptMover, NULL, NULL, NULL, NULL, NULL, misc_EntInfo, NULL, 0, 0 },
  { G_ExplodeMissile, NULL, NULL, Touch_Item_Auto, NULL, NULL, NULL, misc_EntInfo, NULL, 3, 4 },
  { G_ExplodeMissile, NULL, NULL, NULL, NULL, NULL, NULL, misc_EntInfo, NULL, 5, 6 },
  { G_FinishSetupSpawnPoint, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, Client_Touch, NULL, NULL, player_die, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, Client_Touch, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, NULL, NULL, NULL, NULL, HeadHitEnt_Pain, HeadHitEnt_Die, NULL, NULL, 0, 0 },
  { turret_think_init, NULL, NULL, NULL, turret_use, NULL, NULL, misc_EntInfo, turret_controller, 0, 0 },
  { turret_think, NULL, NULL, NULL, turret_use, NULL, NULL, misc_EntInfo, turret_controller, 0, 0 },
  { NULL, NULL, NULL, Touch_Item_Auto, NULL, NULL, NULL, EntInfo_Item, NULL, 0, 0 },
  { FinishSpawningItem, NULL, NULL, Touch_Item_Auto, NULL, NULL, NULL, EntInfo_Item, NULL, 0, 0 },
  { NULL, NULL, NULL, Touch_Item_Auto, NULL, NULL, NULL, EntInfo_Item, NULL, 0, 0 },
  { NULL, NULL, NULL, NULL, use_trigger_use, NULL, NULL, NULL, NULL, 0, 0 },
  { NULL, Reached_ScriptMover, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 }
};


gentity_s g_entities[MAX_GENTITIES];
sentient_s g_sentients[33];
char g_nextMap[64];
level_locals_t level;
gclient_s g_clients[1];
actor_s g_actors[32];


const dvar_t *turretPlayerAvoidScale;
const dvar_t *ai_debugFindPathDirect;
const dvar_t *ai_playerNearRange;
const dvar_t *g_entinfo_scale;
const dvar_t *g_entinfo_AItext;
const dvar_t *vehHelicopterTiltFromControllerAxes;
const dvar_t *player_damageMultiplier;
const dvar_t *ai_playerADSTargetTime;
const dvar_t *g_gravity;
const dvar_t *vehHelicopterTiltFromFwdAndYaw;
const dvar_t *ai_debugPlayerLOS;
const dvar_t *ai_eventDistGunShot;
const dvar_t *ai_showClaimedNode;
const dvar_t *vehHelicopterJitterJerkyness;
const dvar_t *g_playerTrailTime;
const dvar_t *vehHelicopterTiltSpeed;
const dvar_t *radius_damage_debug;
const dvar_t *ai_showDodge;
const dvar_t *ai_debugFindPathWidth;
const dvar_t *ai_showVisDataDist;
const dvar_t *ai_coverScore_coverType;
const dvar_t *ai_showBadPlaces;
const dvar_t *ai_playerNearAccuracy;
const dvar_t *ai_playerLOSRange;
const dvar_t *g_dumpAnims;
const dvar_t *ai_eventDistPain;
const dvar_t *vehHelicopterInvertUpDown;
const dvar_t *ai_showSuppression;
const dvar_t *vehHelicopterMaxYawAccel;
const dvar_t *g_dumpAnimsCommands;
const dvar_t *vehHelicopterLookaheadTime;
const dvar_t *ai_eventDistSilencedShot;
const dvar_t *g_dumpAIEvents;
const dvar_t *ai_noDodge;
const dvar_t *ai_showVolume;
const dvar_t *g_vehHudTargetScreenEdgeClampBufferBottom;
const dvar_t *g_friendlyfireDist;
const dvar_t *g_dumpAIEventListeners;
const dvar_t *g_ai;
const dvar_t *vehHelicopterMaxSpeedVertical;
const dvar_t *ai_eventDistFootstep;
const dvar_t *g_enteqEnable;
const dvar_t *ai_friendlySuppression;
const dvar_t *g_vehicleDrawPath;
const dvar_t *ai_debugFindPathLock;
const dvar_t *vehHelicopterTiltFromDeceleration;
const dvar_t *ai_showNodes;
const dvar_t *ai_coverScore_playerLos;
const dvar_t *g_debugBullets;
const dvar_t *g_drawEntBBoxes;
const dvar_t *vehHelicopterRightStickDeadzone;
const dvar_t *player_deathInvulnerableToMelee;
const dvar_t *ai_debugGrenadeHintArc;
const dvar_t *ai_debugMayMove;
const dvar_t *g_knockback;
const dvar_t *ai_badPathSpam;
const dvar_t *g_vehHudTargetScreenEdgeClampBufferLeft;
const dvar_t *ai_ShowCanshootChecks;
const dvar_t *player_throwbackInnerRadius;
const dvar_t *ai_showPaths;
const dvar_t *ai_accuracyDistScale;
const dvar_t *g_vehHudTargetSize;
const dvar_t *ai_debugMeleeAttackSpots;
const dvar_t *ai_threatUpdateInterval;
const dvar_t *player_deathInvulnerableToProjectile;
const dvar_t *ai_friendlySuppressionDist;
const dvar_t *g_earthquakeEnable;
const dvar_t *ai_eventDistExplosion;
const dvar_t *vehHelicopterSoftCollisions;
const dvar_t *ai_coverScore_nodeAngle;
const dvar_t *g_entinfo;
const dvar_t *vehHelicopterYawOnLeftStick;
const dvar_t *melee_debug;
const dvar_t *vehHelicopterDecelerationFwd;
const dvar_t *ai_debugThreatSelection;
const dvar_t *ai_debugEntIndex;
const dvar_t *ai_corpseCount;
const dvar_t *g_player_maxhealth;
const dvar_t *vehHelicopterMaxAccelVertical;
const dvar_t *g_vehicleDebug;
const dvar_t *g_entinfo_type;
const dvar_t *ai_showPotentialThreatDir;
const dvar_t *ai_showFriendlyChains;
const dvar_t *g_weaponAmmoPools;
const dvar_t *ai_eventDistDeath;
const dvar_t *g_enteqDist;
const dvar_t *g_fogColorReadOnly;
const dvar_t *g_deathDelay;
const dvar_t *ai_eventDistProjImpact;
const dvar_t *ai_debugAccuracy;
const dvar_t *ai_debugGrenadeFailSafe;
const dvar_t *g_vehHudTargetScreenEdgeClampBufferRight;
const dvar_t *g_recordScriptPlace;
const dvar_t *vehHelicopterMaxPitch;
const dvar_t *ai_eventDistFootstepLite;
const dvar_t *ai_foliageIngoreDist;
const dvar_t *g_listEntity;
const dvar_t *ai_eventDistProjPing;
const dvar_t *vehHelicopterTiltFromAcceleration;
const dvar_t *g_cheats;
const dvar_t *g_fogHalfDistReadOnly;
const dvar_t *chaplinCheat;
const dvar_t *g_vehicleTexScrollScale;
const dvar_t *g_spawnai;
const dvar_t *ai_playerADS_LOSRange;
const dvar_t *g_minGrenadeDamageSpeed;
const dvar_t *vehHelicopterStrafeDeadzone;
const dvar_t *vehHelicopterHoverSpeedThreshold;
const dvar_t *ai_disableSpawn;
const dvar_t *ai_showVisData;
const dvar_t *g_vehHudTargetScreenEdgeClampBufferTop;
const dvar_t *vehHelicopterMaxYawRate;
const dvar_t *ai_debugClaimedNodes;
const dvar_t *vehHelicopterMaxAccel;
const dvar_t *g_useholdtime;
const dvar_t *vehHelicopterYawAltitudeControls;
const dvar_t *g_speed;
const dvar_t *g_friendlyNameDist;
const dvar_t *player_meleeDamageMultiplier;
const dvar_t *ai_moveOrientMode;
const dvar_t *ai_coverScore_engagement;
const dvar_t *ai_eventDistGrenadePing;
const dvar_t *ai_playerLOSHalfWidth;
const dvar_t *vehHelicopterMaxRoll;
const dvar_t *g_drawGrenadeHints;
const dvar_t *bullet_penetrationEnabled;
const dvar_t *vehHelicopterScaleMovement;
const dvar_t *g_entinfo_maxdist;
const dvar_t *ai_debugAnimDeltas;
const dvar_t *ai_eventDistBadPlace;
const dvar_t *ai_showNearestNode;
const dvar_t *player_deathInvulnerableTime;
const dvar_t *g_reloading;
const dvar_t *vehHelicopterMaxSpeed;
const dvar_t *g_NoScriptSpam;
const dvar_t *g_enteqDelay;
const dvar_t *ai_eventDistNewEnemy;
const dvar_t *ai_coverScore_visibility;
const dvar_t *g_fogStartDistReadOnly;
const dvar_t *ai_coverScore_targetDir;
const dvar_t *player_maxGrenadeCatchHeight;
const dvar_t *ai_eventDistBullet;
const dvar_t *vehHelicopterTiltFromVelocity;
const dvar_t *g_debugLocDamage;
const dvar_t *ai_friendlyFireBlockDuration;
const dvar_t *ai_debugFindPath;
const dvar_t *g_changelevel_time;
const dvar_t *ai_coverScore_distance;
const dvar_t *player_throwbackOuterRadius;
const dvar_t *ai_debugCoverEntityNum;
const dvar_t *vehHelicopterTiltMomentum;
const dvar_t *ai_playerFarAccuracy;
const dvar_t *ai_debugCoverSelection;
const dvar_t *vehHelicopterHeadSwayDontSwayTheTurret;
const dvar_t *ai_playerFarRange;
const dvar_t *ai_showRegion;
const dvar_t *ai_coverScore_priority;
const dvar_t *ai_pathMomentum;
const dvar_t *ai_showNodesDist;
const dvar_t *g_debugDamage;
const dvar_t *vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
const dvar_t *ai_pathNegotiationOverlapCost;
const dvar_t *vehHelicopterDecelerationSide;
const dvar_t *pickupPrints;
const dvar_t *ai_playerLOSMinTime;
const dvar_t *player_radiusDamageMultiplier;


const dvar_s *G_RegisterServerDemoDvars()
{
    // KISAKTODO: missing some flags and descriptions (it is extremely tedious to extract these from the asm)

    Dvar_RegisterString("gamename", "main", 0x46u, "The name of the game");
    Dvar_RegisterString("gamedate", "Sep  7 2007", 0x42u, "The date compiled");
    Dvar_RegisterString("sv_mapname", "", 0x46u, "The current map name");
    Dvar_RegisterString("ui_campaign", "american", 0x1000u, "The current campaign");
    compassMaxRange = Dvar_RegisterFloat("compassMaxRange", 3500.0, 0.0, FLT_MAX, 0, "The maximum range from the player that objects will show on the compass");
    g_reloading = Dvar_RegisterInt("g_reloading", 0, 0, 4, 0x40u, "True if the game is reloading");
    g_player_maxhealth = Dvar_RegisterInt("g_player_maxhealth", 100, 10, 2000, 0x62u, "Player's maximum health");
    player_damageMultiplier = Dvar_RegisterFloat("player_damageMultiplier", 1.0, 0.0, 1000.0, 0, "Player damage is scaled by this amount; used by script for setting difficulty level");
    player_radiusDamageMultiplier = Dvar_RegisterFloat("player_radiusDamageMultiplier", 1.0, 0.0, 1000.0, 0, "Radius damage to the player is scaled by this amount; used by script for setting difficulty level");
    player_meleeDamageMultiplier = Dvar_RegisterFloat("player_meleeDamageMultiplier", 1.0, 0.0, 1000.0, 0, "Melee damage to the player is scaled by this amount; used by script for setting difficulty level");
    player_maxGrenadeCatchHeight = Dvar_RegisterFloat("player_maxGrenadeCatchHeight", 80.0, 0.0, 200.0, 0, "The maximum height difference from grenade to player viewheight at which the player can catch a grenade");
    player_throwbackInnerRadius = Dvar_RegisterFloat("player_throwbackInnerRadius", 72.0, 0.0, FLT_MAX, 0, "The radius to a live grenade player must be within initially to do a throwback");
    player_throwbackOuterRadius = Dvar_RegisterFloat("player_throwbackOuterRadius", 192.0, 0.0, FLT_MAX, 0, "The radius player is allow to throwback a grenade once the player has been in the inner radius");
    g_minGrenadeDamageSpeed = Dvar_RegisterFloat("g_minGrenadeDamageSpeed", 400.0, 0.0, FLT_MAX, 0, "Minimum speed at which getting hit be a grenade will do damage (not the grenade explosion damage)");
    g_speed = Dvar_RegisterInt("g_speed", 190, 0x80000000, 0x7FFFFFFF, 0x1002u, "Maximum player speed");
    g_gravity = Dvar_RegisterFloat("g_gravity", 800.0, 1.0, FLT_MAX, 0, "Game gravity in inches per second per second");
    g_knockback = Dvar_RegisterFloat("g_knockback", 1000.0, -FLT_MAX, FLT_MAX, 0, "Maximum knockback");
    bullet_penetrationEnabled = Dvar_RegisterBool("bullet_penetrationEnabled", 1, 0x82u, "Enable/Disable bullet penetration.");
    g_ai = Dvar_RegisterBool("g_ai", 1, 0x82u, "Enable AI");
    g_spawnai = Dvar_RegisterBool("g_spawnai", 1, 0xA2u, "Enable AI spawning");
    ai_corpseCount = Dvar_RegisterInt("ai_corpseCount", 16, 0, 16, 3u, "Maximum number of AI corpses");
    ai_noDodge = Dvar_RegisterBool("ai_noDodge", 0, 0x82u, "AI won't dodge to the side");
    ai_pathMomentum = Dvar_RegisterFloat("ai_pathMomentum", 0.77999997, 0.0, 0.89999998, 0, "Momentum factor for continuing motion in previous direction. 0 for no momentum carry over");
    ai_pathNegotiationOverlapCost = Dvar_RegisterFloat("ai_pathNegotiationOverlapCost", 300.0, 0.0, 1000.0, 0, "The distance AI would travel around to avoid going to a negotiation being used. Multiplied by number of users of the negotiation");
    ai_playerADS_LOSRange = Dvar_RegisterFloat("ai_playerADS_LOSRange", 150.0, 0.0, 500.0, 0, "Friendly AI will not get in front of player in ADS pointing at enemy");
    ai_playerLOSRange = Dvar_RegisterFloat("ai_playerLOSRange", 150.0, 0.0, 500.0, 0, "Friendly AI will try not to claim cover nodes in this range of player's line of sight. They will also try not to get in front of player throwing grenade in this range.");
    ai_playerLOSHalfWidth = Dvar_RegisterFloat("ai_playerLOSHalfWidth", 15.0, 0.0, 100.0, 0, 0);
    ai_playerLOSMinTime = Dvar_RegisterInt("ai_playerLOSMinTime", 1500, 0, 0x7FFFFFFF, 0x82u, "Player line of sight effect will take effect after player is almost stationary for this time (millisec)");
    ai_playerADSTargetTime = Dvar_RegisterInt("ai_playerADSTargetTime", 10000, 0,
        0x7FFFFFFF,
        0x82u,
        "Duration which the AI will not step in front of the player ADS'ing at a target (millisec)");
    ai_debugPlayerLOS = Dvar_RegisterBool(
        "ai_debugPlayerLOS",
        0,
        0x80u,
        "Debug information for AI staying out of player LOS");
    ai_friendlySuppression = Dvar_RegisterBool(
        "ai_friendlySuppression",
        1,
        0x1082u,
        "Whether AI fire will suppression teammates or not.");
    ai_friendlySuppressionDist = Dvar_RegisterFloat("ai_friendlySuppressionDist", 128.0, 0.0, 2048.0, 0, "Max distance at which AI suppress teammates");
    ai_friendlyFireBlockDuration = Dvar_RegisterInt(
        "ai_friendlyFireBlockDuration",
        2000,
        0,
        0x7FFFFFFF,
        0x1082u,
        "Friendly fire movement block duration");
    ai_playerNearAccuracy = Dvar_RegisterFloat("ai_playerNearAccuracy", 0.5, 0.0, 1.0, 0, "Accuracy for an AI near to a player");
    ai_playerNearRange = Dvar_RegisterFloat("ai_playerNearRange", 800.0, 0.0, FLT_MAX, 0, "Maximum range for AI to use 'near' accuracy");
    ai_playerFarAccuracy = Dvar_RegisterFloat("ai_playerFarAccuracy", 0.1, 0.0, 1.0, 0, "Accuracy for an AI far from a player");
    ai_playerFarRange = Dvar_RegisterFloat("ai_playerFarRange", 2000.0, 0.0, FLT_MAX, 0, "Minimum range for AI to use 'far' accuracy");
    ai_threatUpdateInterval = Dvar_RegisterInt(
        "ai_threatUpdateInterval",
        500,
        0,
        0x7FFFFFFF,
        0x82u,
        "AI target threat update interval in milliseconds");
    ai_foliageIngoreDist = Dvar_RegisterFloat("ai_foliageSeeThroughDist", 128.0, 0.0, FLT_MAX, 0, "Maximum distance AI ignore foliage for sight trace to targets");
    ai_coverScore_coverType = Dvar_RegisterFloat("ai_coverScore_coverType", 2.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on concelament vs. cover nodes");
    ai_coverScore_distance = Dvar_RegisterFloat("ai_coverScore_distance", 16.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on distance to node");
    ai_coverScore_engagement = Dvar_RegisterFloat("ai_coverScore_engagement", 4.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on engagement distance to enemy");
    ai_coverScore_nodeAngle = Dvar_RegisterFloat("ai_coverScore_nodeAngle", 4.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on node angles to enemy");
    ai_coverScore_playerLos = Dvar_RegisterFloat("ai_coverScore_playerLos", 8.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on blocking the player's LOS");
    ai_coverScore_priority = Dvar_RegisterFloat("ai_coverScore_priority", 8.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on node priority flag");
    ai_coverScore_targetDir = Dvar_RegisterFloat("ai_coverScore_targetDir", 4.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on direction to enemy");
    ai_coverScore_visibility = Dvar_RegisterFloat("ai_coverScore_visibility", 8.0, 0.0, FLT_MAX, 0, "Cover node score used when weighting based on visibilty to enemy node");
    ai_accuracyDistScale = Dvar_RegisterFloat("ai_accuracyDistScale", 1.0, 1.1754944e-38, FLT_MAX, 0, "Distance scale for AI accuracy calculations. Higher = less accurate");
    g_playerTrailTime = Dvar_RegisterInt(
        "g_playerTrailTime",
        500,
        0,
        0x7FFFFFFF,
        0x82u,
        "Time for player 'trail' for AI missing");
    ai_disableSpawn = Dvar_RegisterBool("ai_disableSpawn", 0, 0x82u, "Do not spawn AI");
    g_changelevel_time = Dvar_RegisterFloat("g_changelevel_time", 4.0, 0.0, FLT_MAX, 0, "Time for change level fade out");
    g_debugLocDamage = Dvar_RegisterBool("g_debugLocDamage", 0, 0x82u, "Display locational damage debug information");
    g_friendlyNameDist = Dvar_RegisterFloat("g_friendlyNameDist", 15000.0, 0.0, 15000.0, 0, "Maximum distance at which a friendly name shows when the crosshairs is over them");
    g_friendlyfireDist = Dvar_RegisterFloat("g_friendlyfireDist", 175.0, 0.0, 15000.0, 0, "Maximum distance at which the player channot shoot while the crosshair is over a friendly");
    melee_debug = Dvar_RegisterBool("melee_debug", 0, 0x80u, "Turn on debug lines for melee traces");
    radius_damage_debug = Dvar_RegisterBool(
        "radius_damage_debug",
        0,
        0x80u,
        "Turn on debug lines for radius damage traces");
    g_useholdtime = Dvar_RegisterInt(
        "g_useholdtime",
        250,
        0,
        0x7FFFFFFF,
        2u,
        "The time to hold down the 'use' button to activate a 'use' command");
    player_deathInvulnerableTime = Dvar_RegisterInt(
        "player_deathInvulnerableTime",
        1000,
        0,
        0x7FFFFFFF,
        0x1082u,
        "The time that the player is invulnerable before dying");
    player_deathInvulnerableToMelee = Dvar_RegisterBool(
        "player_deathInvulnerableToMelee",
        0,
        0x82u,
        "The player is invulnerable to melee attacks during death invulnerability");
    player_deathInvulnerableToProjectile = Dvar_RegisterBool(
        "player_deathInvulnerableToProjectile",
        1,
        0x82u,
        "The player is invulnerable to projectile attacks during death invulnerability");
    Dvar_RegisterBool("hud_drawHUD", 1, 0x1000u, "Draw the HUD. Controlled from non-UI script");
    vehHelicopterMaxSpeed = Dvar_RegisterFloat("vehHelicopterMaxSpeed", 150.0, 0.0099999998, FLT_MAX, 0, "Maximum horizontal speed of the player helicopter (in MPH)");
    vehHelicopterMaxSpeedVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxSpeedVertical",
        65.0,
        0.0099999998,
        FLT_MAX,
        0,
        "Maximum vertical speed of the player helicopter (in MPH)");
    vehHelicopterMaxAccel = Dvar_RegisterFloat("vehHelicopterMaxAccel", 45.0, 0.0099999998, FLT_MAX, 0, "Maximum horizontal acceleration of the player helicopter (in MPH per second)");
    vehHelicopterMaxAccelVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxAccelVertical",
        30.0,
        0.0099999998,
        FLT_MAX,
        0,
        "Maximum vertical acceleration of the player helicopter (in MPH per second)");
    vehHelicopterMaxYawRate = Dvar_RegisterFloat("vehHelicopterMaxYawRate", 120.0, 0.0099999998, FLT_MAX, 0, "Maximum yaw speed of the player helicopter");
    vehHelicopterMaxYawAccel = Dvar_RegisterFloat("vehHelicopterMaxYawAccel", 90.0, 0.0099999998, FLT_MAX, 0, "Maximum yaw acceleration of the player helicopter");
    vehHelicopterMaxPitch = Dvar_RegisterFloat("vehHelicopterMaxPitch", 10.0, 0.0099999998, FLT_MAX, 0, "Maximum pitch of the player helicopter");
    vehHelicopterMaxRoll = Dvar_RegisterFloat("vehHelicopterMaxRoll", 35.0, 0.0099999998, FLT_MAX, 0, "Maximum roll of the player helicopter");
    vehHelicopterLookaheadTime = Dvar_RegisterFloat(
        "vehHelicopterLookaheadTime",
        1.0,
        0.0099999998,
        FLT_MAX,
        0,
        "How far ahead (in seconds) the player helicopter looks ahead, to avoid hard collisions.  (Like driving down the highway, you should keep 2 seconds distance between you and the vehicle in front of you)");
    vehHelicopterHoverSpeedThreshold = Dvar_RegisterFloat(
        "vehHelicopterHoverSpeedThreshold",
        400.0,
        0.0099999998,
        FLT_MAX,
        0,
        "The speed below which the player helicopter begins to jitter the tilt, for hovering");
    vehHelicopterRightStickDeadzone = Dvar_RegisterFloat(
        "vehHelicopterRightStickDeadzone",
        0.30000001,
        0.0099999998,
        1.0,
        0,
        "Dead-zone for the axes of the right thumbstick.  This helps to better control the two axes separately.");
    vehHelicopterStrafeDeadzone = Dvar_RegisterFloat(
        "vehHelicopterStrafeDeadzone",
        0.30000001,
        0.0099999998,
        1.0,
        0,
        "Dead-zone so that you can fly straight forward easily without accidentally strafing (and thus rolling).");
    vehHelicopterScaleMovement = Dvar_RegisterBool(
        "vehHelicopterScaleMovement",
        1,
        0x80u,
        "Scales down the smaller of the left stick axes.");
    vehHelicopterYawAltitudeControls = Dvar_RegisterEnum(
        "vehHelicopterYawAltitudeControls",
        g_helicopterYawAltitudeControlsNames,
        0,
        0,
        "Determines how to control yaw and altitude");
    vehHelicopterSoftCollisions = Dvar_RegisterBool(
        "vehHelicopterSoftCollisions",
        0,
        0x80u,
        "Player helicopters have soft collisions (slow down before they collide).");
    vehHelicopterDecelerationFwd = Dvar_RegisterFloat("vehHelicopterDecelerationFwd", 0.5, 0.0, FLT_MAX, 0, 0);
    vehHelicopterDecelerationSide = Dvar_RegisterFloat("vehHelicopterDecelerationSide", 1.0, 0.0, FLT_MAX, 0, 0);
    vehHelicopterInvertUpDown = Dvar_RegisterBool(
        "vehHelicopterInvertUpDown",
        0,
        0x80u,
        "Invert the altitude control on the player helicopter.");
    vehHelicopterYawOnLeftStick = Dvar_RegisterFloat("vehHelicopterYawOnLeftStick", 5.0, 0.0, FLT_MAX, 0, 0);
    vehHelicopterTiltSpeed = Dvar_RegisterFloat("vehHelicopterTiltSpeed", 1.2, 0.0099999998, FLT_MAX, 0, 0);
    vehHelicopterTiltFromAcceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromAcceleration",
        2.0,
        0.0099999998,
        FLT_MAX,
        0,
        0);
    vehHelicopterTiltFromDeceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromDeceleration",
        2.0,
        0.0,
        FLT_MAX,
        0,
        0);
    vehHelicopterTiltFromVelocity = Dvar_RegisterFloat(
        "vehHelicopterTiltFromVelocity",
        1.0,
        0.0,
        FLT_MAX,
        0,
        0);
    vehHelicopterTiltFromControllerAxes = Dvar_RegisterFloat(
        "vehHelicopterTiltFromControllerAxes",
        0.0,
        0.0,
        FLT_MAX,
        0,
        0);
    vehHelicopterTiltFromFwdAndYaw = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw",
        0.0,
        0.0,
        FLT_MAX,
        0,
        0);
    vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt",
        1.0,
        0.0,
        FLT_MAX,
        0,
        0);
    vehHelicopterJitterJerkyness = Dvar_RegisterFloat(
        "vehHelicopterJitterJerkyness",
        0.30000001,
        0.000099999997,
        FLT_MAX,
        0,
        0);
    vehHelicopterHeadSwayDontSwayTheTurret = Dvar_RegisterBool(
        "vehHelicopterHeadSwayDontSwayTheTurret",
        1,
        0,
        "If set, the turret will not fire through the crosshairs, but straight ahead"
        " of the vehicle, when the player is not freelooking.");
    vehHelicopterTiltMomentum = Dvar_RegisterFloat(
        "vehHelicopterTiltMomentum",
        0.40000001,
        0.000099999997,
        FLT_MAX,
        0,
        0);
    g_vehHudTargetSize = Dvar_RegisterFloat("vehHudTargetSize", 30.0, 0.0099999998, 10000.0, 0, 0);
    g_vehHudTargetScreenEdgeClampBufferLeft = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferLeft",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    g_vehHudTargetScreenEdgeClampBufferRight = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferRight",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    g_vehHudTargetScreenEdgeClampBufferTop = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferTop",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    g_vehHudTargetScreenEdgeClampBufferBottom = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferBottom",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    turretPlayerAvoidScale = Dvar_RegisterFloat("turretPlayerAvoidScale", 1.7, 0.0, FLT_MAX, 0, 0);
    G_RegisterMissileDvars();
    BG_RegisterDvars();
    g_fogColorReadOnly = Dvar_RegisterColor("g_fogColorReadOnly", 1.0, 0.0, 0.0, 1.0, 0, 0);
    g_fogStartDistReadOnly = Dvar_RegisterFloat("g_fogStartDistReadOnly", 0.0, 0.0, FLT_MAX, 0, 0);
    g_fogHalfDistReadOnly = Dvar_RegisterFloat("g_fogHalfDistReadOnly", 0.1, 0.0, FLT_MAX, 0, 0);
    chaplinCheat = Dvar_RegisterBool("chaplinCheat", 0, 0x1080u, "");
    return Dvar_RegisterBool("scr_dof_enable", 1, 0x1082u, "enable dof");
}

void G_RegisterDebugDvars()
{
    // KISAKTODO: missing some flags and descriptions (it is extremely tedious to extract these from the asm)

    g_cheats = Dvar_RegisterBool(
        "sv_cheats",
        1,
        0,
        "Turn on cheats: do not access directly - access through CheatsOk for server demo");
    g_debugDamage = Dvar_RegisterBool("g_debugDamage", 0, 0x80u, "Turn on debug information for damage");
    g_debugBullets = Dvar_RegisterInt("g_debugBullets", 0, -3, 6, 0x80u, "Turn on debug information for bullets");
    g_vehicleDrawPath = Dvar_RegisterString(
        "g_vehicleDrawPath",
        "",
        0x80u,
        "Turn on debug information for vehicle paths");
    g_vehicleDebug = Dvar_RegisterBool("g_vehicleDebug", 0, 0x80u, "Turn on debug information for vehicles");
    g_vehicleTexScrollScale = Dvar_RegisterFloat("g_vehicleTexScrollScale", 0.0, 0.0, FLT_MAX, 0, 0);
    g_listEntity = Dvar_RegisterBool("g_listEntity", 0, 0, "list all of the current entities");
    g_entinfo = Dvar_RegisterEnum("g_entinfo", g_entinfoNames, 0, 0x80u, "Display entity information");
    g_entinfo_type = Dvar_RegisterEnum(
        "g_entinfo_type",
        g_entinfoTypeNames,
        1,
        0x80u,
        "Type of entities to display information");
    g_entinfo_AItext = Dvar_RegisterEnum(
        "g_entinfo_AItext",
        g_entinfoAITextNames,
        1,
        0x80u,
        "Type of text information for AI entinfo");
    g_entinfo_maxdist = Dvar_RegisterFloat("g_entinfo_maxdist", 2048.0, 0.0, FLT_MAX, 0, 0);
    g_entinfo_scale = Dvar_RegisterFloat("g_entinfo_scale", 1.2, 0.0, 10.0, 0, 0);
    g_drawEntBBoxes = Dvar_RegisterInt("g_drawEntBBoxes", 0, 0, 4, 0x80u, "Draw entity bounding boxes");
    g_drawGrenadeHints = Dvar_RegisterInt(
        "g_drawGrenadeHints",
        0,
        0,
        0x7FFFFFFF,
        0x80u,
        "Draw debug information for grenades");
    g_deathDelay = Dvar_RegisterInt("g_deathDelay", 3000, 0, 0x7FFFFFFF, 1u, "Delay a level restart on death");
    ai_showNodes = Dvar_RegisterInt("ai_showNodes", 0, 0, 4, 0x80u, "Show AI navigation node debug information");
    ai_showNodesDist = Dvar_RegisterFloat("ai_showNodesDist", 384.0, 0.0, FLT_MAX, 0, 0);
    ai_showNearestNode = Dvar_RegisterInt("ai_showNearestNode", 0, 0, 64, 0x80u, "Show nodes closest to AI");
    ai_showFriendlyChains = Dvar_RegisterInt(
        "ai_showFriendlyChains",
        0,
        0,
        2,
        0x80u,
        "Display debug information for friendly chains");
    ai_showVisData = Dvar_RegisterInt("ai_showVisData", 0, 0, 2, 0x80u, "Display debug information for visibility data");
    ai_showVisDataDist = Dvar_RegisterFloat("ai_showVisDataDist", 1000.0, 0.0, FLT_MAX, 0, 0);
    ai_debugCoverEntityNum = Dvar_RegisterInt(
        "ai_debugCoverEntityNum",
        -1,
        -1,
        ENTITYNUM_NONE,
        0x80u,
        "Display debug info for cover");
    ai_showBadPlaces = Dvar_RegisterBool("ai_showBadPlaces", 0, 0x80u, "Display debug information for 'bad places'");
    ai_showDodge = Dvar_RegisterBool("ai_showDodge", 0, 0x80u, "Display debug information for AI dodging");
    ai_debugMayMove = Dvar_RegisterBool(
        "ai_debugMayMove",
        0,
        0x80u,
        "Display debug information for AI 'may move' calculations");
    ai_showPotentialThreatDir = Dvar_RegisterBool(
        "ai_showPotentialThreatDir",
        0,
        0x80u,
        "Display AI potential threat direction");
    ai_moveOrientMode = Dvar_RegisterEnum("ai_moveOrientMode", moveOrientModeStrings, 0, 0x80u, "Debug AI Orient Mode");
    ai_showSuppression = Dvar_RegisterInt(
        "ai_showSuppression",
        -1,
        -1,
        ENTITYNUM_NONE,
        0x80u,
        "Draw the suppression planes for this entity");
    ai_eventDistFootstep = Dvar_RegisterFloat("ai_eventDistFootstep", 512.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistFootstepLite = Dvar_RegisterFloat("ai_eventDistFootstepLite", 256.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistNewEnemy = Dvar_RegisterFloat("ai_eventDistNewEnemy", 1024.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistPain = Dvar_RegisterFloat("ai_eventDistPain", 512.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistDeath = Dvar_RegisterFloat("ai_eventDistDeath", 1024.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistExplosion = Dvar_RegisterFloat("ai_eventDistExplosion", 1024.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistGrenadePing = Dvar_RegisterFloat("ai_eventDistGrenadePing", 512.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistProjPing = Dvar_RegisterFloat("ai_eventDistProjPing", 128.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistGunShot = Dvar_RegisterFloat("ai_eventDistGunShot", 2048.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistSilencedShot = Dvar_RegisterFloat("ai_eventDistSilencedShot", 128.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistBullet = Dvar_RegisterFloat("ai_eventDistBullet", 96.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistProjImpact = Dvar_RegisterFloat("ai_eventDistProjImpact", 256.0, 0.0, FLT_MAX, 0, 0);
    ai_eventDistBadPlace = Dvar_RegisterFloat("ai_eventDistBadPlace", 256.0, 0.0, FLT_MAX, 0, 0);
    ai_showPaths = Dvar_RegisterInt("ai_showPaths", 0, 0, 2, 0x80u, "Show AI navigation paths");
    ai_debugFindPath = Dvar_RegisterInt(
        "ai_debugFindPath",
        0,
        0,
        5,
        0x80u,
        "Display AI 'find path' debugging information");
    ai_debugFindPathDirect = Dvar_RegisterBool(
        "ai_debugFindPathDirect",
        0,
        0x80u,
        "Display AI 'find direct path' debugging information");
    ai_debugFindPathWidth = Dvar_RegisterFloat("ai_debugFindPathWidth", 0.0, -FLT_MAX, FLT_MAX, 0, 0);
    ai_debugFindPathLock = Dvar_RegisterBool("ai_debugFindPathLock", 0, 0x80u, "Find path lock");
    ai_debugAnimDeltas = Dvar_RegisterInt(
        "ai_debugAnimDeltas",
        0,
        0,
        ENTITYNUM_NONE,
        0x80u,
        "Display animation delta debug information");
    ai_debugGrenadeFailSafe = Dvar_RegisterBool(
        "ai_debugGrenadeFailSafe",
        0,
        0x80u,
        "Display grenade fail safe debug information");
    ai_debugGrenadeHintArc = Dvar_RegisterFloat("ai_debugGrenadeHintArc", 150.0, 0.0, 180.0, 0, 0);
    ai_ShowCanshootChecks = Dvar_RegisterBool(
        "ai_ShowCanshootChecks",
        0,
        1u,
        "Display debugging information for 'can shoot' checks");
    ai_badPathSpam = Dvar_RegisterBool("ai_badPathSpam", 0, 1u, "Write debugging information for a bad AI paths");
    ai_debugCoverSelection = Dvar_RegisterBool(
        "ai_debugCoverSelection",
        0,
        0x81u,
        "Enable debugging information for cover selection");
    ai_showClaimedNode = Dvar_RegisterBool("ai_showClaimedNode", 0, 0x81u, "Show claimed node debugging information");
    ai_debugThreatSelection = Dvar_RegisterBool(
        "ai_debugThreatSelection",
        0,
        0x81u,
        "Enable debugging information for threat selection");
    ai_debugMeleeAttackSpots = Dvar_RegisterBool(
        "ai_debugMeleeAttackSpots",
        0,
        0x81u,
        "Enable debugging information for melee attack spots");
    ai_debugAccuracy = Dvar_RegisterBool("ai_debugAccuracy", 0, 0x81u, "Enable debugging information for accuracy");
    ai_debugClaimedNodes = Dvar_RegisterInt(
        "ai_debugClaimedNodes",
        0,
        0,
        64,
        0x80u,
        "Enable debugging information claimed status of nodes");
    ai_showRegion = Dvar_RegisterBool("ai_showRegion", 0, 0x81u, "Draw all the cover in a volume for debugging");
    ai_showVolume = Dvar_RegisterInt(
        "ai_showVolume",
        -1,
        -1,
        ENTITYNUM_NONE,
        0x80u,
        "Draw the goal volume and fixed node safe volume for an AI");
    ai_debugEntIndex = Dvar_RegisterInt("ai_debugEntIndex", -1, -1, ENTITYNUM_NONE, 0, "Entity index of an entity to debug");
    g_NoScriptSpam = Dvar_RegisterBool("g_no_script_spam", 0, 0, "Turn off script debugging information");
    g_recordScriptPlace = Dvar_RegisterBool(
        "g_recordScriptPlace",
        0,
        0,
        "Records the file and line of the current script command");
    g_dumpAnims = Dvar_RegisterInt("g_dumpAnims", -1, -1, ENTITYNUM_NONE, 0x80u, "Write animation debug info for this entity");
    g_dumpAnimsCommands = Dvar_RegisterInt(
        "g_dumpAnimsCommands",
        -1,
        -1,
        ENTITYNUM_NONE,
        0x80u,
        "Write animation commands debug info for this entity");
    g_dumpAIEvents = Dvar_RegisterInt("g_aiEventDump", -1, -1, ENTITYNUM_NONE, 0x80u, "Print AI events happening for this entity");
    g_dumpAIEventListeners = Dvar_RegisterBool(
        "g_aiEventListenerDump",
        0,
        0x80u,
        "Dump the AI event listeners once. Automatically reset to false after dump");
    g_earthquakeEnable = Dvar_RegisterBool("g_earthquakeEnable", 1, 0x80u, "Enable camera shake");
    g_enteqDelay = Dvar_RegisterInt(
        "g_enteqDelay",
        150,
        0,
        0x7FFFFFFF,
        0x80u,
        "Minimum delay in (ms) beteewen entity eq updates");
    g_enteqEnable = Dvar_RegisterBool("g_enteqEnable", 1, 0x1080u, "Enable entity eq");
    g_enteqDist = Dvar_RegisterFloat("g_enteqDist", 4000.0, 0.0, FLT_MAX, 0, 0);
    pickupPrints = Dvar_RegisterBool(
        "pickupPrints",
        0,
        0x80u,
        "Print a message to the game window when picking up ammo, etc.");
    G_RegisterMissileDebugDvars();
}

void __cdecl TRACK_g_main()
{
    track_static_alloc_internal(&level, 15892, "level", 9);
    track_static_alloc_internal(g_entities, 1366528, "g_entities", 9);
    track_static_alloc_internal(g_clients, 46104, "g_clients", 9);
    track_static_alloc_internal(g_sentients, 3828, "g_sentients", 5);
    track_static_alloc_internal(entityHandlers, 1188, "entityHandlers", 9);
    track_static_alloc_internal(g_entinfoNames, 28, "g_entinfoNames", 0);
    track_static_alloc_internal(g_entinfoTypeNames, 16, "g_entinfoTypeNames", 0);
    track_static_alloc_internal(g_helicopterYawAltitudeControlsNames, 16, "g_helicopterYawAltitudeControlsNames", 0);
    track_static_alloc_internal(g_actors, 250368, "g_actors", 5);
}

int __cdecl G_GetServerSnapTime()
{
    return level.snapTime;
}

int __cdecl G_GetTime()
{
    return level.time;
}

int __cdecl G_GetSpecialIndex(int entnum)
{
    return level.specialIndex[entnum];
}

void G_RegisterDvars()
{
    ProfLoad_Begin("server demo dvars");
    G_RegisterServerDemoDvars();
    ProfLoad_End();
    ProfLoad_Begin("debug dvars");
    G_RegisterDebugDvars();
    ProfLoad_End();
}

void G_InitDvars()
{
    ProfLoad_Begin("init game dvars");
    ProfLoad_Begin("server demo dvars");
    G_RegisterServerDemoDvars();
    ProfLoad_End();
    ProfLoad_Begin("debug dvars");
    G_RegisterDebugDvars();
    ProfLoad_End();
    Dvar_ResetDvars(0x1000u, DVAR_SOURCE_INTERNAL);
    if (!Dvar_GetBool("sv_cheats"))
        Dvar_SetCheatState();
    SV_SetValuesFromSkill();
    ProfLoad_End();
}

void __cdecl G_FreeEntities()
{
    int v0; // r29
    gentity_s *v1; // r31
    int i; // r11

    v0 = 0;
    v1 = g_entities;
    for (i = level.num_entities; v0 < i; ++v1)
    {
        if (v1->r.inuse)
        {
            G_FreeEntity(v1);
            i = level.num_entities;
        }
        ++v0;
    }
    if (g_entities[ENTITYNUM_WORLD].r.inuse)
        G_FreeEntity(&g_entities[ENTITYNUM_WORLD]);
    G_FreeAllEntityRefs();
    iassert(!g_entities[ENTITYNUM_NONE].r.inuse);
    level.num_entities = 0;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
    if (g_scr_data.actorBackupXAnimTree)
        XAnimClearTree(g_scr_data.actorBackupXAnimTree);
    Path_CheckLinkLeaks();
    Path_CheckUserCountLeaks();
}

void *__cdecl Hunk_AllocActorXAnimServer(int size)
{
    return Hunk_AllocLow(size, "Hunk_AllocActorXAnimServer", 5);
}

void *G_LoadAnimTreeInstances()
{
    int v0; // r31
    XAnimTree_s **actorXAnimTrees; // r30
    XAnim_s *anims; // r29
    int v3; // r30
    int *p_entnum; // r31
    int v5; // r31
    XAnimTree_s **actorXAnimClientTrees; // r30
    void *result; // r3

    v0 = 32;
    actorXAnimTrees = g_scr_data.actorXAnimTrees;
    anims = g_scr_data.generic_human.tree.anims;
    do
    {
        --v0;
        *actorXAnimTrees++ = XAnimCreateTree(anims, Hunk_AllocActorXAnimServer);
    } while (v0);
    g_scr_data.actorBackupXAnimTree = XAnimCreateTree(anims, Hunk_AllocActorXAnimServer);
    v3 = 16;
    p_entnum = &g_scr_data.actorCorpseInfo[0].entnum;
    do
    {
        --v3;
        *(p_entnum - 1) = (int)XAnimCreateTree(anims, Hunk_AllocActorXAnimServer);
        *p_entnum = -1;
        p_entnum += 8;
    } while (v3);
    v5 = 64;
    actorXAnimClientTrees = g_scr_data.actorXAnimClientTrees;
    do
    {
        --v5;
        *actorXAnimClientTrees++ = XAnimCreateTree(anims, Hunk_AllocActorXAnimServer);
    } while (v5);
    result = memset(g_scr_data.actorXAnimClientTreesInuse, 0, sizeof(g_scr_data.actorXAnimClientTreesInuse));
    g_scr_data.actorFreeClientTree = 0;
    return result;
}

void G_FreeAnimTreeInstances()
{
    int v0; // r30
    XAnimTree_s **actorXAnimTrees; // r31
    int v2; // r30
    corpseInfo_t *actorCorpseInfo; // r31
    int v4; // r30
    XAnimTree_s **actorXAnimClientTrees; // r31

    v0 = 32;
    actorXAnimTrees = g_scr_data.actorXAnimTrees;
    do
    {
        if (*actorXAnimTrees)
        {
            XAnimFreeTree(*actorXAnimTrees, 0);
            *actorXAnimTrees = 0;
        }
        --v0;
        ++actorXAnimTrees;
    } while (v0);
    if (g_scr_data.actorBackupXAnimTree)
    {
        XAnimFreeTree(g_scr_data.actorBackupXAnimTree, 0);
        g_scr_data.actorBackupXAnimTree = 0;
    }
    v2 = 16;
    actorCorpseInfo = g_scr_data.actorCorpseInfo;
    do
    {
        if (actorCorpseInfo->tree)
        {
            XAnimFreeTree(actorCorpseInfo->tree, 0);
            actorCorpseInfo->tree = 0;
        }
        --v2;
        ++actorCorpseInfo;
    } while (v2);
    v4 = 64;
    actorXAnimClientTrees = g_scr_data.actorXAnimClientTrees;
    do
    {
        if (*actorXAnimClientTrees)
        {
            XAnimFreeTree(*actorXAnimClientTrees, 0);
            *actorXAnimClientTrees = 0;
        }
        --v4;
        ++actorXAnimClientTrees;
    } while (v4);
}

void __cdecl G_ClearLowHunk()
{
    if (level.loading == LOADING_SAVEGAME)
        XAnimDisableLeakCheck();
    else
        G_FreeAnimTreeInstances();
    Mantle_ShutdownAnims();
    GScr_FreeScripts();
    Scr_FreeScripts(1);
    Com_FreeWeaponInfoMemory(1);
    Hunk_ClearToMarkLow(0);
    Hunk_ResetDebugMem();
}

bool __cdecl G_DemoPlaying()
{
    return level.demoplaying;
}

void *__cdecl Hunk_AllocXAnimServer(int size)
{
    return Hunk_AllocLow(size, "Hunk_AllocXAnimServer", 11);
}

bool __cdecl G_ExitAfterConnectPaths()
{
    return false;
}

void *GScr_LoadScriptsAndAnims()
{
    const char *mapname; // r3
    ScriptFunctions functions; // [sp+50h] [-220h] BYREF
    int address[128];

    functions.count = 0;
    functions.maxSize = 128;
    functions.address = address;
    mapname = Dvar_GetString("mapname");
    GScr_LoadScripts(mapname, &functions);
    functions.maxSize = functions.count;
    functions.count = 0;
    GScr_SetScripts(&functions);

    if (functions.maxSize != functions.count)
        Com_Error(ERR_DROP, "Script function count mismatch");

    return G_LoadAnimTreeInstances();
}

void ScriptIOFilesInit()
{
    level.openScriptIOFileHandles[0] = 0;
    level.openScriptIOFileBuffers[0] = 0;
    level.currentScriptIOLineMark[0].lines = 0;
    level.currentScriptIOLineMark[0].text = 0;
    level.currentScriptIOLineMark[0].ungetToken = 0;
    level.currentScriptIOLineMark[0].backup_lines = 0;
    level.currentScriptIOLineMark[0].backup_text = 0;
}

void ScriptIOFilesShutdown()
{
    if (level.openScriptIOFileBuffers[0])
        Z_VirtualFree(level.openScriptIOFileBuffers[0]);
    level.openScriptIOFileBuffers[0] = 0;
    if (level.openScriptIOFileHandles[0])
        FS_FCloseFile(level.openScriptIOFileHandles[0]);
    level.openScriptIOFileHandles[0] = 0;
}

void __cdecl G_PrintFastFileErrors(const char *fastfile)
{
    iassert(fastfile);

    RawFile *rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fastfile).rawfile;
    iassert(rawfile);

    if (rawfile->len)
    {
        Com_PrintError(1, "There were errors when building fast file '%s'\n", fastfile);
        Com_PrintError(1, rawfile->buffer);
    }
}

void __cdecl G_PrintAllFastFileErrors()
{
    G_PrintFastFileErrors("code_post_gfx");
    G_PrintFastFileErrors("common");
    G_PrintFastFileErrors(sv_mapname->current.string);
}

void __cdecl G_InitGame(
    unsigned int randomSeed,
    int restart,
    int checksum,
    int loadScripts,
    int savegame,
    SaveGame **save)
{
    int v17; // r30
    actorBackup_s *actorBackup; // r3
    const SaveHeader *Header; // r3
    int maxclients; // r11
    gclient_s *clients; // r6
    gclient_s *v22; // r10
    gclient_s **p_client; // r9
    loading_t loading; // r11
    const char *v25; // r5
    int v26; // r4
    const char *v27; // r5
    int v28; // r4
    const char *v29; // r5
    int v30; // r4
    int v31[2]; // [sp+50h] [-70h] BYREF
    int v32[26]; // [sp+58h] [-68h] BYREF

    //a12 = randomSeed;
    SV_ResetDemo();
    Swap_Init();
    EntHandle::Init();
    SentientHandle::Init();
    memset(&level, 0, sizeof(level));
    level.cachedTagMat.entnum = ENTITYNUM_NONE;
    level.cachedEntTargetTagMat.entnum = ENTITYNUM_NONE;
    level.currentEntityThink = -1;
    level.initializing = 1;
    level.scriptPrintChannel = 24;
    g_nextMap[0] = 0;
    G_InitDvars();
    ProfLoad_Begin("clear config strings");
    G_ClearAllConfigstrings();
    ProfLoad_End();
    Dvar_SetInt(g_reloading, 0);
    Path_Init(restart);
    v31[0] = loadScripts == 0;
    SaveMemory_CleanupSaveMemory();
    if (savegame)
    {
        G_PreLoadGame(checksum, v31, save);
    }
    else
    {
        SaveMemory_ShutdownSaveSystem();
        SaveMemory_InitializeSaveSystem();
        *save = 0;
    }
    ProfLoad_Begin("GScr_LoadConsts");
    GScr_LoadConsts();
    ProfLoad_End();
    ProfLoad_Begin("G_SetupWeaponDef");
    G_SetupWeaponDef();
    ProfLoad_End();
    if (!SV_InitDemoSavegame(save) || *save)
        CM_LinkWorld();
    else
        level.demoplaying = SV_InitDemo((int*)&randomSeed);
    G_srand(randomSeed);
    level.openScriptIOFileHandles[0] = 0;
    level.openScriptIOFileBuffers[0] = 0;
    level.currentScriptIOLineMark[0].lines = 0;
    level.currentScriptIOLineMark[0].text = 0;
    level.currentScriptIOLineMark[0].ungetToken = 0;
    level.currentScriptIOLineMark[0].backup_lines = 0;
    level.currentScriptIOLineMark[0].backup_text = 0;
    level.soundAliasFirst = 256;
    level.soundAliasLast = 256;
    //level.loading = ((_cntlzw((unsigned int)*save) & 0x20) == 0) + 1;
    level.loading = (loading_t)((((uintptr_t)*save) != 0) + 1);
    if (level.loading == LOADING_SAVEGAME)
    {
        if (!*save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 1488, 0, "%s", "*save");
        SaveMemory_MoveToSegment(*save, 1);
        v17 = Hunk_SetMarkLow();
        if (!G_LoadWeapons(*save))
        {
            SaveMemory_MoveToSegment(*save, 1);
            Hunk_ClearToMarkLow(v17);
            Com_FreeWeaponInfoMemory(1);
            G_SetupWeaponDef();
            if (!G_LoadWeapons(*save))
                Com_Error(ERR_DROP, "Failed to load weapons.\n");
        }
        G_LoadItems(*save);
    }
    if (v31[0])
    {
        actorBackup = g_scr_data.actorBackup;
    }
    else
    {
        memset(&g_scr_data, 0, sizeof(g_scr_data));
        actorBackup = (actorBackup_s *)Hunk_AllocLow(380, "actorBackup", 5);
        g_scr_data.actorBackup = actorBackup;
    }
    memset(actorBackup, 0, sizeof(actorBackup_s));
    if (level.time)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 1517, 0, "%s", "!level.time");
    Mantle_CreateAnims(Hunk_AllocXAnimServer);
    if (restart)
        G_RestartScrVehicleInfo();
    else
        G_ParseScrVehicleInfo();
    level.actorCorpseCount = ai_corpseCount->current.integer;
    if (level.actorCorpseCount < 1 || level.actorCorpseCount > 16)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
            1527,
            1,
            "%s",
            "level.actorCorpseCount >= 1 && level.actorCorpseCount <= MAX_ACTOR_CORPSES");
    ProfLoad_Begin("Load scripts and anims");
    if (!v31[0])
        GScr_LoadScriptsAndAnims();
    if (*save)
    {
        Scr_GetChecksum((unsigned int*)v32);
        Header = SaveMemory_GetHeader(*save);
        if (v32[0] != Header->scrCheckSum[0] || v32[1] != Header->scrCheckSum[1] || v32[2] != Header->scrCheckSum[2])
            Com_Error(ERR_DROP, "Save game saved with different script files");
    }
    ProfLoad_End();
    memset(g_entities, 0, sizeof(g_entities));
    level.gentities = g_entities;
    g_entities[ENTITYNUM_NONE].flags |= FL_OBSTACLE;
    level.maxclients = 1;
    memset(g_clients, 0, sizeof(g_clients));
    level.clients = g_clients;
    level.sentients = g_sentients;
    level.actors = g_actors;
    G_InitSentients();
    G_InitActors();
    maxclients = level.maxclients;
    clients = level.clients;
    if (level.maxclients > 0)
    {
        v22 = level.clients;
        p_client = &g_entities[0].client;
        do
        {
            --maxclients;
            *p_client = v22++;
            p_client += 157;
        } while (maxclients);
    }
    level.num_entities = 1;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
    SV_LocateGameData(level.gentities, 1, 628, &clients->ps, 46104);
    G_ParseHitLocDmgTable();
    BG_LoadPenetrationDepthTable();
    G_InitVehiclePaths();
    G_InitScrVehicles();
    G_InitTurrets();
    Path_PreSpawnInitPaths();
    if (!restart)
        G_DropPathnodesToFloor();
    ProfLoad_Begin("G_SpawnEntitiesFromString");
    G_SpawnEntitiesFromString();
    ProfLoad_End();
    G_UpdateTrackExtraNodes();
    G_DropActorSpawnersToFloor();
    G_SetupVehiclePaths();
    G_SetupScrVehicles();
    level.bRegisterItems = 1;
    level.bDrawCompassFriendlies = 1;
    level.fFogOpaqueDist = FLT_MAX;
    level.fFogOpaqueDistSqrd = FLT_MAX;
    G_InitObjectives();
    G_InitTargets();
    Missile_InitAttractors();
    AimTarget_Init();
    loading = level.loading;
    if (level.loading != LOADING_LEVEL)
    {
        if (level.loading == LOADING_SAVEGAME)
        {
        LABEL_42:
            G_InitLoadGame(*save);
            goto LABEL_43;
        }
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
            1643,
            0,
            "%s",
            "(level.loading == LOADING_LEVEL) || (level.loading == LOADING_SAVEGAME)");
        loading = level.loading;
    }
    if (loading == LOADING_SAVEGAME)
        goto LABEL_42;
LABEL_43:
    Scr_FreeEntityList();
    G_PrintFastFileErrors("code_post_gfx");
    G_PrintFastFileErrors("common");
    G_PrintFastFileErrors(sv_mapname->current.string);
    level.initializing = 0;
}

void __cdecl G_ShutdownGame(int clearScripts)
{
    unsigned __int8 v2; // r11

    SV_ResetDemo();
    Com_DPrintf(15, "ShutdownGame:\n");
    Com_DPrintf(15, "------------------------------------------------------------\n");
    Cmd_UnregisterAllNotifications();
    if (G_LoadErrorCleanup())
    {
        Com_Printf(15, "G_ShutdownGame: save game failed to load\n");
    }
    else
    {
        G_FreeEntities();
        HudElem_DestroyAll();
        Path_Shutdown();
        G_FreeScrVehicles();
    }
    G_FreeVehiclePaths();
    Actor_ClearThreatBiasGroups();
    Scr_SetString(&level.cachedTagMat.name, 0);
    Scr_SetString(&level.cachedEntTargetTagMat.name, 0);
    if (!clearScripts || (v2 = 0, !level.savepersist))
        v2 = 1;
    Scr_ShutdownSystem(1u, v2);
    CL_ClearState();
    if (clearScripts)
        G_ClearLowHunk();
    if (level.openScriptIOFileBuffers[0])
        Z_VirtualFree(level.openScriptIOFileBuffers[0]);
    level.openScriptIOFileBuffers[0] = 0;
    if (level.openScriptIOFileHandles[0])
        FS_FCloseFile(level.openScriptIOFileHandles[0]);
    level.openScriptIOFileHandles[0] = 0;
    if (!G_LoadErrorCleanup())
        EntHandle::Shutdown();
    SV_track_shutdown();
}

void G_ChangeLevel()
{
    gentity_s *v0; // r31
    const char *v1; // r3
    int v2; // r29
    int ConfigstringIndex; // r3
    const char *v4; // r3

    v0 = G_Find(0, 284, scr_const.player);
    if (!v0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 1735, 0, "%s", "player");
    if (v0->health > 0 && !g_reloading->current.integer)
    {
        v1 = va("scr_fade 1 %i %i", level.time + 250, level.exitTime + 750);
        SV_GameSendServerCommand(-1, v1);
        if (level.bMissionSuccess)
        {
            v2 = level.exitTime + 1000;
            ConfigstringIndex = G_FindConfigstringIndex("victoryscreen", 2535, 16, 0, 0);
            v4 = va("setchannelvol %d %d %d", 1, ConfigstringIndex, v2);
        }
        else
        {
            v4 = va("snd_fade 0 %d", level.exitTime + 1000);
        }
        SV_GameSendServerCommand(-1, v4);
        level.reloadDelayTime = level.time + level.exitTime + 1000;
        Dvar_SetInt(g_reloading, 4);
    }
}

bool __cdecl G_IsNextMapWaiting()
{
    return g_reloading->current.integer == 4;
}

void __cdecl G_SetNextMap(const char *mapname)
{
    I_strncpyz(g_nextMap, mapname, 64);
}

void __cdecl G_LoadNextMap()
{
    const char *v0; // r4
    char *v1; // r3

    if (g_nextMap[0])
    {
        if (Dvar_GetBool("sv_cheats"))
            v1 = va("spdevmap %s\n", g_nextMap);
        else
            v1 = va("spmap %s\n", g_nextMap);
        v0 = v1;
    }
    else
    {
        v0 = "disconnect\n";
    }
    Cbuf_AddText(0, v0);
}

void __cdecl G_CheckReloadStatus()
{
    const char *v0; // r3

    if (level.changelevel)
    {
        level.changelevel = 0;
        G_ChangeLevel();
    }
    if (level.reloadDelayTime && level.reloadDelayTime < level.time)
    {
        if (g_reloading->current.integer != 4)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 1804, 0, "%s", "G_IsNextMapWaiting()");
        level.reloadDelayTime = 0;
        if (level.bMissionSuccess)
        {
            level.bMissionSuccess = 0;
            if (level.cinematic[0])
            {
                v0 = va("cinematic %s 0\n", level.cinematic);
                Cbuf_AddText(0, v0);
            }
            else
            {
                SV_GameSendServerCommand(-1, "openvictoryscreen");
            }
            level.cinematic[0] = 0;
        }
        else if (level.bMissionFailed)
        {
            level.bMissionFailed = 0;
            Cbuf_AddText(0, "loadgame_continue\n");
        }
        else
        {
            G_LoadNextMap();
        }
    }
    if (level.absoluteReloadDelayTime && level.absoluteReloadDelayTime - Sys_Milliseconds() < 0)
    {
        if (!g_reloading->current.integer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 1833, 0, "%s", "g_reloading->current.integer");
        level.absoluteReloadDelayTime = 0;
        Cbuf_AddText(0, "loadgame_continue\n");
    }
}

void __cdecl G_ApplyEntityEq(gentity_s *ent)
{
    gentity_s *Player; // r3
    gclient_s *client; // r11
    int hitnum; // r3
    float visBonePos[3]; // [sp+50h] [-40h] BYREF
    float origin[3]; // [sp+60h] [-30h] BYREF
    int eflags;

    if (g_enteqEnable->current.enabled && ent->nexteq <= level.time)
    {
        PROF_SCOPED("ent Eq")
        Player = G_GetPlayer();
        G_GetPlayerViewOrigin(&Player->client->ps, origin);
        if (!G_DObjGetWorldTagPos(ent, scr_const.aim_vis_bone, visBonePos))
        {
            visBonePos[0] = ent->r.currentOrigin[0];
            visBonePos[1] = ent->r.currentOrigin[1];
            visBonePos[2] = ent->r.currentOrigin[2];
        }
        
        if (Vec3DistanceSq(origin, visBonePos) > (g_enteqDist->current.value * g_enteqDist->current.value))
        {
            client = ent->client;
            if (client)
                client->ps.eFlags &= ~0x200000u;
            else
                ent->s.lerp.eFlags &= ~0x200000u;
            return;
        }
        hitnum = CM_BoxSightTrace(0, origin, visBonePos, vec3_origin, vec3_origin, 0, 1);
        client = ent->client;
        if (hitnum)
        {
            if (client)
            {
                client->ps.eFlags &= ~0x200000u;
                ent->nexteq = g_enteqDelay->current.integer + level.time;
                return;
            }
            eflags = ent->s.lerp.eFlags & 0xFFDFFFFF;
        }
        else
        {
            if (client)
            {
                client->ps.eFlags |= 0x200000u;
                ent->nexteq = g_enteqDelay->current.integer + level.time;
                return;
            }
            eflags = ent->s.lerp.eFlags | 0x200000;
        }
        ent->s.lerp.eFlags = eflags;
        ent->nexteq = g_enteqDelay->current.integer + level.time;
        return;
    }
}

void __cdecl G_RunThink(gentity_s *ent)
{
    int nextthink; // r10
    int handler; // r10
    void(__cdecl * think)(gentity_s *); // r31

    nextthink = ent->nextthink;
    if (nextthink > 0 && nextthink <= level.time)
    {
        handler = ent->handler;
        ent->nextthink = 0;
        think = entityHandlers[handler].think;
        if (!think)
            Com_Error(ERR_DROP, "NULL ent think");

        {
            PROF_SCOPED("ent think");
            think(ent);
        }
    }
}

void __cdecl G_DrawEntityBBoxes()
{
    const dvar_s *v4; // r9
    int v5; // r27
    int num_entities; // r11
    float *v7; // r31
    int i; // r30
    double v9; // fp0
    float v10[4]; // [sp+50h] [-70h] BYREF
    float v11[4]; // [sp+60h] [-60h] BYREF
    float v12[6]; // [sp+70h] [-50h] BYREF

    v4 = g_drawEntBBoxes;
    if (g_drawEntBBoxes->current.integer)
    {
#if 0 // KISAKTODO
        v5 = 0;
        v12[0] = 1.0;
        v12[1] = 1.0;
        v12[2] = 1.0;
        v12[3] = 1.0;
        num_entities = level.num_entities;
        if (level.num_entities > 0)
        {
            v7 = &g_entities[0].r.currentOrigin[2];
            do
            {
                if (*((_BYTE *)v7 - 64))
                {
                    if ((unsigned int)(v4->current.integer - 3) <= 1)
                    {
                        for (i = 0; i < 3; ++i)
                        {
                            v10[0] = *(v7 - 2);
                            v10[1] = *(v7 - 1);
                            v10[2] = *v7;
                            v11[0] = *(v7 - 2);
                            v11[1] = *(v7 - 1);
                            v11[2] = *v7;
                            v9 = (float)(v11[i] - (float)4.0);
                            v10[i] = v10[i] + (float)4.0;
                            v11[i] = v9;
                            G_DebugLine(v11, v10, v12, v4->current.integer & 1);
                            v4 = g_drawEntBBoxes;
                        }
                    }
                    G_DebugBox(v7 - 2, v7 - 15, v7 - 12, v7[2], a4, (int)v12, v4->current.integer & 1);
                    v4 = g_drawEntBBoxes;
                    num_entities = level.num_entities;
                }
                ++v5;
                v7 += 157;
            } while (v5 < num_entities);
        }
#endif
    }
}

void DebugDumpAnims()
{
    int integer; // r11
    const char *v1; // r3

    integer = g_dumpAnims->current.integer;
    if (integer >= 0)
    {
        if (integer >= 2176)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                2098,
                1,
                "%s",
                "g_dumpAnims->current.integer < MAX_GENTITIES");
        v1 = va("server: (level time:%d)\n", level.time);
        SV_DObjDisplayAnim(&level.gentities[g_dumpAnims->current.integer], v1);
    }
}

void DebugDumpAIEventListeners()
{
    if (g_dumpAIEventListeners->current.enabled)
    {
        Actor_DumpEventListners();
        Dvar_SetBool(g_dumpAIEventListeners, 0);
    }
}

void __cdecl G_CheckLoadGame(int checksum, SaveGame *save)
{
    loading_t loading; // r11

    loading = level.loading;
    if (level.loading == LOADING_LEVEL)
    {
    LABEL_4:
        if (loading != LOADING_SAVEGAME)
            goto LABEL_8;
        goto LABEL_5;
    }
    if (level.loading != LOADING_SAVEGAME)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
            2198,
            0,
            "%s",
            "(level.loading == LOADING_LEVEL) || (level.loading == LOADING_SAVEGAME)");
        loading = level.loading;
        goto LABEL_4;
    }
LABEL_5:
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2202, 0, "%s", "save");
    SV_ClearPendingSaves();
    G_LoadGame(checksum, save);
LABEL_8:
    SV_GameSendServerCommand(-1, "snd_fade 1 0");
    SV_GameSendServerCommand(-1, "scr_fade 0 0 0");
    SV_GameSendServerCommand(-1, "clear_blur");
    SV_GameSendServerCommand(-1, "time_slow 0 1 1");
    SV_GameSendServerCommand(-1, "closedeadscreen");
    level.absoluteReloadDelayTime = 0;
    Dvar_SetInt(g_reloading, 0);
    level.loading = LOADING_DONE;
}

void __cdecl G_XAnimUpdateEnt(gentity_s *ent)
{
    while (ent->r.inuse)
    {
        if ((ent->flags & 0x1000) != 0)
            break;
        if (!G_DObjUpdateServerTime(ent, 1))
            break;
        Scr_RunCurrentThreads();
    }
}

void __cdecl G_ClientDoPerFrameNotifies(gentity_s *ent)
{
    gclient_s *client; // r31
    unsigned int weapon; // r3
    WeaponDef *WeaponDef; // r3
    char v5; // r11
    char v6; // r30
    unsigned __int16 begin_firing; // r4
    bool *p_previouslyUsingNightVision; // r11

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2244, 0, "%s", "ent");
    client = ent->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2248, 0, "%s", "client");
    weapon = client->ps.weapon;
    if (weapon != client->lastWeapon)
    {
        WeaponDef = BG_GetWeaponDef(weapon);
        Scr_AddString(WeaponDef->szInternalName);
        Scr_Notify(ent, scr_const.weapon_change, 1u);
        client->lastWeapon = client->ps.weapon;
    }
    if (client->ps.weaponstate != 5 || (v5 = 1, client->ps.pm_type >= 5))
        v5 = 0;
    v6 = v5;
    if (v5 != client->previouslyFiring)
    {
        if (v5)
            begin_firing = scr_const.begin_firing;
        else
            begin_firing = scr_const.end_firing;
        Scr_Notify(ent, begin_firing, 0);
        client->previouslyFiring = v6;
    }
    p_previouslyUsingNightVision = &client->previouslyUsingNightVision;
    if ((client->ps.weapFlags & 0x40) != 0)
    {
        if (!client->previouslyUsingNightVision)
        {
            *p_previouslyUsingNightVision = 1;
            Scr_Notify(ent, scr_const.night_vision_on, 0);
        }
    }
    else if (client->previouslyUsingNightVision)
    {
        *p_previouslyUsingNightVision = 0;
        Scr_Notify(ent, scr_const.night_vision_off, 0);
    }
}

void __cdecl G_RunFrameForEntityInternal(gentity_s *ent)
{
    int eventTime; // r10
    int eType; // r11
    gentity_s *v4; // r3
    int v5; // r11

    eventTime = ent->r.eventTime;
    if (eventTime && level.time - eventTime > 200)
    {
        if ((ent->r.eventType & 1) != 0)
        {
            G_FreeEntity(ent);
            return;
        }
        if ((ent->r.eventType & 2) != 0)
        {
            ent->r.eventType &= ~2u;
            SV_UnlinkEntity(ent);
        }
        ent->r.eventTime = 0;
    }
    eType = ent->s.eType;
    if (eType == 3)
    {
        G_RunMissile(ent);
        return;
    }
    if (eType == 2)
    {
        v4 = ent;
        if (ent->tagInfo)
        {
            G_GeneralLink(ent);
            G_RunThink(ent);
            return;
        }
        goto LABEL_16;
    }
    if ((unsigned __int8)Com_IsRagdollTrajectory(&ent->s.lerp.pos))
    {
    LABEL_23:
        G_RunThink(ent);
        return;
    }
    if (ent->physicsObject)
    {
        v4 = ent;
    LABEL_16:
        G_RunItem(v4);
        return;
    }
    v5 = ent->s.eType;
    if (v5 != 5 && v5 != 9)
    {
        if (ent->client)
            return;
        if (!ent->s.eType)
        {
            if (ent->tagInfo)
                G_GeneralLink(ent);
        }
        goto LABEL_23;
    }
    G_RunMover(ent);
}

void __cdecl G_RunFrameForEntity(gentity_s *ent)
{
    tagInfo_s *tagInfo; // r10
    const char *v3; // r3
    const char *v4; // r3
    const char *v5; // r3
    const char *v6; // r3
    const char *v7; // r3
    const char *v8; // r3
    scr_vehicle_s *scr_vehicle; // r10
    int v10; // r4
    const char *v11; // r7
    int duration; // r11
    __int64 v13; // r10
    _QWORD v14[10]; // [sp+50h] [-50h] BYREF

    if (!ent->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2374, 0, "%s", "ent->r.inuse");
    if (ent->processedFrame != level.framenum)
    {
        tagInfo = ent->tagInfo;
        ent->processedFrame = level.framenum;
        if (tagInfo)
        {
            iassert(ent->tagInfo->parent);
            G_RunFrameForEntity(ent->tagInfo->parent);
        }
        iassert(((ent->r.svFlags & ((1 << 1) | (1 << 2))) != ((1 << 1) | (1 << 2))));
        iassert(ent->r.maxs[0] >= ent->r.mins[0]);
        iassert(ent->r.maxs[1] >= ent->r.mins[1]);
        iassert(ent->r.maxs[2] >= ent->r.mins[2]);
        
        scr_vehicle = ent->scr_vehicle;
        if (scr_vehicle)
        {
            if ((unsigned int)(scr_vehicle - level.vehicles) >= 0x40)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                    2403,
                    0,
                    "%s",
                    "(unsigned)( ent->scr_vehicle - level.vehicles ) < MAX_VEHICLES");
            if (ent->scr_vehicle->entNum != ENTITYNUM_NONE)
                goto LABEL_24;
            v10 = 2404;
            v11 = "ent->scr_vehicle->entNum != ENTITYNUM_NONE";
        }
        else
        {
            if (ent->s.eType != 11)
            {
            LABEL_24:
                if (ent->snd_wait.notifyString)
                {
                    duration = ent->snd_wait.duration;
                    if (duration < 0)
                        duration = 5000;
                    if (level.time - ent->snd_wait.basetime - duration >= 0)
                    {
                        Scr_Notify(ent, ent->snd_wait.notifyString, 0);
                        Scr_SetString(&ent->snd_wait.notifyString, 0);
                    }
                }
                G_RunFrameForEntityInternal(ent);
                if (ent->s.eType == 14 || (ent->s.lerp.eFlags & 0x800) != 0)
                    AimTarget_ProcessEntity(ent);
                if (snd_enableEq->current.enabled)
                {
                    switch (ent->s.eType)
                    {
                    case 3u:
                    case 6u:
                    case 0xAu:
                    case 0xEu:
                        G_ApplyEntityEq(ent);
                        break;
                    default:
                        break;
                    }
                }
                if (g_debugLocDamage->current.enabled)
                {
                    if (SV_DObjExists(ent))
                    {
                        LODWORD(v13) = -1;
                        v14[0] = v13;
                        v14[1] = v13;
                        G_DObjCalcPose(ent, (int *)v14);
                        SV_XModelDebugBoxes(ent);
                    }
                }
                return;
            }
            v10 = 2408;
            v11 = "ent->s.eType != ET_VEHICLE";
        }
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", v10, 0, "%s", v11);
        goto LABEL_24;
    }
}

void __cdecl G_RunPreFrame()
{
    level.previousTime = level.time;
    ++level.framenum;
    level.time += 50;
}

int __cdecl G_GetFramePos()
{
    return level.framePos;
}

int __cdecl NotifyTriggers()
{
    int v0; // r18
    int v1; // r25
    trigger_info_t *i; // r30
    int entnum; // r27
    trigger_info_t *v4; // r29
    gentity_s *v5; // r26
    gentity_s *v6; // r28
    trigger_info_t *v7; // r11

    v0 = 0;
    v1 = 0;
    ++level.triggerIndex;
    if (level.currentTriggerListSize > 0)
    {
        for (i = level.currentTriggerList; ; ++i)
        {
            entnum = i->entnum;
            v4 = i;
            v5 = &g_entities[entnum];
            if (v5->s.lerp.useCount != i->useCount)
                goto LABEL_12;
            if (!v5->r.inuse)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2551, 0, "%s", "ent->r.inuse");
            v6 = &g_entities[i->otherEntnum];
            if (v6->s.lerp.useCount != i->otherUseCount)
                goto LABEL_12;
            if (!v6->r.inuse)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2557, 0, "%s", "other->r.inuse");
            if (level.entTriggerIndex[entnum] != level.triggerIndex)
                break;
            v0 = 1;
        LABEL_13:
            if (++v1 >= level.currentTriggerListSize)
                return v0;
        }
        level.entTriggerIndex[entnum] = level.triggerIndex;
        Scr_AddEntity(v6);
        Scr_Notify(&g_entities[entnum], scr_const.trigger, 1u);
    LABEL_12:
        --v1;
        --i;
        v7 = &level.currentTriggerList[--level.currentTriggerListSize];
        *(unsigned int *)&v4->entnum = *(unsigned int *)&v7->entnum;
        v4->useCount = v7->useCount;
        v4->otherUseCount = v7->otherUseCount;
        goto LABEL_13;
    }
    return v0;
}

void __cdecl G_SendClientMessages()
{
    int v0; // r28
    int v1; // r25
    actor_prone_info_s *cgData_actorProneInfo; // r27
    actor_s *v3; // r30
    actor_prone_info_s *p_ProneInfo; // r11
    actor_prone_info_s *v5; // r10
    int v6; // ctr
    unsigned int ActorFriendlyIndex; // r3
    unsigned int v8; // r29
    unsigned __int8 *v9; // r11
    unsigned __int8 v10; // r10
    actor_prone_info_s *v11; // r9
    int *p_entnum; // r11
    int v13; // r4
    int *v14; // r8
    actor_prone_info_s *v15; // r7
    int v16; // ctr
    int v17; // r5
    int *v18; // r8
    actor_prone_info_s *v19; // r7
    int v20; // ctr
    int v21; // r6
    int *v22; // r8
    actor_prone_info_s *v23; // r7
    int v24; // ctr
    int v25; // r6
    int *v26; // r8
    actor_prone_info_s *v27; // r7
    int v28; // ctr
    int v29; // r8
    int v30; // r9
    int entNum; // r11
    int v32; // r11
    int v33; // r11
    int v34; // r11

    level.snapTime = level.time;
    memset(level.specialIndex, 255, sizeof(level.specialIndex));
    AimTarget_UpdateClientTargets();
    v0 = 0;
    v1 = 0;
    cgData_actorProneInfo = level.cgData_actorProneInfo;
    do
    {
        v3 = &level.actors[v1];
        if (level.actors[v1].inuse)
        {
            p_ProneInfo = &v3->ProneInfo;
            v5 = cgData_actorProneInfo;
            level.specialIndex[v3->ent->s.number] = v0;
            v6 = 6;
            do
            {
                *(unsigned int *)&v5->bCorpseOrientation = *(unsigned int *)&p_ProneInfo->bCorpseOrientation;
                p_ProneInfo = (actor_prone_info_s *)((char *)p_ProneInfo + 4);
                v5 = (actor_prone_info_s *)((char *)v5 + 4);
                --v6;
            } while (v6);
            ActorFriendlyIndex = G_GetActorFriendlyIndex(v3->ent->s.number);
            v8 = ActorFriendlyIndex;
            if (ActorFriendlyIndex != -1 && ActorFriendlyIndex != v0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                    2961,
                    0,
                    "%s",
                    "actorFriendlyIndex == -1 || actorFriendlyIndex == i");
            v9 = &level.cgData_actorTeam[v0];
            //*(v9 - 32) = (_cntlzw(v8 + 1) & 0x20) == 0;
            *(v9 - 32) = (v8 + 1) != 0;
            *v9 = v3->sentient->eTeam;
            if (level.cgData_actorTeam[v0] != v3->sentient->eTeam)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                    2964,
                    0,
                    "%s\n\t(actor->sentient->eTeam) = %i",
                    "(level.cgData_actorTeam[i] == actor->sentient->eTeam)",
                    v3->sentient->eTeam);
        }
        ++cgData_actorProneInfo;
        ++v0;
        ++v1;
    } while ((int)cgData_actorProneInfo < (int)&level.cgData_actorProneInfo[32]);
    v10 = 33;
    v11 = &level.cgData_actorProneInfo[33];
    p_entnum = &g_scr_data.actorCorpseInfo[0].entnum;
    v13 = 4;
    do
    {
        if (*p_entnum >= 0)
        {
            v14 = p_entnum + 1;
            v15 = v11 - 1;
            level.specialIndex[*p_entnum] = v10 - 1;
            v16 = 6;
            do
            {
                *(unsigned int *)&v15->bCorpseOrientation = *v14++;
                v15 = (actor_prone_info_s *)((char *)v15 + 4);
                --v16;
            } while (v16);
        }
        v17 = p_entnum[8];
        if (v17 >= 0)
        {
            v18 = p_entnum + 9;
            v19 = v11;
            level.specialIndex[v17] = v10;
            v20 = 6;
            do
            {
                *(unsigned int *)&v19->bCorpseOrientation = *v18++;
                v19 = (actor_prone_info_s *)((char *)v19 + 4);
                --v20;
            } while (v20);
        }
        v21 = p_entnum[16];
        if (v21 >= 0)
        {
            v22 = p_entnum + 17;
            v23 = v11 + 1;
            level.specialIndex[v21] = v10 + 1;
            v24 = 6;
            do
            {
                *(unsigned int *)&v23->bCorpseOrientation = *v22++;
                v23 = (actor_prone_info_s *)((char *)v23 + 4);
                --v24;
            } while (v24);
        }
        v25 = p_entnum[24];
        if (v25 >= 0)
        {
            v26 = p_entnum + 25;
            v27 = v11 + 2;
            level.specialIndex[v25] = v10 + 2;
            v28 = 6;
            do
            {
                *(unsigned int *)&v27->bCorpseOrientation = *v26++;
                v27 = (actor_prone_info_s *)((char *)v27 + 4);
                --v28;
            } while (v28);
        }
        --v13;
        v10 += 4;
        p_entnum += 32;
        v11 += 4;
    } while (v13);
    v29 = 2;
    v30 = 0;
    do
    {
        entNum = level.vehicles[v30].entNum;
        if (entNum != ENTITYNUM_NONE)
            level.specialIndex[entNum] = v29 - 2;
        v32 = level.vehicles[v30 + 1].entNum;
        if (v32 != ENTITYNUM_NONE)
            level.specialIndex[v32] = v29 - 1;
        v33 = level.vehicles[v30 + 2].entNum;
        if (v33 != ENTITYNUM_NONE)
            level.specialIndex[v33] = v29;
        v34 = level.vehicles[v30 + 3].entNum;
        if (v34 != ENTITYNUM_NONE)
            level.specialIndex[v34] = v29 + 1;
        v29 += 4;
        v30 += 4;
    } while (v29 - 2 < 64);
    if (level.bRegisterItems)
        SaveRegisteredItems();
}

void __cdecl G_ArchiveSpecialEntityInfo(const entityState_s *es, MemoryFile *memFile)
{
    int eType; // r11
    int v5; // r30

    MemFile_ArchiveData(memFile, 1, &level.specialIndex[es->number]);
    eType = es->eType;
    v5 = level.specialIndex[es->number];
    if (eType == 14)
    {
        MemFile_ArchiveData(memFile, 24, &level.cgData_actorProneInfo[v5]);
        MemFile_ArchiveData(memFile, 1, &level.cgData_actorOnCompass[v5]);
        MemFile_ArchiveData(memFile, 1, &level.cgData_actorTeam[v5]);
    }
    else if (eType == 16)
    {
        MemFile_ArchiveData(memFile, 24, &level.cgData_actorProneInfo[v5]);
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
    PROF_SCOPED("G_TraceCapsule");

    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-Ch] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    SV_Trace(results, start, mins, maxs, end, &ignoreEntParams, contentmask, 0, 0, 0);
}

int __cdecl G_TraceCapsuleComplete(
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask)
{
    return SV_TracePassed(start, mins, maxs, end, passEntityNum, ENTITYNUM_NONE, contentmask, 0, 0, 0);
}

void __cdecl G_LocationalTrace(
    trace_t *results,
    const float *start,
    const float *end,
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
    const float *start,
    const float *end,
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
    const float *start,
    const float *end,
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

void __cdecl G_SightTrace(int *hitNum, const float *start, const float *end, int passEntityNum, int contentmask)
{
    SV_SightTrace(hitNum, start, vec3_origin, vec3_origin, end, passEntityNum, ENTITYNUM_NONE, contentmask);
}

void __cdecl G_AddDebugString(const float *xyz, const float *color, double scale, const char *pszText)
{
    CL_AddDebugString(xyz, color, scale, pszText, 0, 1);
}

void __cdecl G_AddDebugStringWithDuration(
    const float *xyz,
    const float *color,
    double scale,
    const char *pszText,
    int duration)
{
    CL_AddDebugString(xyz, color, scale, pszText, duration, 1);
}

static int lastEntTime;
void __cdecl ShowEntityInfo()
{
    // KISAKTODO: broken sv_trace
#if 0
    int integer; // r11
    int v1; // r30
    int num_entities; // r10
    entityShared_t *p_r; // r31
    int hitnum; // r11
    float client; // r11
    void(__cdecl * v6)(gentity_s *, float *); // r11
    int visBonePos; // r11
    int v8; // r31
    int EntityHitId; // r4
    int time; // r10
    int v11; // r11
    int v12; // r9
    gentity_s *v13; // r31
    void(__cdecl * entinfo)(gentity_s *, float *); // r11
    unsigned __int8 *v15; // [sp+8h] [-F8h]
    int v16; // [sp+Ch] [-F4h]
    float v17[4]; // [sp+60h] [-A0h] BYREF
    float v18[4]; // [sp+70h] [-90h] BYREF
    float v19[4]; // [sp+80h] [-80h] BYREF
    IgnoreEntParams v20; // [sp+90h] [-70h] BYREF
    trace_t v21[2]; // [sp+A0h] [-60h] BYREF

    if (g_entinfo->current.integer)
    {
        CL_GetViewPos(v17);
        integer = g_entinfo->current.integer;
        if (integer == 2 || integer == 3 || integer == 5)
        {
            CL_GetViewForward(v18);
            v19[0] = (float)(v18[0] * (float)16000.0) + v17[0];
            v19[1] = (float)(v18[1] * (float)16000.0) + v17[1];
            v19[2] = (float)(v18[2] * (float)16000.0) + v17[2];
            visBonePos = g_entinfo_type->current.integer;
            if (visBonePos == 1)
            {
                v8 = 0x4000;
            }
            else
            {
                v8 = 0x800000;
                if (visBonePos != 2)
                    v8 = 8413312;
            }
            SV_SetupIgnoreEntParams(&v20, 0);
            SV_Trace(v21, v17, (float*)vec3_origin, (float *)vec3_origin, v19, &v20, v8, 0, v15, v16);
            EntityHitId = Trace_GetEntityHitId(v21);
            if (entityHandlers[g_entities[EntityHitId].handler].entinfo)
            {
                Dvar_SetInt(ai_debugEntIndex, EntityHitId);
                time = level.time;
                v11 = level.time;
                lastEntTime = level.time;
            }
            else
            {
                v11 = lastEntTime;
                time = level.time;
            }
            v12 = ai_debugEntIndex->current.integer;
            if (v12 != -1 && v11 + 30000 > time)
            {
                v13 = &g_entities[v12];
                if (v13->actor && ai_debugCoverEntityNum->current.integer > 0)
                    Dvar_SetInt(ai_debugCoverEntityNum, v13->s.number);
                entinfo = entityHandlers[v13->handler].entinfo;
                if (entinfo)
                    entinfo(v13, v17);
            }
        }
        else
        {
            v1 = 0;
            num_entities = level.num_entities;
            if (level.num_entities > 0)
            {
                p_r = &g_entities[0].r;
                do
                {
                    if (!p_r->inuse || !p_r->linked)
                        goto LABEL_16;
                    hitnum = g_entinfo_type->current.integer;
                    if (hitnum == 1)
                    {
                        client = *(float *)&p_r[1].inuse;
                    }
                    else
                    {
                        if (hitnum != 2)
                            goto LABEL_14;
                        client = p_r[1].mins[1];
                    }
                    if (client != 0.0)
                    {
                    LABEL_14:
                        v6 = entityHandlers[BYTE2(p_r[1].maxs[1])].entinfo;
                        if (v6)
                        {
                            v6((gentity_s *)p_r[-2].maxs, v17);
                            num_entities = level.num_entities;
                        }
                    }
                LABEL_16:
                    ++v1;
                    p_r = (entityShared_t *)((char *)p_r + 628);
                } while (v1 < num_entities);
            }
        }
    }
#endif
}

int __cdecl G_RunFrame(ServerFrameExtent extent, int timeCap)
{
    int currentIndex; // r5
    int num_entities; // r11
    gentity_s *i; // r30
    int number; // r4
    const char *v7; // r3
    int v8; // r29
    gentity_s *v9; // r30
    int v10; // r29
    int j; // r11
    int v12; // r11
    gentity_s *k; // r30
    int inuse; // r11
    bool checkAnimChange; // r11
    unsigned __int8 *p_inuse; // r30
    int v17; // r10
    int v18; // r10
    int v19; // r3
    gentity_s *vehEnt; // r3
    gentity_s *v21; // r30
    int currentEntityThink; // r5
    int v23; // r11
    gentity_s *m; // r30
    int v25; // r4
    const char *v26; // r3
    int maxclients; // r11
    int v28; // r29
    gclient_s **p_client; // r30
    const float *v30; // r6
    int v31; // r5
    int v32; // r4
    int v33; // r3
    __int64 v34; // r8
    int v35; // r6
    int v36; // r5
    const float *v37; // r4
    int v38; // r3
    int v39; // r30
    unsigned __int16 *p_classname; // r31
    const char *v41; // r3

    iassert(extent == SV_FRAME_DO_ALL);
    iassert(!timeCap);

    {
        PROF_SCOPED("update player");
        G_UpdatePlayer(level.gentities);
        G_UpdatePlayerTriggers(level.gentities);
        G_CheckReloadStatus();
        iassert(!level.actorPredictDepth);
        level.currentIndex = 1;
    }
    {
        PROF_SCOPED("update anim1");
        currentIndex = level.currentIndex;
        num_entities = level.num_entities;
        for (i = &g_entities[level.currentIndex]; currentIndex < num_entities; ++i)
        {
            if (i->r.inuse)
            {
                number = i->s.number;
                if (number != currentIndex)
                {
                    v7 = va(
                        "s.number = %i, level.currentIndex = %i, classname = %s",
                        number,
                        currentIndex,
                        (const char *)i->classname);
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                        2643,
                        0,
                        "%s\n\t%s",
                        "ent->s.number == level.currentIndex",
                        v7);
                }
                SV_DObjInitServerTime(i, 0.05f);
                currentIndex = ++level.currentIndex;
                num_entities = level.num_entities;
            }
            else
            {
                level.currentIndex = ++currentIndex;
            }
        }
    }
    {
        PROF_SCOPED("script threads1");
        memset(level.entTriggerIndex, 0, sizeof(level.entTriggerIndex));
        level.triggerIndex = 0;
        iassert(level.currentTriggerListSize == 0);
        Com_Memcpy(level.currentTriggerList, level.pendingTriggerList, 12 * level.pendingTriggerListSize);
        level.currentTriggerListSize = level.pendingTriggerListSize;
        level.pendingTriggerListSize = 0;
        do
        {
            v8 = NotifyTriggers();
            G_ProcessCommandNotifies();
            Scr_RunCurrentThreads();
        } while (v8);
        iassert(level.currentTriggerListSize == 0);
        v9 = g_entities;
        v10 = 0;
        level.currentIndex = 1;
        level.checkAnimChange = 0;
        for (j = level.maxclients; v10 < j; ++v9)
        {
            if (v9->r.inuse)
            {
                G_ClientDoPerFrameNotifies(v9);
                j = level.maxclients;
            }
            ++v10;
        }
    }
    {
        PROF_SCOPED("update anim2");
        v12 = level.currentIndex;
        for (k = &g_entities[level.currentIndex]; v12 < level.num_entities; level.currentIndex = v12)
        {
            if (k->r.inuse)
            {
                do
                {
                    if ((k->flags & 0x1000) != 0)
                        break;
                    if (!G_DObjUpdateServerTime(k, 1))
                        break;
                    Scr_RunCurrentThreads();
                    inuse = k->r.inuse;
                    level.checkAnimChange = 1;
                } while (inuse);
                v12 = level.currentIndex;
            }
            ++v12;
            ++k;
        }
        if (level.checkAnimChange)
        {
            do
            {
                checkAnimChange = 0;
                level.currentIndex = 1;
                level.checkAnimChange = 0;
                if (level.num_entities > 1)
                {
                    p_inuse = &g_entities[1].r.inuse;
                    do
                    {
                        if (*p_inuse)
                        {
                            do
                            {
                                v17 = *((unsigned int *)p_inuse + 34);
                                if ((v17 & 0x40000) == 0 || (v17 & 0x1000) != 0)
                                    break;
                                if (!G_DObjUpdateServerTime((gentity_s *)(p_inuse - 168), 1))
                                {
                                    checkAnimChange = level.checkAnimChange;
                                    break;
                                }
                                Scr_RunCurrentThreads();
                                checkAnimChange = 1;
                                v18 = *p_inuse;
                                level.checkAnimChange = 1;
                            } while (v18);
                        }
                        p_inuse += 628;
                        ++level.currentIndex;
                    } while (level.currentIndex < level.num_entities);
                }
            } while (checkAnimChange);
        }
    }
    {
        PROF_SCOPED("script threads2");
        if (g_recordScriptPlace->current.enabled || (v19 = 0, g_dumpAnimsCommands->current.integer > 0))
            v19 = 1;
        Scr_SetRecordScriptPlace(v19);
        Scr_IncTime();
        SV_ResetSkeletonCache();
    }
    {
        PROF_SCOPED("bad places");
        Path_RunBadPlaces();
    }
    {
        PROF_SCOPED("update player");
        G_UpdatePlayer(level.gentities);
        AimTarget_ClearTargetList();
        vehEnt = G_GetPlayerVehicle(level.gentities);
        v21 = vehEnt;
        if (vehEnt)
        {
            if (!vehEnt->r.inuse)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp", 2811, 0, "%s", "vehEnt->r.inuse");
            G_RunFrameForEntity(v21);
        }
    }
    iassert(level.currentEntityThink == -1);
    level.currentEntityThink = 0;
    {
        PROF_SCOPED("update ents");
        currentEntityThink = level.currentEntityThink;
        v23 = level.num_entities;
        for (m = &g_entities[level.currentEntityThink]; currentEntityThink < v23; ++m)
        {
            if (m->r.inuse)
            {
                v25 = m->s.number;
                if (v25 != currentEntityThink)
                {
                    v26 = va(
                        "s.number = %i, level.currentEntityThink = %i, classname = %s",
                        v25,
                        currentEntityThink,
                        (const char *)m->classname);
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_main.cpp",
                        2844,
                        0,
                        "%s\n\t%s",
                        "ent->s.number == level.currentEntityThink",
                        v26);
                }
                G_RunFrameForEntity(m);
                currentEntityThink = ++level.currentEntityThink;
                v23 = level.num_entities;
            }
            else
            {
                level.currentEntityThink = ++currentEntityThink;
            }
        }
    }
    level.currentEntityThink = -1;
    if (level.actorPredictDepth)
        Com_Error(ERR_DROP, "unmatching beginPrediction and endPrediction");

    {
        PROF_SCOPED("update client");

        for (int i = 0; i < level.maxclients; i++)
        {
            gentity_s *ent = &g_entities[i];

            if (ent->r.inuse)
            {
                HudElem_UpdateClient(ent->client);
                ClientEndFrame(ent);
            }
        }
        //maxclients = level.maxclients;
        //v28 = 0;
        //if (level.maxclients > 0)
        //{
        //    p_client = &g_entities[0].client;
        //    do
        //    {
        //        if (*((_BYTE *)p_client - 88))
        //        {
        //            HudElem_UpdateClient(*p_client);
        //            ClientEndFrame((gentity_s *)(p_client - 64));
        //            maxclients = level.maxclients;
        //        }
        //        ++v28;
        //        p_client += 157;
        //    } while (v28 < maxclients);
        //}
    }
    {
        PROF_SCOPED("update corpses");
        G_UpdateActorCorpses();
    }
    Path_DrawDebug();
    G_DrawVehiclePaths();
    G_DrawEntityBBoxes();
    G_DrawGrenadeHints();
    if (g_listEntity->current.enabled)
    {
        v39 = 0;
        p_classname = &g_entities[0].classname;
        do
        {
            v41 = SL_ConvertToString(*p_classname);
            Com_Printf(15, "%4i: %s\n", v39, v41);
            p_classname += 314;
            ++v39;
        } while ((int)p_classname < (int)&ai_showFriendlyChains);
        Dvar_SetBool(g_listEntity, 0);
    }
    ShowEntityInfo();
    DebugDumpAnims();
    if (g_dumpAIEventListeners->current.enabled)
    {
        Actor_DumpEventListners();
        Dvar_SetBool(g_dumpAIEventListeners, 0);
    }
    return 1;
}

void __cdecl G_LoadLevel()
{
    iassert((level.loading == LOADING_LEVEL) || (level.loading == LOADING_SAVEGAME));

    level.initializing = 1;

    {
        PROF_SCOPED("G_PrecacheDefaultModels");
        G_PrecacheDefaultModels();
    }
    {
        PROF_SCOPED("Scr_InitSystem");
        Scr_InitSystem(1);
    }
    {
        PROF_SCOPED("Scr_SetLoading");
        Scr_SetLoading(1);
    }
    {
        PROF_SCOPED("Scr_AllocGameVariable");
        Scr_AllocGameVariable();
    }
    {
        PROF_SCOPED("ClientBegin");
        ClientBegin(0);
    }
    {
        PROF_SCOPED("G_LoadStructs");
        G_LoadStructs();
    }
    {
        PROF_SCOPED("Actor_FinishSpawningAll");
        Actor_FinishSpawningAll();
    }
    {
        PROF_SCOPED("Path_AutoDisconnectPaths");
        Path_AutoDisconnectPaths();
    }
    {
        PROF_SCOPED("Scr_LoadLevel");
        Scr_LoadLevel();
    }
    {
        PROF_SCOPED("G_RunFrame");
        G_RunFrame(SV_FRAME_DO_ALL, 0);
    }
    {
        PROF_SCOPED("Scr_SetLoading");
        Scr_SetLoading(0);
    }

    iassert(level.gentities[0].client);
    level.initializing = 0;

    {
        PROF_SCOPED("G_SendClientMessages");
        G_SendClientMessages();
    }
}

