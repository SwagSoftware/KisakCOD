#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <script/scr_animtree.h>

#include "g_scr_main.h"
#include "g_public.h"
#include <script/scr_stringlist.h>
#include <qcommon/mem_track.h>
#include <script/scr_vm.h>
#include "g_local.h"
#include "pathnode.h"
#include <server/sv_game.h>

#include "actor_script_cmd.h"
#include "g_vehicle_path.h"
#include "g_main.h"
#include <stringed/stringed_hooks.h>
#include <script/scr_const.h>
#include "turret.h"
#include <xanim/dobj_utils.h>
#include "actor_spawner.h"
#include "actor_grenade.h"
#include "bullet.h"
#include <server/sv_public.h>
#include <universal/com_sndalias.h>
#include <universal/surfaceflags.h>
#include <database/database.h>
#include "savememory.h"
#include <gfx_d3d/r_cinematic.h>
#include "actor_corpse.h"
#include <cgame/cg_view.h>
#include <universal/com_files.h>
#include <qcommon/cmd.h>
#include "actor_event_listeners.h"
#include <cgame/cg_ents.h>
#include <script/scr_main.h>
#include "actor_state.h"
#include <script/scr_memorytree.h>

const BuiltinMethodDef methods[104] =
{
  { "startscriptedanim", ActorCmd_StartScriptedAnim, 0 },
  { "startcoverarrival", ActorCmd_StartCoverArrival, 0 },
  { "starttraversearrival", ActorCmd_StartTraverseArrival, 0 },
  { "checkcoverexitposwithpath", ActorCmd_CheckCoverExitPosWithPath, 0 },
  { "shoot", ActorCmd_Shoot, 0 },
  { "shootblank", ActorCmd_ShootBlank, 0 },
  { "melee", ActorCmd_Melee, 0 },
  { "updateplayersightaccuracy", ActorCmd_UpdatePlayerSightAccuracy, 0 },
  { "findcovernode", ActorCmd_FindCoverNode, 0 },
  { "findbestcovernode", ActorCmd_FindBestCoverNode, 0 },
  { "getcovernode", ActorCmd_GetCoverNode, 0 },
  { "usecovernode", ActorCmd_UseCoverNode, 0 },
  { "reacquirestep", ActorCmd_ReacquireStep, 0 },
  { "findreacquirenode", ActorCmd_FindReacquireNode, 0 },
  { "getreacquirenode", ActorCmd_GetReacquireNode, 0 },
  { "usereacquirenode", ActorCmd_UseReacquireNode, 0 },
  { "findreacquiredirectpath", ActorCmd_FindReacquireDirectPath, 0 },
  { "trimpathtoattack", ActorCmd_TrimPathToAttack, 0 },
  { "reacquiremove", ActorCmd_ReacquireMove, 0 },
  { "findreacquireproximatepath", ActorCmd_FindReacquireProximatePath, 0 },
  { "flagenemyunattackable", ActorCmd_FlagEnemyUnattackable, 0 },
  { "setaimanims", ActorCmd_SetAimAnims, 0 },
  { "aimatpos", ActorCmd_AimAtPos, 0 },
  { "enterprone", ActorCmd_EnterProne, 0 },
  { "exitprone", ActorCmd_ExitProne, 0 },
  { "setproneanimnodes", ActorCmd_SetProneAnimNodes, 0 },
  { "updateprone", ActorCmd_UpdateProne, 0 },
  { "clearpitchorient", ActorCmd_ClearPitchOrient, 0 },
  { "setlookatanimnodes", ActorCmd_SetLookAtAnimNodes, 0 },
  { "setlookat", ActorCmd_SetLookAt, 0 },
  { "setlookatyawlimits", ActorCmd_SetLookAtYawLimits, 0 },
  { "stoplookat", ActorCmd_StopLookAt, 0 },
  { "canshoot", ActorCmd_CanShoot, 0 },
  { "cansee", ActorCmd_CanSee, 0 },
  { "dropweapon", ActorCmd_DropWeapon, 0 },
  { "maymovetopoint", ActorCmd_MayMoveToPoint, 0 },
  { "maymovefrompointtopoint", ActorCmd_MayMoveFromPointToPoint, 0 },
  { "teleport", ActorCmd_Teleport, 0 },
  { "withinapproxpathdist", ActorCmd_WithinApproxPathDist, 0 },
  { "ispathdirect", ActorCmd_IsPathDirect, 0 },
  { "allowedstances", ActorCmd_AllowedStances, 0 },
  { "isstanceallowed", ActorCmd_IsStanceAllowed, 0 },
  { "issuppressionwaiting", ActorCmd_IsSuppressionWaiting, 0 },
  { "issuppressed", ActorCmd_IsSuppressed, 0 },
  { "ismovesuppressed", ActorCmd_IsMoveSuppressed, 0 },
  { "checkgrenadethrow", ActorCmd_CheckGrenadeThrow, 0 },
  { "checkgrenadelaunch", ActorCmd_CheckGrenadeLaunch, 0 },
  { "checkgrenadelaunchpos", ActorCmd_CheckGrenadeLaunchPos, 0 },
  { "firegrenadelauncher", ActorCmd_FireGrenadeLauncher, 0 },
  { "throwgrenade", ActorCmd_ThrowGrenade, 0 },
  { "pickupgrenade", ActorCmd_PickUpGrenade, 0 },
  { "useturret", ActorCmd_UseTurret, 0 },
  { "stopuseturret", ActorCmd_StopUseTurret, 0 },
  { "canuseturret", ActorCmd_CanUseTurret, 0 },
  { "traversemode", ActorCmd_TraverseMode, 0 },
  { "animmode", ActorCmd_AnimMode, 0 },
  { "orientmode", ActorCmd_OrientMode, 0 },
  { "getmotionangle", ActorCmd_GetMotionAngle, 0 },
  { "getanglestolikelyenemypath", ActorCmd_GetAnglesToLikelyEnemyPath, 0 },
  { "setturretanim", ActorCmd_SetTurretAnim, 0 },
  { "getturret", ActorCmd_GetTurret, 0 },
  { "beginprediction", ActorCmd_BeginPrediction, 0 },
  { "endprediction", ActorCmd_EndPrediction, 0 },
  { "lerpposition", ActorCmd_LerpPosition, 0 },
  { "predictoriginandangles", ActorCmd_PredictOriginAndAngles, 0 },
  { "predictanim", ActorCmd_PredictAnim, 0 },
  { "gethitenttype", ActorCmd_GetHitEntType, 0 },
  { "gethityaw", ActorCmd_GetHitYaw, 0 },
  { "getgroundenttype", ActorCmd_GetGroundEntType, 0 },
  { "isdeflected", ActorCmd_IsDeflected, 0 },
  { "trackscriptstate", ActorCmd_trackScriptState, 0 },
  { "dumphistory", ActorCmd_DumpHistory, 0 },
  { "animcustom", ScrCmd_animcustom, 0 },
  { "canattackenemynode", ScrCmd_CanAttackEnemyNode, 0 },
  { "getnegotiationstartnode", ScrCmd_GetNegotiationStartNode, 0 },
  { "getnegotiationendnode", ScrCmd_GetNegotiationEndNode, 0 },
  { "checkprone", ActorCmd_CheckProne, 0 },
  { "pushplayer", ActorCmd_PushPlayer, 0 },
  { "checkgrenadethrowpos", ActorCmd_CheckGrenadeThrowPos, 0 },
  { "setgoalnode", ActorCmd_SetGoalNode, 0 },
  { "setgoalpos", ActorCmd_SetGoalPos, 0 },
  { "setgoalentity", ActorCmd_SetGoalEntity, 0 },
  { "setgoalvolume", ActorCmd_SetGoalVolume, 0 },
  { "getgoalvolume", ActorCmd_GetGoalVolume, 0 },
  { "cleargoalvolume", ActorCmd_ClearGoalVolume, 0 },
  { "setfixednodesafevolume", ActorCmd_SetFixedNodeSafeVolume, 0 },
  { "getfixednodesafevolume", ActorCmd_GetFixedNodeSafeVolume, 0 },
  { "clearfixednodesafevolume", ActorCmd_ClearFixedNodeSafeVolume, 0 },
  { "isingoal", ActorCmd_IsInGoal, 0 },
  { "setruntopos", ActorCmd_SetOverrideRunToPos, 0 },
  { "nearnode", ActorCmd_NearNode, 0 },
  { "clearenemy", ActorCmd_ClearEnemy, 0 },
  { "setentitytarget", ActorCmd_SetEntityTarget, 0 },
  { "clearentitytarget", ActorCmd_ClearEntityTarget, 0 },
  { "setpotentialthreat", ActorCmd_SetPotentialThreat, 0 },
  { "clearpotentialthreat", ActorCmd_ClearPotentialThreat, 0 },
  { "setflashbangimmunity", ActorCmd_SetFlashbangImmunity, 0 },
  { "setflashbanged", ActorCmd_SetFlashBanged, 0 },
  { "getflashbangedstrength", ActorCmd_GetFlashBangedStrength, 0 },
  { "setengagementmindist", ActorCmd_SetEngagementMinDist, 0 },
  { "setengagementmaxdist", ActorCmd_SetEngagementMaxDist, 0 },
  { "isknownenemyinradius", ActorCmd_IsKnownEnemyInRadius, 0 },
  { "isknownenemyinvolume", ActorCmd_IsKnownEnemyInVolume, 0 },
  { "settalktospecies", ActorCmd_SetTalkToSpecies, 0 }
};

const BuiltinMethodDef methods_2[166] =
{
  { "animscripted", ScrCmd_animscripted, 0 },
  { "animrelative", ScrCmd_animrelative, 0 },
  { "stopanimscripted", ScrCmd_stopanimscripted, 0 },
  { "startignoringspotlight", ScrCmd_startIgnoringSpotLight, 0 },
  { "stopignoringspotlight", ScrCmd_stopIgnoringSpotLight, 0 },
  { "attach", ScrCmd_attach, 0 },
  { "detach", ScrCmd_detach, 0 },
  { "detachall", ScrCmd_detachAll, 0 },
  { "getattachsize", ScrCmd_GetAttachSize, 0 },
  { "getattachmodelname", ScrCmd_GetAttachModelName, 0 },
  { "getattachtagname", ScrCmd_GetAttachTagName, 0 },
  { "getattachignorecollision", ScrCmd_GetAttachIgnoreCollision, 0 },
  { "hidepart", ScrCmd_hidepart, 0 },
  { "showpart", ScrCmd_showpart, 0 },
  { "showallparts", ScrCmd_showallparts, 0 },
  { "linkto", ScrCmd_LinkTo, 0 },
  { "setstablemissile", Scr_SetStableMissile, 0 },
  { "playersetgroundreferenceent", ScrCmd_PlayerSetGroundReferenceEnt, 0 },
  { "playerlinkto", ScrCmd_PlayerLinkTo, 0 },
  { "playerlinktodelta", ScrCmd_PlayerLinkToDelta, 0 },
  { "playerlinktoabsolute", ScrCmd_PlayerLinkToAbsolute, 0 },
  { "dontinterpolate", ScrCmd_DontInterpolate, 0 },
  { "unlink", ScrCmd_Unlink, 0 },
  { "enablelinkto", ScrCmd_EnableLinkTo, 0 },
  { "dospawn", ScrCmd_dospawn, 0 },
  { "stalingradspawn", ScrCmd_StalingradSpawn, 0 },
  { "getorigin", ScrCmd_GetOrigin, 0 },
  { "getcentroid", ScrCmd_GetCentroid, 0 },
  { "getshootatpos", ScrCmd_GetShootAtPosition, 0 },
  { "geteye", ScrCmd_GetEye, 0 },
  { "getdebugeye", ScrCmd_GetDebugEye, 1 },
  { "useby", ScrCmd_UseBy, 0 },
  { "istouching", ScrCmd_IsTouching, 0 },
  { "playsound", ScrCmd_PlaySound, 0 },
  { "playsoundasmaster", ScrCmd_PlaySoundAsMaster, 0 },
  { "playloopsound", ScrCmd_PlayLoopSound, 0 },
  { "stoploopsound", ScrCmd_StopLoopSound, 0 },
  { "stopsounds", ScrCmd_StopSounds, 0 },
  { "eqon", ScrCmd_EqOn, 0 },
  { "eqoff", ScrCmd_EqOff, 0 },
  { "haseq", ScrCmd_HasEq, 0 },
  { "playrumbleonentity", ScrCmd_PlayRumbleOnEntity, 0 },
  { "playrumblelooponentity", ScrCmd_PlayRumbleLoopOnEntity, 0 },
  { "stoprumble", ScrCmd_StopRumble, 0 },
  { "iswaitingonsound", ScrCmd_IsWaitingOnSound, 0 },
  { "delete", ScrCmd_Delete, 0 },
  { "setmodel", ScrCmd_SetModel, 0 },
  { "setshadowhint", ScrCmd_SetShadowHint, 0 },
  { "getnormalhealth", ScrCmd_GetNormalHealth, 0 },
  { "setnormalhealth", ScrCmd_SetNormalHealth, 0 },
  { "dodamage", ScrCmd_DoDamage, 0 },
  { "show", ScrCmd_Show, 0 },
  { "hide", ScrCmd_Hide, 0 },
  { "laseron", ScrCmd_LaserOn, 0 },
  { "laseroff", ScrCmd_LaserOff, 0 },
  { "setcontents", ScrCmd_SetContents, 0 },
  { "disconnectpaths", GScr_DisconnectPaths, 0 },
  { "connectpaths", GScr_ConnectPaths, 0 },
  { "startfiring", GScr_StartFiring, 0 },
  { "stopfiring", GScr_StopFiring, 0 },
  { "shootturret", GScr_ShootTurret, 0 },
  { "setmode", GScr_SetMode, 0 },
  { "getturretowner", GScr_GetTurretOwner, 0 },
  { "settargetentity", GScr_SetTargetEntity, 0 },
  { "setplayerspread", GScr_SetPlayerSpread, 0 },
  { "setaispread", GScr_SetAiSpread, 0 },
  { "setconvergencetime", GScr_SetConvergenceTime, 0 },
  { "setsuppressiontime", GScr_SetSuppressionTime, 0 },
  { "cleartargetentity", GScr_ClearTargetEntity, 0 },
  { "setturretteam", GScr_SetTurretTeam, 0 },
  { "maketurretusable", GScr_MakeTurretUsable, 0 },
  { "maketurretunusable", GScr_MakeTurretUnusable, 0 },
  { "setturretaccuracy", GScr_SetTurretAccuracy, 0 },
  { "setturretignoregoals", GScr_SetTurretIgnoreGoals, 0 },
  { "getturrettarget", GScr_GetTurretTarget, 0 },
  { "setcursorhint", GScr_SetCursorHint, 0 },
  { "sethintstring", GScr_SetHintString, 0 },
  { "usetriggerrequirelookat", GScr_UseTriggerRequireLookAt, 0 },
  { "clearanim", GScr_ClearAnim, 0 },
  { "setanimknob", GScr_SetAnimKnob, 0 },
  { "setanimknoblimited", GScr_SetAnimKnobLimited, 0 },
  { "setanimknobrestart", GScr_SetAnimKnobRestart, 0 },
  { "setanimknoblimitedrestart", GScr_SetAnimKnobLimitedRestart, 0 },
  { "setanimknoball", GScr_SetAnimKnobAll, 0 },
  { "setanimknoballlimited", GScr_SetAnimKnobAllLimited, 0 },
  { "setanimknoballrestart", GScr_SetAnimKnobAllRestart, 0 },
  { "setanimknoballlimitedrestart", GScr_SetAnimKnobAllLimitedRestart, 0 },
  { "setanim", GScr_SetAnim, 0 },
  { "setanimlimited", GScr_SetAnimLimited, 0 },
  { "setanimrestart", GScr_SetAnimRestart, 0 },
  { "setanimlimitedrestart", GScr_SetAnimLimitedRestart, 0 },
  { "getanimtime", GScr_GetAnimTime, 0 },
  { "getanimassettype", GScr_GetAnimAssetType, 1 },
  { "setflaggedanimknob", GScr_SetFlaggedAnimKnob, 0 },
  { "setflaggedanimknoblimited", GScr_SetFlaggedAnimKnobLimited, 0 },
  { "setflaggedanimknobrestart", GScr_SetFlaggedAnimKnobRestart, 0 },
  {
    "setflaggedanimknoblimitedrestart",
    GScr_SetFlaggedAnimKnobLimitedRestart,
    0
  },
  { "setflaggedanimknoball", GScr_SetFlaggedAnimKnobAll, 0 },
  { "setflaggedanimknoballrestart", GScr_SetFlaggedAnimKnobAllRestart, 0 },
  { "setflaggedanim", GScr_SetFlaggedAnim, 0 },
  { "setflaggedanimlimited", GScr_SetFlaggedAnimLimited, 0 },
  { "setflaggedanimrestart", GScr_SetFlaggedAnimRestart, 0 },
  { "setflaggedanimlimitedrestart", GScr_SetFlaggedAnimLimitedRestart, 0 },
  { "useanimtree", GScr_UseAnimTree, 0 },
  { "stopuseanimtree", GScr_StopUseAnimTree, 0 },
  { "setanimtime", GScr_SetAnimTime, 0 },
  { "dumpanims", GScr_DumpAnims, 0 },
  { "getstance", ScrCmd_GetStance, 0 },
  { "setstance", ScrCmd_SetStance, 0 },
  { "getammocount", GScr_GetAmmoCount, 0 },
  { "magicgrenade", ScrCmd_MagicGrenade, 0 },
  { "magicgrenademanual", ScrCmd_MagicGrenadeManual, 0 },
  { "isfiringturret", GScr_IsFiringTurret, 0 },
  { "setfriendlychain", GScr_SetFriendlyChain, 0 },
  { "gettagorigin", GScr_GetTagOrigin, 0 },
  { "gettagangles", GScr_GetTagAngles, 0 },
  { "shellshock", GScr_ShellShock, 0 },
  { "stopshellshock", GScr_StopShellShock, 0 },
  { "setdepthoffield", GScr_SetDepthOfField, 0 },
  { "setviewmodeldepthoffield", GScr_SetViewModelDepthOfField, 0 },
  { "viewkick", GScr_ViewKick, 0 },
  { "getentnum", GScr_GetEntnum, 1 },
  { "locklightvis", GScr_LockLightVis, 0 },
  { "unlocklightvis", GScr_UnlockLightVis, 0 },
  { "launch", GScr_Launch, 0 },
  { "setsoundblend", GScr_SetSoundBlend, 0 },
  { "localtoworldcoords", GScr_LocalToWorldCoords, 0 },
  { "getentitynumber", GScr_GetEntityNumber, 0 },
  { "enablegrenadetouchdamage", GScr_EnableGrenadeTouchDamage, 0 },
  { "disablegrenadetouchdamage", GScr_DisableGrenadeTouchDamage, 0 },
  { "enableaimassist", GScr_EnableAimAssist, 0 },
  { "disableaimassist", GScr_DisableAimAssist, 0 },
  { "makefakeai", GScr_MakeFakeAI, 0 },
  { "setlookattext", GScr_SetLookatText, 0 },
  { "setspawnerteam", GScr_SetSpawnerTeam, 0 },
  { "setrightarc", GScr_SetRightArc, 0 },
  { "setleftarc", GScr_SetLeftArc, 0 },
  { "settoparc", GScr_SetTopArc, 0 },
  { "setbottomarc", GScr_SetBottomArc, 0 },
  { "setdefaultdroppitch", GScr_SetDefaultDropPitch, 0 },
  { "restoredefaultdroppitch", GScr_RestoreDefaultDropPitch, 0 },
  { "turretfiredisable", GScr_TurretFireDisable, 0 },
  { "turretfireenable", GScr_TurretFireEnable, 0 },
  { "addaieventlistener", ScrCmd_AddAIEventListener, 0 },
  { "removeaieventlistener", ScrCmd_RemoveAIEventListener, 0 },
  { "radiusdamage", GScr_EntityRadiusDamage, 0 },
  { "detonate", GScr_Detonate, 0 },
  { "damageconetrace", GScr_DamageConeTrace, 0 },
  { "sightconetrace", GScr_SightConeTrace, 0 },
  { "missile_settarget", GScr_MissileSetTarget, 0 },
  { "getlightcolor", GScr_GetLightColor, 0 },
  { "setlightcolor", GScr_SetLightColor, 0 },
  { "getlightintensity", GScr_GetLightIntensity, 0 },
  { "setlightintensity", GScr_SetLightIntensity, 0 },
  { "getlightradius", GScr_GetLightRadius, 0 },
  { "setlightradius", GScr_SetLightRadius, 0 },
  { "getlightfovinner", GScr_GetLightFovInner, 0 },
  { "getlightfovouter", GScr_GetLightFovOuter, 0 },
  { "setlightfovrange", GScr_SetLightFovRange, 0 },
  { "getlightexponent", GScr_GetLightExponent, 0 },
  { "setlightexponent", GScr_SetLightExponent, 0 },
  { "startragdoll", GScr_StartRagdoll, 0 },
  { "isragdoll", GScr_IsRagdoll, 0 },
  { "setmovespeedscale", ScrCmd_SetMoveSpeedScale, 0 },
  { "itemweaponsetammo", ScrCmd_ItemWeaponSetAmmo, 0 },
  { "logstring", ScrCmd_LogString, 0 }
};

BuiltinFunctionDef functions[250] =
{
  { "createprintchannel", GScr_CreatePrintChannel, 1 },
  { "setprintchannel", GScr_printChannelSet, 1 },
  { "print", print, 1 },
  { "println", println, 1 },
  { "iprintln", iprintln, 0 },
  { "iprintlnbold", iprintlnbold, 0 },
  { "print3d", GScr_print3d, 1 },
  { "line", GScr_line, 1 },
  { "getent", Scr_GetEnt, 0 },
  { "getentarray", Scr_GetEntArray, 0 },
  { "getnode", Scr_GetNode, 0 },
  { "getnodearray", Scr_GetNodeArray, 0 },
  { "getallnodes", Scr_GetAllNodes, 0 },
  { "getreflectionlocs", Scr_GetReflectionLocs, 1 },
  { "logstring", Scr_LogString, 0 },
  { "getvehiclenode", GScr_GetVehicleNode, 0 },
  { "getvehiclenodearray", GScr_GetVehicleNodeArray, 0 },
  { "getallvehiclenodes", GScr_GetAllVehicleNodes, 0 },
  { "getnumvehicles", GScr_GetNumVehicles, 0 },
  { "precachevehicle", GScr_PrecacheVehicle, 0 },
  { "precacheturret", (void(*)())GScr_PrecacheTurret, 0 },
  { "spawn", GScr_Spawn, 0 },
  { "spawnvehicle", GScr_SpawnVehicle, 0 },
  { "spawnturret", GScr_SpawnTurret, 0 },
  { "canspawnturret", GScr_CanSpawnTurret, 0 },
  { "spawnstruct", Scr_AddStruct, 0 },
  { "assert", assertCmd, 1 },
  { "assertex", assertexCmd, 1 },
  { "assertmsg", assertmsgCmd, 1 },
  { "isdefined", GScr_IsDefined, 0 },
  { "isstring", GScr_IsString, 0 },
  { "isarray", GScr_IsArray, 0 },
  { "isalive", GScr_IsAlive, 0 },
  { "isplayer", GScr_IsPlayer, 0 },
  { "isai", GScr_IsAI, 0 },
  { "issentient", GScr_IsSentient, 0 },
  { "isgodmode", GScr_IsGodMode, 0 },
  { "getdvar", GScr_GetDvar, 0 },
  { "getdvarint", GScr_GetDvarInt, 0 },
  { "getdvarfloat", GScr_GetDvarFloat, 0 },
  { "getdebugdvar", GScr_GetDebugDvar, 1 },
  { "getdebugdvarint", GScr_GetDebugDvarInt, 1 },
  { "getdebugdvarfloat", GScr_GetDebugDvarFloat, 1 },
  { "setdvar", GScr_SetDvar, 0 },
  { "setsaveddvar", GScr_SetSavedDvar, 0 },
  { "gettime", GScr_GetTime, 0 },
  { "getdifficulty", GScr_GetDifficulty, 0 },
  { "getentbynum", Scr_GetEntByNum, 1 },
  { "getaiarray", (void(*)())Scr_GetAIArray, 0 },
  { "getaispeciesarray", (void(*)())Scr_GetAISpeciesArray, 0 },
  { "getspawnerarray", Scr_GetSpawnerArray, 0 },
  { "getspawnerteamarray", Scr_GetSpawnerTeamArray, 0 },
  { "getweaponmodel", Scr_GetWeaponModel, 0 },
  { "getweaponclipmodel", Scr_GetWeaponClipModel, 0 },
  { "getanimlength", GScr_GetAnimLength, 0 },
  { "animhasnotetrack", GScr_AnimHasNotetrack, 0 },
  { "getnotetracktimes", GScr_GetNotetrackTimes, 0 },
  { "getbrushmodelcenter", GScr_GetBrushModelCenter, 0 },
  { "getkeybinding", GScr_GetKeyBinding, 0 },
  { "getcommandfromkey", GScr_GetCommandFromKey, 0 },
  { "objective_add", (void(*)())Scr_Objective_Add, 0 },
  { "objective_delete", Scr_Objective_Delete, 0 },
  { "objective_state", (void(*)())Scr_Objective_State, 0 },
  { "objective_string", Scr_Objective_String, 0 },
  { "objective_string_nomessage", Scr_Objective_String_NoMessage, 0 },
  { "objective_icon", Scr_Objective_Icon, 0 },
  { "objective_position", Scr_Objective_Position, 0 },
  { "objective_additionalposition", Scr_Objective_AdditionalPosition, 0 },
  { "objective_current", (void(*)())Scr_Objective_Current, 0 },
  { "objective_additionalcurrent", (void(*)())Scr_Objective_AdditionalCurrent, 0 },
  { "objective_ring", Scr_Objective_Ring, 0 },
  { "target_set", Scr_Target_Set, 0 },
  { "target_remove", Scr_Target_Remove, 0 },
  { "target_setshader", Scr_Target_SetShader, 0 },
  { "target_setoffscreenshader", Scr_Target_SetOffscreenShader, 0 },
  { "target_isinrect", Scr_Target_IsInRect, 0 },
  { "target_isincircle", Scr_Target_IsInCircle, 0 },
  { "target_startreticlelockon", Scr_Target_StartLockOn, 0 },
  { "target_clearreticlelockon", Scr_Target_ClearLockOn, 0 },
  { "target_getarray", Scr_Target_GetArray, 0 },
  { "target_istarget", Scr_Target_IsTarget, 0 },
  { "target_setattackmode", Scr_Target_SetAttackMode, 0 },
  { "target_setjavelinonly", Scr_Target_SetJavelinOnly, 0 },
  { "missile_createattractorent", Scr_MissileCreateAttractorEnt, 0 },
  { "missile_createattractororigin", Scr_MissileCreateAttractorOrigin, 0 },
  { "missile_createrepulsorent", Scr_MissileCreateRepulsorEnt, 0 },
  { "missile_createrepulsororigin", Scr_MissileCreateRepulsorOrigin, 0 },
  { "missile_deleteattractor", Scr_MissileDeleteAttractor, 0 },
  { "bullettrace", Scr_BulletTrace, 0 },
  { "bullettracepassed", Scr_BulletTracePassed, 0 },
  { "sighttracepassed", Scr_SightTracePassed, 0 },
  { "physicstrace", Scr_PhysicsTrace, 0 },
  { "playerphysicstrace", Scr_PlayerPhysicsTrace, 0 },
  { "getstartorigin", GScr_GetStartOrigin, 0 },
  { "getstartangles", GScr_GetStartAngles, 0 },
  { "getcycleoriginoffset", GScr_GetCycleOriginOffset, 0 },
  { "getmovedelta", GScr_GetMoveDelta, 0 },
  { "getangledelta", GScr_GetAngleDelta, 0 },
  { "getnorthyaw", GScr_GetNorthYaw, 0 },
  { "randomint", Scr_RandomInt, 0 },
  { "randomfloat", Scr_RandomFloat, 0 },
  { "randomintrange", Scr_RandomIntRange, 0 },
  { "randomfloatrange", Scr_RandomFloatRange, 0 },
  { "sin", GScr_sin, 0 },
  { "cos", GScr_cos, 0 },
  { "tan", GScr_tan, 0 },
  { "asin", GScr_asin, 0 },
  { "acos", GScr_acos, 0 },
  { "atan", GScr_atan, 0 },
  { "int", GScr_CastInt, 0 },
  { "abs", GScr_abs, 0 },
  { "min", GScr_min, 0 },
  { "max", GScr_max, 0 },
  { "floor", GScr_floor, 0 },
  { "ceil", GScr_ceil, 0 },
  { "sqrt", GScr_sqrt, 0 },
  { "vectorfromlinetopoint", GScr_VectorFromLineToPoint, 0 },
  { "pointonsegmentnearesttopoint", GScr_PointOnSegmentNearestToPoint, 0 },
  { "distance", Scr_Distance, 0 },
  { "distance2d", Scr_Distance2D, 0 },
  { "distancesquared", Scr_DistanceSquared, 0 },
  { "length", Scr_Length, 0 },
  { "lengthsquared", Scr_LengthSquared, 0 },
  { "closer", Scr_Closer, 0 },
  { "vectordot", Scr_VectorDot, 0 },
  { "vectornormalize", Scr_VectorNormalize, 0 },
  { "vectortoangles", Scr_VectorToAngles, 0 },
  { "vectorlerp", Scr_VectorLerp, 0 },
  { "anglestoup", Scr_AnglesToUp, 0 },
  { "anglestoright", Scr_AnglesToRight, 0 },
  { "anglestoforward", Scr_AnglesToForward, 0 },
  { "combineangles", Scr_CombineAngles, 0 },
  { "issubstr", Scr_IsSubStr, 0 },
  { "getsubstr", Scr_GetSubStr, 0 },
  { "tolower", Scr_ToLower, 0 },
  { "strtok", Scr_StrTok, 0 },
  { "setblur", Scr_SetBlur, 0 },
  { "musicplay", Scr_MusicPlay, 0 },
  { "musicstop", Scr_MusicStop, 0 },
  { "soundfade", Scr_SoundFade, 0 },
  { "amplify", Scr_Amplify, 0 },
  { "amplifystop", Scr_AmplifyStop, 0 },
  { "ambientplay", Scr_AmbientPlay, 0 },
  { "ambientstop", Scr_AmbientStop, 0 },
  { "precachemodel", (void(*)())Scr_PrecacheModel, 0 },
  { "precacheshellshock", Scr_PrecacheShellShock, 0 },
  { "precacheitem", Scr_PrecacheItem, 0 },
  { "precacheshader", (void(*)())Scr_PrecacheMaterial, 0 },
  { "precachestring", Scr_PrecacheString, 0 },
  { "precachemenu", GScr_PrecacheMenu, 0 },
  { "precacherumble", (void(*)())Scr_PrecacheRumble, 0 },
  { "precachelocationselector", GScr_PrecacheLocationSelector, 0 },
  { "precachenightvisioncodeassets", Scr_PrecacheNightvisionCodeAssets, 0 },
  { "savegame", Scr_SaveGame, 0 },
  { "issavesuccessful", Scr_IsSaveSuccessful, 0 },
  { "issaverecentlyloaded", Scr_IsRecentlyLoaded, 0 },
  { "savegamenocommit", Scr_SaveGameNoCommit, 0 },
  { "commitsave", Scr_CommitSave, 0 },
  { "loadfx", Scr_LoadFX, 0 },
  { "playfx", Scr_PlayFX, 0 },
  { "playfxontag", Scr_PlayFXOnTag, 0 },
  { "playloopedfx", Scr_PlayLoopedFX, 0 },
  { "spawnfx", Scr_SpawnFX, 0 },
  { "triggerfx", Scr_TriggerFX, 0 },
  { "getfxvisibility", Scr_GetFXVis, 0 },
  { "physicsexplosionsphere", Scr_PhysicsExplosionSphere, 0 },
  { "physicsexplosioncylinder", Scr_PhysicsExplosionCylinder, 0 },
  { "physicsjolt", Scr_PhysicsRadiusJolt, 0 },
  { "physicsjitter", Scr_PhysicsRadiusJitter, 0 },
  { "setexpfog", Scr_SetExponentialFog, 0 },
  { "setculldist", Scr_SetCullDist, 0 },
  { "visionsetnaked", Scr_VisionSetNaked, 0 },
  { "visionsetnight", Scr_VisionSetNight, 0 },
  { "getmapsunlight", Scr_GetMapSunLight, 0 },
  { "setsunlight", Scr_SetSunLight, 0 },
  { "resetsunlight", Scr_ResetSunLight, 0 },
  { "getmapsundirection", Scr_GetMapSunDirection, 0 },
  { "setsundirection", Scr_SetSunDirection, 0 },
  { "lerpsundirection", Scr_LerpSunDirection, 0 },
  { "resetsundirection", Scr_ResetSunDirection, 0 },
  { "radiusdamage", GScr_RadiusDamage, 0 },
  { "setplayerignoreradiusdamage", (void(*)())GScr_SetPlayerIgnoreRadiusDamage, 0 },
  { "changelevel", GScr_ChangeLevel, 0 },
  { "missionsuccess", GScr_MissionSuccess, 0 },
  { "missionfailed", GScr_MissionFailed, 0 },
  { "setmissiondvar", GScr_SetMissionDvar, 0 },
  { "cinematic", GScr_Cinematic, 0 },
  { "cinematicingame", GScr_CinematicInGame, 0 },
  { "cinematicingamesync", GScr_CinematicInGameSync, 0 },
  { "cinematicingameloop", GScr_CinematicInGameLoop, 0 },
  { "cinematicingameloopresident", GScr_CinematicInGameLoopResident, 0 },
  {
    "cinematicingameloopfromfastfile",
    GScr_CinematicInGameLoopFromFastfile,
    0
  },
  { "stopcinematicingame", GScr_StopCinematicInGame, 0 },
  { "earthquake", GScr_Earthquake, 0 },
  { "drawcompassfriendlies", (void(*)())GScr_DrawCompassFriendlies, 0 },
  { "bulletspread", Scr_BulletSpread, 0 },
  { "bullettracer", Scr_BulletTracer, 0 },
  { "magicbullet", Scr_MagicBullet, 0 },
  { "getnumparts", GScr_GetNumParts, 0 },
  { "getpartname", GScr_GetPartName, 0 },
  { "weaponfiretime", GScr_WeaponFireTime, 0 },
  { "weaponclipsize", GScr_WeaponClipSize, 0 },
  { "weaponissemiauto", GScr_WeaponIsSemiAuto, 0 },
  { "weaponisboltaction", GScr_WeaponIsBoltAction, 0 },
  { "weapontype", GScr_WeaponType, 0 },
  { "weaponclass", GScr_WeaponClass, 0 },
  { "weaponinventorytype", GScr_WeaponInventoryType, 0 },
  { "weaponstartammo", GScr_WeaponStartAmmo, 0 },
  { "weaponmaxammo", GScr_WeaponMaxAmmo, 0 },
  { "weaponaltweaponname", GScr_WeaponAltWeaponName, 0 },
  { "weaponfightdist", GScr_WeaponFightDist, 0 },
  { "weaponmaxdist", GScr_WeaponMaxDist, 0 },
  { "isturretactive", GScr_IsTurretActive, 0 },
  { "isweaponcliponly", GScr_IsWeaponClipOnly, 0 },
  { "isweapondetonationtimed", GScr_IsWeaponDetonationTimed, 0 },
  { "badplace_delete", Scr_BadPlace_Delete, 0 },
  { "badplace_cylinder", Scr_BadPlace_Cylinder, 0 },
  { "badplace_arc", Scr_BadPlace_Arc, 0 },
  { "badplace_brush", Scr_BadPlace_Brush, 0 },
  { "clearallcorpses", Scr_ClearAllCorpses, 0 },
  { "newhudelem", GScr_NewHudElem, 0 },
  { "resettimeout", Scr_ResetTimeout, 0 },
  { "setturretnode", Scr_SetTurretNode, 0 },
  { "unsetturretnode", Scr_UnsetTurretNode, 0 },
  { "setnodepriority", Scr_SetNodePriority, 0 },
  { "isnodeoccupied", Scr_IsNodeOccupied, 0 },
  { "setdebugorigin", GScr_SetDebugOrigin, 1 },
  { "setdebugangles", GScr_SetDebugAngles, 1 },
  { "playrumbleonposition", (void(*)())Scr_PlayRumbleOnPosition, 0 },
  { "playrumblelooponposition", (void(*)())Scr_PlayRumbleLoopOnPosition, 0 },
  { "stopallrumbles", (void(*)())Scr_StopAllRumbles, 0 },
  { "soundexists", ScrCmd_SoundExists, 0 },
  { "giveachievement", ScrCmd_GiveAchievement, 0 },
  { "updategamerprofile", ScrCmd_UpdateGamerProfile, 0 },
  { "openfile", GScr_OpenFile, 1 },
  { "closefile", GScr_CloseFile, 1 },
  { "fprintln", GScr_FPrintln, 1 },
  { "fprintfields", GScr_FPrintFields, 1 },
  { "freadln", GScr_FReadLn, 1 },
  { "fgetarg", GScr_FGetArg, 1 },
  { "setminimap", GScr_SetMiniMap, 0 },
  { "getarraykeys", GScr_GetArrayKeys, 0 },
  { "clearlocalizedstrings", GScr_ClearLocalizedStrings, 0 },
  { "setphysicsgravitydir", ScrCmd_SetPhysicsGravityDir, 0 },
  { "gettimescale", ScrCmd_GetTimeScale, 0 },
  { "settimescale", ScrCmd_SetTimeScale, 0 },
  { "tablelookup", Scr_TableLookup, 0 },
  { "tablelookupistring", Scr_TableLookupIString, 0 },
  { "refreshhudcompass", Scr_RefreshHudCompass, 0 },
  { "refreshhudammocounter", Scr_RefreshHudAmmoCounter, 0 }
};




bool g_archiveGetDvar;
const char *difficultyStrings[4] = { "easy", "medium", "hard", "fu" };

scr_data_t g_scr_data;

unsigned int __cdecl GScr_AllocString(const char *s)
{
    return Scr_AllocString((char*)s, 1);
}

void __cdecl TRACK_g_scr_main()
{
    track_static_alloc_internal(&g_scr_data, 3380, "g_scr_data", 7);
}

void __cdecl Scr_LoadLevel()
{
    unsigned __int16 v0; // r3

    if (g_scr_data.levelscript)
    {
        v0 = Scr_ExecThread(g_scr_data.levelscript, 0);
        Scr_FreeThread(v0);
    }
}

int __cdecl GScr_SetScriptAndLabel(
    ScriptFunctions *functions,
    const char *filename,
    const char *label,
    int bEnforceExists)
{
    int count; // r11
    int func; // r30

    if (G_ExitAfterConnectPaths())
        return 0;

    if (functions->count >= functions->maxSize)
        Com_Error(ERR_DROP, "CODE ERROR: GScr_SetScriptAndLabel: functions->maxSize exceeded");

    count = functions->count;
    func = functions->address[count];
    functions->count = count + 1;
    if (!func)
    {
        if (bEnforceExists)
            Com_Error(ERR_DROP, "Could not find label '%s' in script %s", label, filename);
    }
    return func;
}

void __cdecl GScr_SetLevelScript(ScriptFunctions *functions)
{
    const char *String; // r31
    char v3[72]; // [sp+50h] [-60h] BYREF

    String = Dvar_GetString("mapname");
    if (I_strnicmp(String, "mp/", 3) && I_strnicmp(String, "mp\\", 3))
    {
        Com_sprintf(v3, 64, "maps/%s", String);
        g_scr_data.levelscript = GScr_SetScriptAndLabel(functions, v3, "main", 0);
    }
    else
    {
        g_scr_data.levelscript = 0;
    }
}

void *__cdecl GScr_AnimscriptAlloc(int size)
{
    return Hunk_AllocLow(size, "GScr_AnimscriptAlloc", 5);
}

void __cdecl GScr_SetScriptsForPathNode(pathnode_t *loadNode, void *data)
{
    const char *animscript; // r30
    char filename[112]; // [sp+50h] [-70h] BYREF

    ScriptFunctions *functions = (ScriptFunctions *)data;

    iassert(data);

    if (loadNode->constant.type)
    {
        if (loadNode->constant.type == NODE_NEGOTIATION_BEGIN)
        {
            if (loadNode->constant.animscript)
            {
                animscript = SL_ConvertToString(loadNode->constant.animscript);
                iassert(animscript);
                loadNode->constant.animscriptfunc = (int)Hunk_FindDataForFile(1, animscript);
                if (!loadNode->constant.animscriptfunc)
                {
                    Com_sprintf(filename, 64, "animscripts/traverse/%s", animscript);
                    loadNode->constant.animscriptfunc = (int)GScr_SetScriptAndLabel(functions, filename, "main", 1);
                    Hunk_SetDataForFile(1, animscript, (void*)loadNode->constant.animscriptfunc, GScr_AnimscriptAlloc);
                }
                if (!loadNode->constant.animscriptfunc)
                {
                    Com_PrintError(
                        1,
                        "ERROR: Pathnode (%s) at (%g %g %g) cannot find animscript '%s'\n",
                        nodeStringTable[loadNode->constant.type],
                        loadNode->constant.vOrigin[0],
                        loadNode->constant.vOrigin[1],
                        loadNode->constant.vOrigin[2],
                        animscript);

                    loadNode->constant.type = NODE_BADNODE;
                }
            }
            else
            {
                Com_PrintError(
                    1,
                    "ERROR: Pathnode (%s) at (%g %g %g) has no animscript specified\n",
                    nodeStringTable[NODE_NEGOTIATION_BEGIN],
                    loadNode->constant.vOrigin[0],
                    loadNode->constant.vOrigin[1],
                    loadNode->constant.vOrigin[2]);

                loadNode->constant.type = NODE_BADNODE;
            }
        }
        else
        {
            iassert(!loadNode->constant.animscript);
        }
    }
}

void __cdecl GScr_SetScriptsForPathNodes(ScriptFunctions *functions)
{
    Path_CallFunctionForNodes(GScr_SetScriptsForPathNode, functions);
}

scr_animtree_t GScr_FindAnimTree(const char *filename, int bEnforceExists)
{
    scr_animtree_t tree;

    tree = Scr_FindAnimTree(filename);

    if (!tree.anims && bEnforceExists)
        Com_Error(ERR_DROP, "Could not find animation tree %s", filename);

    return tree;
}

void __cdecl GScr_FindAnimTrees()
{
    scr_animtree_t tree; // r31

    tree = Scr_FindAnimTree("generic_human");
    if (!tree.anims)
        Com_Error(ERR_DROP, "Could not find animation tree '%s'", "generic_human");
    g_scr_data.generic_human.tree = tree;
}

void __cdecl GScr_SetSingleAnimScript(ScriptFunctions *functions, scr_animscript_t *pAnim, const char *name)
{
    char v6[112]; // [sp+50h] [-70h] BYREF

    if (!pAnim)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 283, 0, "%s", "pAnim");
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 284, 0, "%s", "name");
    Com_sprintf(v6, 64, "animscripts/%s", name);
    pAnim->func = GScr_SetScriptAndLabel(functions, v6, "main", 1);
    pAnim->name = Scr_AllocString((char*)name, 1);
}

void __cdecl GScr_SetAnimScripts(ScriptFunctions *functions)
{
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.combat, "combat");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.concealment_crouch, "concealment_crouch");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.concealment_prone, "concealment_prone");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.concealment_stand, "concealment_stand");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_arrival, "cover_arrival");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_crouch, "cover_crouch");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_left, "cover_left");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_prone, "cover_prone");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_right, "cover_right");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_stand, "cover_stand");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_wide_left, "cover_wide_left");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.cover_wide_right, "cover_wide_right");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.death, "death");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.grenade_return_throw, "grenade_return_throw");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.init, "init");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.pain, "pain");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.move, "move");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.scripted, "scripted");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.stop, "stop");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.grenade_cower, "grenade_cower");
    GScr_SetSingleAnimScript(functions, &g_scr_data.anim.flashed, "flashed");
    g_scr_data.scripted_init = GScr_SetScriptAndLabel(functions, "animscripts/scripted", "init", 1);
    g_animScriptTable[0] = &g_scr_data.anim;
    g_animScriptTable[1] = &g_scr_data.dogAnim;
}

void __cdecl GScr_SetDogAnimScripts(ScriptFunctions *functions)
{
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.combat, "dog_combat");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.death, "dog_death");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.init, "dog_init");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.pain, "dog_pain");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.move, "dog_move");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.scripted, "dog_scripted");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.stop, "dog_stop");
    GScr_SetSingleAnimScript(functions, &g_scr_data.dogAnim.flashed, "dog_flashed");
}

void GScr_PostLoadScripts()
{
    signed int i; // r31

    for (i = 0; i < 4; ++i)
        Scr_SetClassMap(i);
    GScr_AddFieldsForEntity();
    GScr_AddFieldsForHudElems();
    GScr_AddFieldsForPathnode();
    GScr_AddFieldsForVehicleNode();
    GScr_AddFieldsForRadiant();
}

void *__cdecl Hunk_AllocXAnimCreate(int size)
{
    return Hunk_AllocLow(size, "XAnimCreateAnims", 11);
}

void __cdecl GScr_FreeScripts()
{
    signed int i; // r31

    for (i = 0; i < 4; ++i)
        Scr_RemoveClassMap(i);
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
        iassert(entref.entnum < MAX_GENTITIES);
        return &g_entities[entref.entnum];
    }
}

gentity_s *__cdecl GetPlayerEntity(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *v2; // r30
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 414, 0, "%s", "ent");
    if (!Entity->client)
    {
        if (Entity->targetname)
            v2 = SL_ConvertToString(Entity->targetname);
        else
            v2 = "<undefined>";
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va(
            "only valid on players; called on entity %i at %.0f %.0f %.0f classname %s targetname %s\n",
            0, // KISAKTODO ent %i
            Entity->r.currentOrigin[0],
            Entity->r.currentOrigin[1],
            Entity->r.currentOrigin[2],
            v3,
            v2);
        Scr_Error(v4);
    }
    return Entity;
}

void GScr_CreatePrintChannel()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("illegal call to createprintchannel()");
    if (!Con_OpenChannel((char*)Scr_GetString(0), 1))
        Scr_Error("Unable to create new channel.  Maximum number of channels exeeded.");
}

void GScr_printChannelSet()
{
    int scriptPrintChannel; // r30
    int Type; // r3
    const char *String; // r3
    int Int; // [sp+50h] [-20h] BYREF

    Int = 24;
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("illegal call to setprintchannel()");
        return;
    }
    scriptPrintChannel = level.scriptPrintChannel;
    Type = Scr_GetType(0);
    if (Type == 2)
    {
        String = Scr_GetString(0);
        if (!Con_GetChannel(String, &Int))
        {
            Scr_ParamError(0, "Invalid Print Channel");
            return;
        }
    }
    else if (Type != 6 || (Int = Scr_GetInt(0), !Con_IsChannelOpen(Int)))
    {
        Scr_ParamError(0, "Invalid Print Channel");
        return;
    }
    if (Con_ScriptHasPermission(Int))
    {
        level.scriptPrintChannel = Int;
        Scr_AddInt(scriptPrintChannel);
    }
    else
    {
        Scr_ParamError(0, "Script does not have permission to print to this channel");
    }
}

void print()
{
    signed int NumParam; // r28
    signed int i; // r31
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
    const dvar_s *v0; // r11
    int v1; // r3

    v0 = logScriptTimes;
    if (!logScriptTimes)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 545, 0, "%s", "logScriptTimes");
        v0 = logScriptTimes;
    }
    if (v0->current.enabled)
    {
        v1 = Sys_Milliseconds();
        Com_Printf(level.scriptPrintChannel, "PRINT TIME: %d\n", v1);
    }
    if (!g_NoScriptSpam->current.enabled)
    {
        print();
        Com_Printf(level.scriptPrintChannel, "\n");
    }
}

// attributes: thunk
void __cdecl Scr_LocalizationError(unsigned int iParm, const char *pszErrorMessage)
{
    Scr_ParamError(iParm, pszErrorMessage);
}

void __cdecl Scr_ValidateLocalizedStringRef(unsigned int parmIndex, const char *token, int tokenLen)
{
    int v6; // r31
    const char *v7; // r3

    if (!token)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 571, 0, "%s", "token");
    if (tokenLen < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 572, 0, "%s", "tokenLen >= 0");
    if (tokenLen > 1)
    {
        v6 = 0;
        do
        {
            if (!isalnum(token[v6]) && token[v6] != 95)
            {
                v7 = va(
                    "Illegal localized string reference: %s must contain only alpha-numeric characters and underscores",
                    token);
                Scr_ParamError(parmIndex, v7);
            }
            ++v6;
        } while (v6 < tokenLen);
    }
}

int __cdecl Scr_ValidateNonLocalizedStringRef(
    unsigned int parmIndex,
    const char *token,
    int tokenLen,
    const char *errorContext)
{
    int v8; // r31
    char v9; // r11
    const char *v10; // r3
    const char *v12; // r3

    if (!token)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 590, 0, "%s", "token");
    v8 = 0;
    if (tokenLen <= 0)
        return 1;
    while (1)
    {
        v9 = token[v8];
        if (v9 == 20 || v9 == 21 || v9 == 22)
        {
            v10 = va("bad escape character (%i) present in string", v9);
            Scr_ParamError(parmIndex, v10);
        }
        if (isalpha(token[v8]))
            break;
        if (++v8 >= tokenLen)
            return 1;
    }
    if (loc_warnings->current.enabled)
    {
        if (loc_warningsAsErrors->current.enabled)
        {
            v12 = va("non-localized %s strings are not allowed to have letters in them: \"%s\"", errorContext, token);
            Scr_ParamError(parmIndex, v12);
            return 0;
        }
        Com_PrintWarning(
            23,
            "WARNING: Non-localized %s string is not allowed to have letters in it. Must be changed over to a localized string: \"%s\"\n",
            errorContext,
            token);
    }
    return 0;
}

unsigned int __cdecl Scr_NonLocalizedStringErrorPrefix(
    unsigned int parmIndex,
    unsigned int tokenLen,
    const char *errorContext,
    unsigned int stringLen,
    unsigned int stringLimit,
    char *string)
{
    const char *v11; // r3

    if (!loc_warnings->current.enabled)
        return stringLen;
    if (tokenLen + stringLen + 14 >= stringLimit)
    {
        v11 = va("%s is too long. Max length is %i\n", errorContext, stringLimit);
        Scr_ParamError(parmIndex, v11);
    }
    if (stringLen + 13 >= stringLimit)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            624,
            0,
            "%s",
            "stringLen + LOCALIZATION_ERROR_PREFIX_LENGTH < stringLimit");
    memcpy(&string[stringLen], "UNLOCALIZED: ", 0xDu);
    return stringLen + 13;
}

int __cdecl Scr_IsValidMessageChar(char key)
{
    bool IsValidGamePadChar; // r3
    unsigned __int8 v2; // r11

    if (key >= 31)
        return 1;
    IsValidGamePadChar = Key_IsValidGamePadChar(key);
    v2 = 0;
    if (IsValidGamePadChar)
        return 1;
    return v2;
}

void __cdecl Scr_ConstructMessageString(
    int firstParmIndex,
    int lastParmIndex,
    const char *errorContext,
    char *string,
    unsigned int stringLimit)
{
    unsigned int v9; // r31
    signed int v10; // r28
    const char *IString; // r3
    const char *v12; // r27
    const char *v13; // r11
    unsigned int v15; // r30
    const char *v16; // r3
    const char *v17; // r3
    const char *v18; // r11
    bool v20; // r29
    const char *v21; // r3
    unsigned int i; // r29
    char v23; // r3
    bool IsValidGamePadChar; // r3
    char v25; // r11

    v9 = 0;
    v10 = firstParmIndex;
    for (*string = 0; v10 <= lastParmIndex; ++v10)
    {
        if (Scr_GetType(v10) == 3)
        {
            IString = Scr_GetIString(v10);
            v12 = IString;
            v13 = IString;
            while (*(unsigned __int8 *)v13++)
                ;
            v15 = v13 - IString - 1;
            Scr_ValidateLocalizedStringRef(v10, IString, v15);
            if (v15 + v9 + 1 >= stringLimit)
            {
                v16 = va("%s is too long. Max length is %i\n", errorContext, stringLimit);
                Scr_ParamError(v10, v16);
            }
            if (v9)
                string[v9++] = 20;
        }
        else
        {
            v17 = Scr_GetString(v10);
            v12 = v17;
            v18 = v17;
            while (*(unsigned __int8 *)v18++)
                ;
            v15 = v18 - v17 - 1;
            v20 = (unsigned __int8)Scr_ValidateNonLocalizedStringRef(v10, v17, v15, errorContext) == 0;
            if (v15 + v9 + 1 >= stringLimit)
            {
                v21 = va("%s is too long. Max length is %i\n", errorContext, stringLimit);
                Scr_ParamError(v10, v21);
            }
            if (v15)
                string[v9++] = 21;
            if (v20)
                v9 = Scr_NonLocalizedStringErrorPrefix(v10, v15, errorContext, v9, stringLimit, string);
        }
        for (i = 0; i < v15; ++v9)
        {
            v23 = v12[i];
            if (v23 >= 31 || (IsValidGamePadChar = Key_IsValidGamePadChar(v23), v25 = 0, IsValidGamePadChar))
                v25 = 1;
            if (v25)
                string[v9] = v12[i];
            else
                string[v9] = 46;
            ++i;
        }
    }
    string[v9] = 0;
}

void __cdecl Scr_MakeGameMessage(const char *cmd)
{
    unsigned int NumParam; // r3
    const char *v3; // r3
    char v4[1056]; // [sp+50h] [-420h] BYREF

    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Game Message", v4, 0x400u);
    v3 = va("%s \"%s\"", cmd, v4);
    SV_GameSendServerCommand(-1, v3);
}

void __cdecl Scr_VerifyWeaponIndex(int weaponIndex, const char *weaponName)
{
    const char *v4; // r3

    if (!weaponName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 723, 0, "%s", "weaponName");
    if (!weaponIndex)
    {
        if (I_stricmp("none", weaponName))
            v4 = va(
                "Unknown weapon name \"%s\": script may need to call PreCacheItem(\"%s\") during level init.\n",
                weaponName,
                weaponName);
        else
            v4 = va("Weapon name \"%s\" is not valid.\n", weaponName);
        Scr_ParamError(0, v4);
    }
}

void iprintln()
{
    unsigned int NumParam; // r3
    const char *v1; // r3
    char v2[1032]; // [sp+50h] [-420h] BYREF

    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Game Message", v2, 0x400u);
    v1 = va("%s \"%s\"", "gm", v2);
    SV_GameSendServerCommand(-1, v1);
}

void iprintlnbold()
{
    unsigned int NumParam; // r3
    const char *v1; // r3
    char v2[1032]; // [sp+50h] [-420h] BYREF

    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Game Message", v2, 0x400u);
    v1 = va("%s \"%s\"", "gmb", v2);
    SV_GameSendServerCommand(-1, v1);
}

void GScr_print3d()
{
    double Float; // fp31
    const char *String; // r30
    const char *v2; // r5
    float v3[4]; // [sp+50h] [-50h] BYREF
    float v4[4]; // [sp+60h] [-40h] BYREF
    float v5; // [sp+70h] [-30h] BYREF
    float v6; // [sp+74h] [-2Ch]
    float v7; // [sp+78h] [-28h]
    float v8; // [sp+7Ch] [-24h]

    Float = 1.0;
    v5 = 1.0;
    v6 = 1.0;
    v7 = 1.0;
    v8 = 1.0;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_6;
    case 3u:
        goto LABEL_5;
    case 4u:
        goto LABEL_4;
    case 5u:
        goto LABEL_3;
    case 6u:
        Scr_GetInt(5);
    LABEL_3:
        Float = Scr_GetFloat(4);
    LABEL_4:
        v8 = Scr_GetFloat(3);
    LABEL_5:
        Scr_GetVector(2u, v3);
        v5 = v3[0];
        v6 = v3[1];
        v7 = v3[2];
    LABEL_6:
        String = Scr_GetString(1);
        Scr_GetVector(0, v4);
        G_AddDebugStringWithDuration(v4, &v5, Float, v2, (int)String);
        break;
    default:
        Scr_Error("illegal call to print3d()");
        break;
    }
}

void GScr_line()
{
    int Int; // r30
    int v1; // r31
    float v2[4]; // [sp+50h] [-60h] BYREF
    float v3[4]; // [sp+60h] [-50h] BYREF
    float v4[4]; // [sp+70h] [-40h] BYREF
    float v5; // [sp+80h] [-30h] BYREF
    float v6; // [sp+84h] [-2Ch]
    float v7; // [sp+88h] [-28h]
    float Float; // [sp+8Ch] [-24h]

    Int = 0;
    v1 = 0;
    v5 = 1.0;
    v6 = 1.0;
    v7 = 1.0;
    Float = 1.0;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_6;
    case 3u:
        goto LABEL_5;
    case 4u:
        goto LABEL_4;
    case 5u:
        goto LABEL_3;
    case 6u:
        Int = Scr_GetInt(5);
    LABEL_3:
        v1 = Scr_GetInt(4);
    LABEL_4:
        Float = Scr_GetFloat(3);
    LABEL_5:
        Scr_GetVector(2u, v2);
        v5 = v2[0];
        v6 = v2[1];
        v7 = v2[2];
    LABEL_6:
        Scr_GetVector(1u, v3);
        Scr_GetVector(0, v4);
        break;
    default:
        Scr_Error("illegal call to line()");
        break;
    }
    CL_AddDebugLine(v4, v3, &v5, v1, Int, 1);
}

void assertCmd()
{
    if (!Scr_GetInt(0))
        Scr_Error("assert fail");
}

void assertexCmd()
{
    const char *String; // r3
    const char *v1; // r3

    if (!Scr_GetInt(0))
    {
        String = Scr_GetString(1);
        v1 = va("assert fail: %s", String);
        Scr_Error(v1);
    }
}

void assertmsgCmd()
{
    const char *String; // r3
    const char *v1; // r3

    String = Scr_GetString(0);
    v1 = va("assert fail: %s", String);
    Scr_Error(v1);
}

void GScr_IsDefined()
{
    int Type; // r3
    int v1; // r31
    int PointerType; // r31
    bool v3; // r3

    Type = Scr_GetType(0);
    v1 = Type;
    if (Type == 1)
    {
        PointerType = Scr_GetPointerType(0);
        if (PointerType < 14)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 952, 0, "%s", "type >= FIRST_OBJECT");
        if (PointerType >= 22 || (v3 = 1, PointerType == 19))
            v3 = 0;
    }
    else
    {
        if (Type >= 14)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 957, 0, "%s", "type < FIRST_OBJECT");
        v3 = v1 != 0;
    }
    Scr_AddInt(v3);
}

void GScr_IsString()
{
    int Type; // r3

    Type = Scr_GetType(0);
    Scr_AddInt(Type == 2);
}

void GScr_IsArray()
{
    int Type; // r3
    int PointerType; // r31
    bool v2; // r3

    Type = Scr_GetType(0);
    if (Type == 1)
    {
        PointerType = Scr_GetPointerType(0);
        if (PointerType < 14)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 997, 0, "%s", "type >= FIRST_OBJECT");
        v2 = PointerType == 21;
    }
    else
    {
        if (Type >= 14)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 1002, 0, "%s", "type < FIRST_OBJECT");
        v2 = 0;
    }
    Scr_AddInt(v2);
}

void GScr_IsAlive()
{
    gentity_s *Entity; // r3
    actor_s *actor; // r11
    int health; // r11
    int v3; // r3

    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        goto LABEL_7;
    Entity = Scr_GetEntity(0);
    actor = Entity->actor;
    if (actor)
    {
        if (actor->Physics.bIsAlive)
        {
            Scr_AddInt(1);
            return;
        }
        goto LABEL_7;
    }
    health = Entity->health;
    v3 = 1;
    if (health <= 0)
        LABEL_7:
    v3 = 0;
    Scr_AddInt(v3);
}

void GScr_IsPlayer()
{
    gclient_s *client; // r11
    int v1; // r3

    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20 || (client = Scr_GetEntity(0)->client, v1 = 1, !client))
        v1 = 0;
    Scr_AddInt(v1);
}

void GScr_IsAI()
{
    actor_s *actor; // r11
    int v1; // r3

    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20 || (actor = Scr_GetEntity(0)->actor, v1 = 1, !actor))
        v1 = 0;
    Scr_AddInt(v1);
}

void GScr_IsSentient()
{
    sentient_s *sentient; // r11
    int v1; // r3

    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20 || (sentient = Scr_GetEntity(0)->sentient, v1 = 1, !sentient))
        v1 = 0;
    Scr_AddInt(v1);
}

void GScr_IsGodMode()
{
    int flags; // r11
    int v1; // r3

    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20 || (flags = Scr_GetEntity(0)->flags, v1 = 1, (flags & 3) == 0))
        v1 = 0;
    Scr_AddInt(v1);
}

void GScr_GetDvar()
{
    const char *String; // r3
    const char *VariantString; // r3

    String = Scr_GetString(0);
    if (g_archiveGetDvar)
        VariantString = SV_Archived_Dvar_GetVariantString(String);
    else
        VariantString = Dvar_GetVariantString(String);
    Scr_AddString(VariantString);
}

void GScr_GetDvarInt()
{
    const char *String; // r3
    const char *VariantString; // r3
    int v2; // r3

    String = Scr_GetString(0);
    VariantString = SV_Archived_Dvar_GetVariantString(String);
    v2 = atol(VariantString);
    Scr_AddInt(v2);
}

void GScr_GetDvarFloat()
{
    const char *String; // r3
    const char *VariantString; // r3
    long double v2; // fp2

    String = Scr_GetString(0);
    VariantString = SV_Archived_Dvar_GetVariantString(String);
    v2 = atof(VariantString);
    Scr_AddFloat((float)*(double *)&v2);
}

void GScr_GetDebugDvar()
{
    const char *String; // r3
    const dvar_s *Var; // r3
    const char *v2; // r3

    String = Scr_GetString(0);
    Var = Dvar_FindVar(String);
    if (Var)
    {
        v2 = Dvar_DisplayableValue(Var);
        Scr_AddString(v2);
    }
    else
    {
        Scr_AddString("");
    }
}

void GScr_GetDebugDvarInt()
{
    const char *String; // r3
    const char *VariantString; // r3
    int v2; // r3

    String = Scr_GetString(0);
    VariantString = Dvar_GetVariantString(String);
    v2 = atol(VariantString);
    Scr_AddInt(v2);
}

void GScr_GetDebugDvarFloat()
{
    const char *String; // r3
    const char *VariantString; // r3
    long double v2; // fp2

    String = Scr_GetString(0);
    VariantString = Dvar_GetVariantString(String);
    v2 = atof(VariantString);
    Scr_AddFloat((float)*(double *)&v2);
}

void GScr_SetDvar()
{
    const char *String; // r30
    unsigned int NumParam; // r3
    char *v2; // r3
    char *v3; // r11
    int i; // r9
    char v5; // r10
    const char *v6; // r3
    const dvar_s *Var; // r3
    const dvar_s *v8; // r31
    const char *v9; // r3
    char v10[1024]; // [sp+50h] [-820h] BYREF
    char v11[1056]; // [sp+450h] [-420h] BYREF

    String = Scr_GetString(0);
    if (Scr_GetType(1) == 3)
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(1, NumParam - 1, "Dvar Value", v11, 0x400u);
        v2 = v11;
    }
    else
    {
        v2 = (char *)Scr_GetString(1);
    }
    v3 = v10;
    for (i = 0; i < 1023; ++i)
    {
        v5 = v3[v2 - v10];
        if (!v5)
            break;
        *v3 = v5;
        if (v5 == 34)
            *v3 = 39;
        ++v3;
    }
    *v3 = 0;
    if (!Dvar_IsValidName(String))
    {
        v6 = va("Dvar %s has an invalid dvar name", String);
        Scr_Error(v6);
        return;
    }
    Var = Dvar_FindVar(String);
    v8 = Var;
    if (Var)
    {
        if ((Var->flags & 0x4000) != 0)
            goto LABEL_19;
        if ((Var->flags & 0x400) != 0)
        {
            if (Com_IsRunningMenuLevel())
            {
                Dvar_SetFromStringByNameFromSource(String, v10, DVAR_SOURCE_SCRIPT);
                goto LABEL_19;
            }
            v9 = va(
                "Invalid Dvar set: %s - Menu-Writable Dvars can be changed by script, but only from levels that start with '%s'.\n",
                String,
                "menu_");
        }
        else
        {
            v9 = va(
                "Invalid Dvar set: %s - Internal Dvars cannot be changed by script. Use 'setsaveddvar' to alter SAVED internal dvars\n",
                String);
        }
        Scr_Error(v9);
    LABEL_19:
        if ((v8->flags & 0x4000) == 0)
            return;
    }
    Dvar_SetFromStringByNameFromSource(String, v10, DVAR_SOURCE_SCRIPT);
}

void GScr_SetSavedDvar()
{
    const char *String; // r30
    unsigned int NumParam; // r3
    char *v2; // r31
    int i; // r9
    char *v4; // r11
    char v5; // r10
    const char *v6; // r3
    const dvar_s *Var; // r3
    const char *v8; // r3
    char v9[1024]; // [sp+50h] [-820h] BYREF
    char v10[1056]; // [sp+450h] [-420h] BYREF

    String = Scr_GetString(0);
    if (Scr_GetType(1) == 3)
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(1, NumParam - 1, "Dvar Value", v10, 0x400u);
        v2 = v10;
    }
    else
    {
        v2 = (char *)Scr_GetString(1);
    }
    memset(v9, 0, sizeof(v9));
    for (i = 0; i < 0x2000; ++i)
    {
        v4 = &v9[i];
        v5 = v9[i + v2 - v9];
        if (!v5)
            break;
        *v4 = v5;
        if (v5 == 34)
            *v4 = 39;
    }
    if (Dvar_IsValidName(String))
    {
        Var = Dvar_FindVar(String);
        if (Var)
        {
            // LWSS: because the flags aren't really set, just allow any dvar to be set here (KISAKTODO: requires fixing all dvar declarations, enjoy)
            //if ((Var->flags & 0x1000) != 0)
            //    Dvar_SetFromStringByNameFromSource(String, v9, DVAR_SOURCE_SCRIPT);
            //else
            //    Scr_Error("SetSavedDvar can only be called on dvars with the SAVED flag set");
            Dvar_SetFromStringByNameFromSource(String, v9, DVAR_SOURCE_SCRIPT);
        }
        else
        {
            v8 = va("SetSavedDvar(): The dvar \"%s\" does not exist.", String);
            Scr_Error(v8);
        }
    }
    else
    {
        v6 = va("Dvar %s has an invalid dvar name", String);
        Scr_Error(v6);
    }
}

void GScr_GetTime()
{
    Scr_AddInt(level.time);
}

void GScr_GetDifficulty()
{
    const dvar_s *v0; // r11

    v0 = sv_gameskill;
    if (sv_gameskill->current.integer >= 4u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            1485,
            0,
            "%s",
            "sv_gameskill->current.integer >= DIFFICULTY_EASY && sv_gameskill->current.integer <= DIFFICULTY_FU");
        v0 = sv_gameskill;
    }
    Scr_AddString(difficultyStrings[v0->current.integer]);
}

void Scr_GetEntByNum()
{
    unsigned int Int; // r3
    gentity_s *v1; // r3

    Int = Scr_GetInt(0);
    if (Int < 0x880)
    {
        v1 = &g_entities[Int];
        if (v1->r.inuse)
            Scr_AddEntity(v1);
    }
}

int __cdecl Scr_GetTeamFlag(const char *pszTeamName, const char *pszCaller)
{
    const char *v5; // r3

    if (!I_stricmp(pszTeamName, "axis"))
        return 2;
    if (!I_stricmp(pszTeamName, "allies"))
        return 4;
    if (!I_stricmp(pszTeamName, "neutral"))
        return 8;
    if (!I_stricmp(pszTeamName, "all"))
        return 14;
    v5 = va("unknown team '%s' in %s (should be axis, allies, or neutral)", pszTeamName, pszCaller);
    Scr_Error(v5);
    return 0;
}

int __cdecl Scr_GetTeamFlags(unsigned int i, const char *pszCaller)
{
    unsigned int v2; // r31
    int j; // r30
    const char *String; // r3

    v2 = i;
    for (j = 0; v2 < Scr_GetNumParam(); j |= Scr_GetTeamFlag(String, pszCaller))
        String = Scr_GetString(v2++);
    return j;
}

int __cdecl Scr_GetSpecies(unsigned __int16 speciesString)
{
    int v1; // r10
    const unsigned __int16 **v2; // r11
    const char *String; // r3
    const char *v4; // r3

    if (speciesString == scr_const.all)
        return 2;
    v1 = 0;
    v2 = g_AISpeciesNames;
    while (speciesString != **v2)
    {
        ++v2;
        ++v1;
        if ((int)v2 >= (int)g_entinfoAITextNames)
        {
            String = Scr_GetString(speciesString);
            v4 = va("unknown species '%s' (should be human, dog, or all)", String);
            Scr_Error(v4);
            return 2;
        }
    }
    return v1;
}

actor_s *Scr_GetAIArray()
{
    int TeamFlags; // r30
    actor_s *result; // r3
    actor_s *i; // r31

    TeamFlags = Scr_GetTeamFlags(0, "getaiarray");
    if (!TeamFlags)
        TeamFlags = 14;
    Scr_MakeArray();
    result = Actor_FirstActor(TeamFlags);
    for (i = result; result; i = result)
    {
        if (i->Physics.bIsAlive && !i->delayedDeath && i->species == AI_SPECIES_HUMAN)
        {
            Scr_AddEntity(i->ent);
            Scr_AddArray();
        }
        result = Actor_NextActor(i, TeamFlags);
    }
    return result;
}

actor_s *Scr_GetAISpeciesArray()
{
    const char *String; // r3
    int TeamFlag; // r29
    unsigned __int16 ConstString; // r3
    int Species; // r30
    actor_s *result; // r3
    actor_s *i; // r31

    if (Scr_GetNumParam())
    {
        String = Scr_GetString(0);
        TeamFlag = Scr_GetTeamFlag(String, "getaiarray");
    }
    else
    {
        TeamFlag = 14;
    }
    if (Scr_GetNumParam() <= 1)
    {
        Species = 2;
    }
    else
    {
        ConstString = Scr_GetConstString(1);
        Species = Scr_GetSpecies(ConstString);
    }
    Scr_MakeArray();
    result = Actor_FirstActor(TeamFlag);
    for (i = result; result; i = result)
    {
        if (i->Physics.bIsAlive && !i->delayedDeath && (Species == i->species || Species == 2))
        {
            Scr_AddEntity(i->ent);
            Scr_AddArray();
        }
        result = Actor_NextActor(i, TeamFlag);
    }
    return result;
}

void Scr_GetSpawnerArray()
{
    int v0; // r29
    int num_entities; // r10
    int v2; // r31
    gentity_s *v3; // r3

    if (Scr_GetNumParam())
        Scr_Error("cannot call getspawnerarray with parameters");
    Scr_MakeArray();
    v0 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        v2 = 0;
        do
        {
            v3 = &level.gentities[v2];
            if (level.gentities[v2].r.inuse)
            {
                if (v3->s.eType == 15)
                {
                    Scr_AddEntity(v3);
                    Scr_AddArray();
                    num_entities = level.num_entities;
                }
            }
            ++v0;
            ++v2;
        } while (v0 < num_entities);
    }
}

void Scr_GetSpawnerTeamArray()
{
    int TeamFlags; // r27
    int v1; // r29
    int num_entities; // r10
    int v3; // r31
    gentity_s *v4; // r3

    TeamFlags = Scr_GetTeamFlags(0, "getspawnerteamarray");
    if (!TeamFlags)
        Scr_Error("no team was specified - use getspawnerarray instead");
    Scr_MakeArray();
    v1 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        v3 = 0;
        do
        {
            v4 = &level.gentities[v3];
            if (level.gentities[v3].r.inuse && v4->s.eType == 15 && ((1 << v4->item[0].ammoCount) & TeamFlags) != 0)
            {
                Scr_AddEntity(v4);
                Scr_AddArray();
                num_entities = level.num_entities;
            }
            ++v1;
            ++v3;
        } while (v1 < num_entities);
    }
}

void Scr_GetWeaponModel()
{
    const char *String; // r31
    unsigned int WeaponIndexForName; // r29
    unsigned int Int; // r30
    const char *v3; // r3
    WeaponDef *WeaponDef; // r3
    const char *Name; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Int = 0;
    if (WeaponIndexForName)
    {
        if (Scr_GetNumParam() == 2)
        {
            Int = Scr_GetInt(1);
            if (Int > 0xFF || !BG_GetWeaponDef(WeaponIndexForName)->worldModel[Int])
                Int = 0;
        }
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        Name = XModelGetName(WeaponDef->worldModel[Int]);
        Scr_AddString(Name);
    }
    else
    {
        if (*String)
        {
            if (I_stricmp(String, "none"))
            {
                v3 = va("unknown weapon '%s' in getWeaponModel\n", String);
                Com_Printf(23, v3);
            }
        }
        Scr_AddString("");
    }
}

void Scr_GetWeaponClipModel()
{
    const char *String; // r31
    unsigned int WeaponIndexForName; // r3
    unsigned int v2; // r30
    const char *v3; // r3
    const char *Name; // r3
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    v2 = WeaponIndexForName;
    if (WeaponIndexForName)
    {
        if (BG_GetWeaponDef(WeaponIndexForName)->worldClipModel)
        {
            WeaponDef = BG_GetWeaponDef(v2);
            Name = XModelGetName(WeaponDef->worldClipModel);
            goto LABEL_6;
        }
    }
    else if (*String && I_stricmp(String, "none"))
    {
        v3 = va("unknown weapon '%s' in getWeaponClipModel\n", String);
        Com_Printf(23, v3);
    }
    Name = "";
LABEL_6:
    Scr_AddString(Name);
}

void __cdecl GScr_GetAnimLength()
{
    XAnim_s *anims; // r31
    scr_anim_s anim; // [sp+50h] [-20h]

    anim = Scr_GetAnim(0, 0);
    anims = Scr_GetAnims(anim.tree);
    if (!XAnimIsPrimitive(anims, anim.index))
        Scr_ParamError(0, "non-primitive animation has no concept of length");
    Scr_AddFloat(XAnimGetLength(anims, anim.index));
}

void __cdecl GScr_AnimHasNotetrack()
{
    unsigned int ConstString; // r31
    const XAnim_s *Anims; // r3
    scr_anim_s Anim; // [sp+50h] [-20h]

    Anim = (scr_anim_s)Scr_GetAnim(0, 0);
    ConstString = Scr_GetConstString(1);
    Anims = Scr_GetAnims(Anim.tree);
    Scr_AddBool(XAnimNotetrackExists(Anims, Anim.index, ConstString));
}

void __cdecl GScr_GetNotetrackTimes()
{
    unsigned int ConstString; // r31
    const XAnim_s *Anims; // r3
    scr_anim_s Anim; // [sp+50h] [-20h]

    Anim = (scr_anim_s)Scr_GetAnim(0, 0);
    ConstString = Scr_GetConstString(1);
    Scr_MakeArray();
    Anims = Scr_GetAnims(Anim.tree);
    XAnimAddNotetrackTimesToScriptArray(Anims, Anim.index, ConstString);
}

void GScr_GetBrushModelCenter()
{
    Scr_Error("Depricated - use GetOrigin.\n");
}

void GScr_GetKeyBinding()
{
    const char *String; // r3
    int KeyBinding; // r30
    char v2[128]; // [sp+50h] [-120h] BYREF
    char v3[136]; // [sp+D0h] [-A0h] BYREF

    String = Scr_GetString(0);
    KeyBinding = CL_GetKeyBinding(0, String, (char (*)[128])v2);
    Scr_MakeArray();
    Scr_AddIString(v2);
    Scr_AddArrayStringIndexed(scr_const.key1);
    Scr_AddIString(v3);
    Scr_AddArrayStringIndexed(scr_const.key2);
    Scr_AddInt(KeyBinding);
    Scr_AddArrayStringIndexed(scr_const.count);
}

void GScr_GetCommandFromKey()
{
    const char *String; // r3
    const char *CommandFromKey; // r3

    String = Scr_GetString(0);
    CommandFromKey = CL_GetCommandFromKey(String);
    if (!CommandFromKey)
        CommandFromKey = "";
    Scr_AddString(CommandFromKey);
}

void GScr_Spawn()
{
    unsigned int ConstString; // r29
    int Int; // r30
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    float v5[12]; // [sp+50h] [-30h] BYREF

    ConstString = Scr_GetConstString(0);
    Scr_GetVector(1u, v5);
    if (Scr_GetNumParam() <= 2)
        Int = 0;
    else
        Int = Scr_GetInt(2);
    v2 = G_Spawn();
    Scr_SetString(&v2->classname, ConstString);
    v2->r.currentOrigin[0] = v5[0];
    v2->r.currentOrigin[1] = v5[1];
    v2->r.currentOrigin[2] = v5[2];
    v2->spawnflags = Int;
    if (G_CallSpawnEntity(v2))
    {
        Scr_AddEntity(v2);
    }
    else
    {
        v3 = SL_ConvertToString(ConstString);
        v4 = va("unable to spawn \"%s\" entity", v3);
        Scr_Error(v4);
    }
}

void GScr_SpawnVehicle()
{
    const char *String; // r30
    unsigned int ConstString; // r29
    unsigned int v2; // r28
    gentity_s *v3; // r31
    const char *v4; // r3
    float v5[4]; // [sp+50h] [-50h] BYREF
    float v6[16]; // [sp+60h] [-40h] BYREF

    String = Scr_GetString(0);
    ConstString = Scr_GetConstString(1);
    v2 = Scr_GetConstString(2);
    Scr_GetVector(3u, v5);
    Scr_GetVector(4u, v6);
    v3 = G_Spawn();
    Scr_SetString(&v3->classname, scr_const.script_vehicle);
    G_SetModel(v3, String);
    Scr_SetString(&v3->targetname, ConstString);
    v3->r.currentOrigin[0] = v5[0];
    v3->r.currentOrigin[1] = v5[1];
    v3->r.currentOrigin[2] = v5[2];
    v3->r.currentAngles[0] = v6[0];
    v3->r.currentAngles[1] = v6[1];
    v3->r.currentAngles[2] = v6[2];
    v4 = SL_ConvertToString(v2);
    G_SpawnVehicle(v3, v4, 0);
    Scr_AddEntity(v3);
}

void GScr_SpawnTurret()
{
    unsigned int ConstString; // r30
    const char *String; // r29
    gentity_s *v2; // r31
    float v3[12]; // [sp+50h] [-30h] BYREF

    ConstString = Scr_GetConstString(0);
    Scr_GetVector(1u, v3);
    String = Scr_GetString(2);
    v2 = G_Spawn();
    Scr_SetString(&v2->classname, ConstString);
    v2->r.currentOrigin[0] = v3[0];
    v2->r.currentOrigin[1] = v3[1];
    v2->r.currentOrigin[2] = v3[2];
    G_SpawnTurret(v2, String);
    Scr_AddEntity(v2);
}

void GScr_CanSpawnTurret()
{
    int CanSpawnTurret; // r3

    CanSpawnTurret = G_CanSpawnTurret();
    Scr_AddBool(CanSpawnTurret);
}

int GScr_PrecacheTurret()
{
    const char *String; // r3

    if (!level.initializing)
        Scr_Error("precacheTurret must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    return G_GetWeaponIndexForName(String);
}

void __cdecl ScrCmd_startIgnoringSpotLight(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags |= 0x80u;
}

void __cdecl ScrCmd_stopIgnoringSpotLight(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags &= ~0x80u;
}

void __cdecl ScrCmd_attach(scr_entref_t entref)
{
    gentity_s *Entity; // r29
    const char *String; // r30
    unsigned int ConstLowercaseString; // r31
    int Int; // r28
    const char *v5; // r3
    const char *v6; // r3
    const char *v7; // r3
    const char *v8; // r3

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    if (Scr_GetNumParam() < 2)
        ConstLowercaseString = scr_const._;
    else
        ConstLowercaseString = Scr_GetConstLowercaseString(1);
    if (Scr_GetNumParam() < 3)
        Int = 0;
    else
        Int = Scr_GetInt(2);
    if (G_EntDetach(Entity, String, ConstLowercaseString))
    {
        v5 = SL_ConvertToString(ConstLowercaseString);
        v6 = va("model '%s' already attached to tag '%s'", String, v5);
        Scr_Error(v6);
    }
    if (!G_EntAttach(Entity, String, ConstLowercaseString, Int))
    {
        v7 = SL_ConvertToString(ConstLowercaseString);
        v8 = va("failed to attach model '%s' to tag '%s'", String, v7);
        Scr_Error(v8);
    }
}

void __cdecl ScrCmd_detach(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *String; // r27
    unsigned int ConstLowercaseString; // r28
    unsigned __int16 *attachModelNames; // r31
    int v5; // r30
    const char *v6; // r26
    unsigned int v7; // r3
    const char *v8; // r3
    const char *v9; // r3
    const char *v10; // r3

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    if (Scr_GetNumParam() < 2)
        ConstLowercaseString = scr_const._;
    else
        ConstLowercaseString = Scr_GetConstLowercaseString(1);
    if (!G_EntDetach(Entity, String, ConstLowercaseString))
    {
        Com_Printf(23, "Current attachments:\n");
        attachModelNames = Entity->attachModelNames;
        v5 = 31;
        do
        {
            if (*attachModelNames)
            {
                if (attachModelNames[31])
                {
                    v6 = SL_ConvertToString(attachModelNames[31]);
                    v7 = G_ModelName(*attachModelNames);
                    v8 = SL_ConvertToString(v7);
                    Com_Printf(23, "model: '%s', tag: '%s'\n", v8, v6);
                }
            }
            --v5;
            ++attachModelNames;
        } while (v5);
        v9 = SL_ConvertToString(ConstLowercaseString);
        v10 = va("failed to detach model '%s' from tag '%s'", String, v9);
        Scr_Error(v10);
    }
}

void __cdecl ScrCmd_detachAll(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    G_EntDetachAll(Entity);
}

void __cdecl ScrCmd_GetAttachSize(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    int v2; // r11
    unsigned __int16 *attachModelNames; // r10

    Entity = GetEntity(entref);
    v2 = 0;
    attachModelNames = Entity->attachModelNames;
    do
    {
        if (!*attachModelNames)
            break;
        ++v2;
        ++attachModelNames;
    } while (v2 < 31);
    Scr_AddInt(v2);
}

void __cdecl ScrCmd_GetAttachModelName(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    unsigned int Int; // r3
    unsigned int v3; // r31
    unsigned int v4; // r3

    Entity = GetEntity(entref);
    Int = Scr_GetInt(0);
    v3 = Int;
    if (Int >= 0x1F || !Entity->attachModelNames[Int])
        Scr_ParamError(0, "bad index");
    v4 = G_ModelName(Entity->attachModelNames[v3]);
    Scr_AddConstString(v4);
}

void __cdecl ScrCmd_GetAttachTagName(scr_entref_t entref)
{
    gentity_s *ent; // r30
    unsigned int i; // r3

    ent = GetEntity(entref);
    i = Scr_GetInt(0);

    if (i >= 0x1F || !ent->attachModelNames[i])
        Scr_ParamError(0, "bad index");

    iassert(ent->attachTagNames[i]);
    Scr_AddConstString(ent->attachTagNames[i]);
}

void __cdecl ScrCmd_GetAttachIgnoreCollision(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    unsigned int Int; // r3
    char v3; // r31

    Entity = GetEntity(entref);
    Int = Scr_GetInt(0);
    v3 = Int;
    if (Int >= 0x1F || !Entity->attachModelNames[Int])
        Scr_ParamError(0, "bad index");
    Scr_AddBool((Entity->attachIgnoreCollision & (1 << v3)) != 0);
}

void __cdecl ScrCmd_hidepart(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    DObj_s *ServerDObj; // r29
    unsigned int ConstLowercaseString; // r31
    const char *v4; // r3
    const char *v5; // r3
    const char *String; // r30
    const char *v7; // r3
    unsigned __int8 v8[16]; // [sp+50h] [-40h] BYREF
    unsigned int v9[12]; // [sp+60h] [-30h] BYREF

    Entity = GetEntity(entref);
    ServerDObj = Com_GetServerDObj(Entity->s.number);
    if (!ServerDObj)
        Scr_Error("entity has no model");
    v8[0] = -2;
    ConstLowercaseString = Scr_GetConstLowercaseString(0);
    if (Scr_GetNumParam() == 1)
    {
        if (!DObjGetBoneIndex(ServerDObj, ConstLowercaseString, v8))
        {
            v4 = SL_ConvertToString(ConstLowercaseString);
            v5 = va("cannot find part '%s' in entity model", v4);
        LABEL_8:
            Scr_Error(v5);
        }
    }
    else
    {
        String = Scr_GetString(1);
        if (!DObjGetModelBoneIndex(ServerDObj, String, ConstLowercaseString, v8))
        {
            v7 = SL_ConvertToString(ConstLowercaseString);
            v5 = va("cannot find part '%s' in entity model '%s'", v7, String);
            goto LABEL_8;
        }
    }
    DObjGetHidePartBits(ServerDObj, v9);
    *(unsigned int *)((char *)v9 + ((v8[0] >> 3) & 0x1FFFFFFC)) |= 0x80000000 >> (v8[0] & 0x1F);
    DObjSetHidePartBits(ServerDObj, v9);
}

void __cdecl ScrCmd_showpart(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    DObj_s *ServerDObj; // r29
    unsigned int ConstLowercaseString; // r31
    const char *v4; // r3
    const char *v5; // r3
    const char *String; // r30
    const char *v7; // r3
    unsigned __int8 v8[16]; // [sp+50h] [-40h] BYREF
    unsigned int v9[12]; // [sp+60h] [-30h] BYREF

    Entity = GetEntity(entref);
    ServerDObj = Com_GetServerDObj(Entity->s.number);
    if (!ServerDObj)
        Scr_Error("entity has no model");
    v8[0] = -2;
    ConstLowercaseString = Scr_GetConstLowercaseString(0);
    if (Scr_GetNumParam() == 1)
    {
        if (!DObjGetBoneIndex(ServerDObj, ConstLowercaseString, v8))
        {
            v4 = SL_ConvertToString(ConstLowercaseString);
            v5 = va("cannot find part '%s' in entity model", v4);
        LABEL_8:
            Scr_Error(v5);
        }
    }
    else
    {
        String = Scr_GetString(1);
        if (!DObjGetModelBoneIndex(ServerDObj, String, ConstLowercaseString, v8))
        {
            v7 = SL_ConvertToString(ConstLowercaseString);
            v5 = va("cannot find part '%s' in entity model '%s'", v7, String);
            goto LABEL_8;
        }
    }
    DObjGetHidePartBits(ServerDObj, v9);
    *(unsigned int *)((char *)v9 + ((v8[0] >> 3) & 0x1FFFFFFC)) &= ~(0x80000000 >> (v8[0] & 0x1F));
    DObjSetHidePartBits(ServerDObj, v9);
}

void __cdecl ScrCmd_showallparts(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    DObj_s *ServerDObj; // r31
    __int64 v3; // r10
    _QWORD v4[2]; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    ServerDObj = Com_GetServerDObj(Entity->s.number);
    if (!ServerDObj)
        Scr_Error("entity has no model");
    LODWORD(v3) = 0;
    v4[0] = v3;
    v4[1] = v3;
    DObjSetHidePartBits(ServerDObj, (const unsigned int *)v4);
}

void __cdecl ScrCmd_LinkTo(scr_entref_t entref)
{
    gentity_s *Entity; // r28
    const char *EntityTypeName; // r31
    const char *v3; // r3
    const char *v4; // r3
    gentity_s *v5; // r31
    int NumParam; // r29
    unsigned int ConstLowercaseString; // r30
    int v8; // r3
    unsigned int v9; // r3
    const char *v10; // r3
    const char *v11; // r3
    unsigned int v12; // r3
    const char *v13; // r31
    const char *v14; // r3
    const char *v15; // r3
    float v16[4]; // [sp+50h] [-50h] BYREF
    float v17[16]; // [sp+60h] [-40h] BYREF

    Entity = GetEntity(entref);
    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        Scr_ParamError(0, "not an entity");
    if ((Entity->flags & 0x800) == 0)
    {
        EntityTypeName = G_GetEntityTypeName(Entity);
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity (classname: '%s', type: '%s') does not currently support linkTo", v3, EntityTypeName);
        Scr_ObjectError(v4);
    }
    v5 = Scr_GetEntity(0);
    NumParam = Scr_GetNumParam();
    ConstLowercaseString = 0;
    if (NumParam < 2)
        goto LABEL_10;
    ConstLowercaseString = Scr_GetConstLowercaseString(1);
    if (ConstLowercaseString == scr_const._)
        ConstLowercaseString = 0;
    if (NumParam > 2)
    {
        Scr_GetVector(2u, v17);
        Scr_GetVector(3u, v16);
        v8 = G_EntLinkToWithOffset(Entity, v5, ConstLowercaseString, v17, v16);
    }
    else
    {
    LABEL_10:
        v8 = G_EntLinkTo(Entity, v5, ConstLowercaseString);
    }
    if (!v8)
    {
        if (!SV_DObjExists(v5))
        {
            if (!v5->model)
                Scr_Error("failed to link entity since parent has no model");
            v9 = G_ModelName(v5->model);
            v10 = SL_ConvertToString(v9);
            v11 = va("failed to link entity since parent model '%s' is invalid", v10);
            Scr_Error(v11);
        }
        if (!v5->model)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 2526, 0, "%s", "parent->model");
        if (ConstLowercaseString)
        {
            if (SV_DObjGetBoneIndex(v5, ConstLowercaseString) < 0)
            {
                SV_DObjDumpInfo(v5);
                v12 = G_ModelName(v5->model);
                v13 = SL_ConvertToString(v12);
                v14 = SL_ConvertToString(ConstLowercaseString);
                v15 = va("failed to link entity since tag '%s' does not exist in parent model '%s'", v14, v13);
                Scr_Error(v15);
            }
        }
        Scr_Error("failed to link entity");
    }
}

void __cdecl ScrCmd_SetMoveSpeedScale(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r29

    PlayerEntity = GetPlayerEntity(entref);
    if (!PlayerEntity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 2554, 0, "%s", "ent");
    if (!PlayerEntity->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 2555, 0, "%s", "ent->client");
    PlayerEntity->client->pers.moveSpeedScaleMultiplier = Scr_GetFloat(0);
}

void ScrCmd_GetTimeScale()
{
    Scr_AddFloat(com_timescale->current.value);
}

void ScrCmd_SetTimeScale()
{
    double Float; // fp1

    Float = Scr_GetFloat(0);
    Dvar_SetFloat(com_timescale, Float);
}

// aislop
void ScrCmd_SetPhysicsGravityDir()
{
    float v7[3];
    double v2, v5, v6;
    const char *v4;

    // Get vector from parameter 0
    Scr_GetVector(0, v7);

    // Compute the magnitude (length) of the vector
    float magnitude = sqrtf(v7[0] * v7[0] + v7[1] * v7[1] + v7[2] * v7[2]);

    // Avoid division by zero
    if (magnitude == 0.0f) {
        Scr_Error("Zero vector length detected, cannot normalize.");
        return;
    }

    // Normalize the vector
    float invMagnitude = 1.0f / magnitude;
    v7[0] *= invMagnitude;
    v7[1] *= invMagnitude;
    v7[2] *= invMagnitude;

    // Prepare the formatted string for the server command
    v4 = va("phys_grav %f %f %f\n", v7[0], v7[1], v7[2]);

    // Send the command to the server
    SV_GameSendServerCommand(-1, v4);
}


void __cdecl ScrCmd_PlayerSetGroundReferenceEnt(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    int number; // r9

    Entity = GetEntity(entref);
    if (!Entity->client)
        Scr_ObjectError("not a player entity");
    if (Scr_GetType(0))
    {
        if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
            Scr_ParamError(0, "not an entity");
        number = Scr_GetEntity(0)->s.number;
    }
    else
    {
        number = ENTITYNUM_NONE;
    }
    Entity->client->groundTiltEntNum = number;
}

// aislop
void __cdecl ScrCmd_PlayerLinkTo(scr_entref_t entref)
{
    gentity_s *Entity = GetEntity(entref);
    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        Scr_ParamError(0, "not an entity");

    if (!Entity->client)
        Scr_ObjectError("not a player entity");

    if ((Entity->flags & 0x800) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            2680, 0, "%s", "ent->flags & FL_SUPPORTS_LINKTO"
        );

    gentity_s *v2 = Scr_GetEntity(0);
    int NumParam = Scr_GetNumParam();
    unsigned int ConstLowercaseString = 0;

    // Set ConstLowercaseString if valid
    if (NumParam > 1 && Scr_GetType(1) && Scr_GetConstLowercaseString(1) != scr_const._)
        ConstLowercaseString = Scr_GetConstLowercaseString(1);

    // Default value for linkAnglesFrac
    float linkAnglesFrac = (NumParam > 2) ? Scr_GetFloat(2) : 0.0f;
    Entity->client->linkAnglesFrac = linkAnglesFrac;
    Entity->client->linkAnglesLocked = 0;
    Entity->client->link_rotationMovesEyePos = 0;
    Entity->client->link_useTagAnglesForViewAngles = 1;

    // Set link_doCollision based on parameter 7
    //bool linkDoCollision = (NumParam > 7 && (_cntlzw(Scr_GetInt(7)) & 0x20) == 0);
    bool linkDoCollision = (NumParam > 7 && Scr_GetInt(7));
    Entity->client->link_doCollision = linkDoCollision;

    // Set min/max clamp values
    float clampAngles[4] = { 180.0f, 180.0f, 180.0f, 180.0f };
    for (int i = 3; i < NumParam && i < 7; i++) {
        clampAngles[i - 3] = Scr_GetFloat(i);
    }

    // Apply clamping angles
    for (int i = 0; i < 2; i++) {
        float angle = clampAngles[i];
        angle -= 180.0f;  // Adjust angle
        Entity->client->linkAnglesMinClamp[i] = -angle;
        Entity->client->linkAnglesMaxClamp[i] = angle;
    }

    // Update view angle clamp
    G_UpdateViewAngleClamp(Entity->client, v2->r.currentAngles);

    // Reset player flags
    Entity->client->ps.pm_flags &= ~0x1000000u;
    Entity->client->prevLinkAnglesSet = 0;

    // Link entities
    if (G_EntLinkTo(Entity, v2, ConstLowercaseString)) {
        gclient_s *client = Entity->client;
        if (client->link_useTagAnglesForViewAngles) {
            float v28[4][3];
            G_CalcTagParentAxis(Entity, v28);
            AxisToAngles((const mat3x3&)v28, client->ps.linkAngles);
        }
        else {
            client->ps.linkAngles[0] = client->ps.linkAngles[1] = client->ps.linkAngles[2] = 0.0f;
        }
    }
    else {
        Scr_Error("failed to link entity");
    }
}


//aislop
void __cdecl ScrCmd_PlayerLinkToDelta(scr_entref_t entref)
{
    gentity_s *Entity = GetEntity(entref);
    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        Scr_ParamError(0, "not an entity");

    if (!Entity->client)
        Scr_ObjectError("not a player entity");

    if ((Entity->flags & 0x800) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            2760, 0, "%s", "ent->flags & FL_SUPPORTS_LINKTO"
        );

    gentity_s *v2 = Scr_GetEntity(0);
    int NumParam = Scr_GetNumParam();
    unsigned int ConstLowercaseString = 0;

    if (NumParam > 1 && Scr_GetType(1) && Scr_GetConstLowercaseString(1) != scr_const._)
        ConstLowercaseString = Scr_GetConstLowercaseString(1);

    float linkAnglesFrac = (NumParam > 2) ? Scr_GetFloat(2) : 0.0f;
    Entity->client->linkAnglesFrac = linkAnglesFrac;
    Entity->client->linkAnglesLocked = 0;
    Entity->client->link_rotationMovesEyePos = 1;

    // Set link angles clamp
    float clampAngles[4] = { 180.0f, 180.0f, 180.0f, 180.0f };
    for (int i = 3; i < NumParam && i < 7; i++) {
        clampAngles[i - 3] = Scr_GetFloat(i);
    }

    // Set min and max clamping angles
    for (int i = 0; i < 2; i++) {
        float angle = clampAngles[i];
        angle -= 180.0f;  // Adjust angle
        Entity->client->linkAnglesMinClamp[i] = -angle;
        Entity->client->linkAnglesMaxClamp[i] = angle;
    }

    // Update the view angle clamp
    G_UpdateViewAngleClamp(Entity->client, v2->r.currentAngles);

    // Link angles flag for view angles
    //bool useTagAnglesForViewAngles = (NumParam > 7 && (_cntlzw(Scr_GetInt(7)) & 0x20) == 0);
    bool useTagAnglesForViewAngles = (NumParam > 7 && Scr_GetInt(7));
    Entity->client->link_useTagAnglesForViewAngles = useTagAnglesForViewAngles;
    Entity->client->ps.pm_flags &= ~0x1000000u;
    Entity->client->prevLinkAnglesSet = 0;

    // Link entities
    if (G_EntLinkTo(Entity, v2, ConstLowercaseString)) {
        gclient_s *client = Entity->client;
        if (client->link_useTagAnglesForViewAngles) {
            float v28[4][3];
            G_CalcTagParentAxis(Entity, v28);
            AxisToAngles((const mat3x3&)v28, client->ps.linkAngles);
        }
        else {
            client->ps.linkAngles[0] = client->ps.linkAngles[1] = client->ps.linkAngles[2] = 0.0f;
        }
    }
    else {
        Scr_Error("failed to link entity");
    }
}


void __cdecl ScrCmd_PlayerLinkToAbsolute(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    gentity_s *v2; // r29
    unsigned int ConstLowercaseString; // r30
    float *p_commandTime; // r11

    Entity = GetEntity(entref);
    if (Scr_GetType(0) != 1 || Scr_GetPointerType(0) != 20)
        Scr_ParamError(0, "Not an entity");
    if (!Entity->client)
        Scr_ObjectError("Not a player entity");
    if ((Entity->flags & 0x800) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            2833,
            0,
            "%s",
            "ent->flags & FL_SUPPORTS_LINKTO");
    v2 = Scr_GetEntity(0);
    ConstLowercaseString = 0;
    if ((int)Scr_GetNumParam() > 1)
    {
        if (Scr_GetType(1))
        {
            ConstLowercaseString = Scr_GetConstLowercaseString(1);
            if (ConstLowercaseString == scr_const._)
                ConstLowercaseString = 0;
        }
    }
    Entity->client->linkAnglesFrac = 1.0;
    Entity->client->linkAnglesLocked = 1;
    Entity->client->ps.pm_flags |= 0x1000000u;
    p_commandTime = (float *)&Entity->client->ps.commandTime;
    p_commandTime[351] = 0.0;
    p_commandTime[352] = 0.0;
    p_commandTime[353] = 0.0;
    Entity->client->link_rotationMovesEyePos = 1;
    if (!G_EntLinkTo(Entity, v2, ConstLowercaseString))
        Scr_Error("Failed to link entity");
}

void __cdecl ScrCmd_Unlink(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    gclient_s *client; // r11

    Entity = GetEntity(entref);
    G_EntUnlink(Entity);
    client = Entity->client;
    if (client)
    {
        client->linkAnglesFrac = 0.0;
        Entity->client->linkAnglesLocked = 0;
        Entity->client->ps.pm_flags &= ~0x1000000u;
    }
}

void __cdecl ScrCmd_EnableLinkTo(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *EntityTypeName; // r30

    Entity = GetEntity(entref);
    if ((Entity->flags & FL_SUPPORTS_LINKTO) != 0)
        Scr_ObjectError("entity already has linkTo enabled");
    if (Entity->s.eType || Entity->physicsObject)
    {
        EntityTypeName = G_GetEntityTypeName(Entity);
        Scr_ObjectError(va("entity (classname: '%s', type: '%s') does not currently support enableLinkTo", SL_ConvertToString(Entity->classname), EntityTypeName));
    }
    if (Entity->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 2907, 0, "%s", "!ent->client");
    Entity->flags |= FL_SUPPORTS_LINKTO;
}

void __cdecl ScrCmd_DontInterpolate(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gclient_s *client; // r11

    Entity = GetEntity(entref);
    client = Entity->client;
    if (client)
        client->ps.eFlags ^= 2u;
    else
        Entity->s.lerp.eFlags ^= 2u;
}

void __cdecl ScrCmd_dospawn(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    unsigned int targetname; // r3
    const char *v4; // r30
    double v5; // fp31
    double v6; // fp30
    double v7; // fp29
    const char *v8; // r3
    int Int; // r29
    unsigned int ConstString; // r30
    gentity_s *v11; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (Entity->s.eType != 15)
    {
        targetname = Entity->targetname;
        if (v2->targetname)
            v4 = SL_ConvertToString(targetname);
        else
            v4 = "<unnamed>";
        v5 = v2->r.currentOrigin[2];
        v6 = v2->r.currentOrigin[1];
        v7 = v2->r.currentOrigin[0];
        SL_ConvertToString(v2->classname);
        v8 = va(
            "dospawn can only be called on actor spawners\n"
            "attempted to call dospawn on entity with name '%s' of type '%s' at (%.0f %.0f %.0f)\n",
            v4,
            (const char *)HIDWORD(v7),
            v7,
            v6,
            v5);
        Scr_Error(v8);
    }
    if (v2->item[0].clipAmmoCount < level.time)
    {
        Int = 0;
        if (Scr_GetNumParam())
            Int = Scr_GetInt(0);
        ConstString = 0;
        if (Scr_GetNumParam() >= 2)
            ConstString = Scr_GetConstString(1);
        v11 = SpawnActor(v2, ConstString, CHECK_SPAWN, Int == 0);
        if (v11)
        {
            v2->item[0].clipAmmoCount = level.time;
            Scr_AddEntity(v11);
        }
    }
}

void __cdecl ScrCmd_StalingradSpawn(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    unsigned int targetname; // r3
    const char *v4; // r30
    double v5; // fp31
    double v6; // fp30
    double v7; // fp29
    const char *v8; // r3
    int Int; // r29
    unsigned int ConstString; // r30
    gentity_s *v11; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (Entity->s.eType != 15)
    {
        targetname = Entity->targetname;
        if (v2->targetname)
            v4 = SL_ConvertToString(targetname);
        else
            v4 = "<unnamed>";
        v5 = v2->r.currentOrigin[2];
        v6 = v2->r.currentOrigin[1];
        v7 = v2->r.currentOrigin[0];
        SL_ConvertToString(v2->classname);
        v8 = va(
            "dospawn can only be called on actor spawners\n"
            "attempted to call dospawn on entity with name '%s' of type '%s' at (%.0f %.0f %.0f)\n",
            v4,
            (const char *)HIDWORD(v7),
            v7,
            v6,
            v5);
        Scr_Error(v8);
    }
    if (v2->item[0].clipAmmoCount < level.time)
    {
        Int = 0;
        if (Scr_GetNumParam())
            Int = Scr_GetInt(0);
        ConstString = 0;
        if (Scr_GetNumParam() >= 2)
            ConstString = Scr_GetConstString(1);
        v11 = SpawnActor(v2, ConstString, FORCE_SPAWN, Int == 0);
        if (v11)
        {
            v2->item[0].clipAmmoCount = level.time;
            Scr_AddEntity(v11);
        }
    }
}

void __cdecl ScrCmd_GetOrigin(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    float v2[6]; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    if (Entity->r.bmodel)
    {
        G_EntityCentroid(Entity, v2);
    }
    else
    {
        v2[0] = Entity->r.currentOrigin[0];
        v2[1] = Entity->r.currentOrigin[1];
        v2[2] = Entity->r.currentOrigin[2];
    }
    Scr_AddVector(v2);
}

void __cdecl ScrCmd_GetCentroid(scr_entref_t entref)
{
    const gentity_s *Entity; // r3
    float v2[6]; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    G_EntityCentroid(Entity, v2);
    Scr_AddVector(v2);
}

void __cdecl ScrCmd_GetStance(scr_entref_t entref)
{
    gclient_s *client; // r11
    int pm_flags; // r11

    client = GetEntity(entref)->client;
    if (client)
    {
        pm_flags = client->ps.pm_flags;
        if ((pm_flags & 1) != 0)
        {
            Scr_AddConstString(scr_const.prone);
        }
        else if ((pm_flags & 2) != 0)
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

void __cdecl ScrCmd_SetStance(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    __int16 ConstString; // r3
    gclient_s *client; // r11
    unsigned int v4; // r10
    unsigned int v5; // r10
    int pm_flags; // r10

    Entity = GetEntity(entref);
    ConstString = Scr_GetConstString(0);
    client = Entity->client;
    if (client)
    {
        if (ConstString == scr_const.stand)
        {
            v4 = client->ps.pm_flags & 0xFFFFFFFC;
            client->ps.viewHeightTarget = 60;
            client->ps.pm_flags = v4;
            G_AddEvent(Entity, 6, 0);
        }
        else if (ConstString == scr_const.crouch)
        {
            v5 = __ROL4__(1, 1) & 3 | client->ps.pm_flags & 0xFFFFFFFC;
            client->ps.viewHeightTarget = 40;
            client->ps.pm_flags = v5;
            G_AddEvent(Entity, 7, 0);
        }
        else if (ConstString == scr_const.prone)
        {
            pm_flags = client->ps.pm_flags;
            if ((pm_flags & 1) == 0)
                client->ps.proneDirection = client->ps.viewangles[1];
            client->ps.viewHeightTarget = 11;
            client->ps.pm_flags = pm_flags & 0xFFFFFFFC | 1;
            G_AddEvent(Entity, 8, 0);
        }
    }
    else
    {
        Scr_Error("SetStance is only defined for players.");
    }
}

void __cdecl ScrCmd_ItemWeaponSetAmmo(scr_entref_t entref)
{
    gentity_s *Entity; // r28
    int Int; // r29
    int v3; // r27
    unsigned int v4; // r31
    const char *v5; // r3
    signed int v6; // r3
    WeaponDef *WeaponDef; // r30
    char *v8; // r11

    Entity = GetEntity(entref);
    if (Entity->s.eType != 2)
        Scr_Error("Entity is not an item.");
    if (*(itemType_t *)((char *)&bg_itemlist[0].giType + __ROL4__(Entity->s.index.item, 2)) != IT_WEAPON)
        Scr_Error("Item entity is not a weapon.");
    Int = Scr_GetInt(0);
    if (Int < 0)
        Scr_ParamError(0, "Ammo count must not be negative");
    v3 = Scr_GetInt(1);
    if (v3 < 0)
        Scr_ParamError(1u, "Ammo count must not be negative");
    v4 = 0;
    if (Scr_GetNumParam() > 2)
    {
        v4 = Scr_GetInt(2);
        if (v4 >= 2)
        {
            v5 = va("Value out of range.  Allowed values: 0 to %i", 2);
            Scr_ParamError(2u, v5);
        }
    }
    v6 = Entity->item[v4].index % 128;
    if (v6 > 0)
    {
        WeaponDef = BG_GetWeaponDef(v6);
        if (WeaponDef->iClipSize < 0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 3240, 0, "%s", "weapDef->iClipSize >= 0");
        if (WeaponDef->iClipSize < Int)
            Int = WeaponDef->iClipSize;
        v8 = (char *)Entity + 12 * v4;
        *((unsigned int *)v8 + 88) = v3;
        *((unsigned int *)v8 + 89) = Int;
    }
}

void __cdecl ScrCmd_MagicGrenade(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    actor_s *actor; // r31
    int v3; // r29
    const char *v4; // r3
    const char *v5; // r3
    float v6[4]; // [sp+58h] [-78h] BYREF
    float v7[3]; // [sp+68h] [-68h] BYREF
    float v10[4]; // [sp+78h] [-58h] BYREF
    float v11[4]; // [sp+88h] [-48h] BYREF
    float v12[14]; // [sp+98h] [-38h] BYREF

    Entity = GetEntity(entref);
    actor = Entity->actor;
    if (actor)
    {
        if (Scr_GetNumParam() != 2 && Scr_GetNumParam() != 3)
            Scr_Error("<actor> MagicGrenade <origin> <target position> [time to blow (seconds)].\n");
        Scr_GetVector(0, v7);
        v6[0] = v7[0];
        v6[1] = v7[1];
        v6[2] = v7[2];
        Scr_GetVector(1, v7);
        v12[0] = v7[0];
        v12[1] = v7[1];
        v12[2] = v7[2];
        Actor_Grenade_GetTossPositions(v6, v12, v10, actor->iGrenadeWeaponIndex);
        if (Actor_Grenade_CheckMinimumEnergyToss(actor, v6, v10, v11)
            || Actor_Grenade_CheckMaximumEnergyToss(actor, v6, v10, 0, v11)
            || Actor_Grenade_CheckMaximumEnergyToss(actor, v6, v10, 1, v11)
            || Actor_Grenade_CheckGrenadeHintToss(actor, v6, v10, v11))
        {
            if (Scr_GetNumParam() == 3)
                v3 = (int)(float)(Scr_GetFloat(2) * (float)1000.0);
            else
                v3 = 5000;
            if (!actor->iGrenadeWeaponIndex)
            {
                v4 = SL_ConvertToString(Entity->classname);
                v5 = va("Actor [%s] doesn't have a grenade weapon set.", v4);
                Scr_Error(v5);
            }
            G_FireGrenade(Entity, v6, v11, actor->iGrenadeWeaponIndex, 0, 1, v3);
        }
        else
        {
            Com_DPrintf(
                23,
                "MagicGrenade: None of the methods worked (probably distance or blocked)...need a good failsafe or remove this print?\n");
        }
    }
    else
    {
        Scr_Error("MagicGrenade only supports actors.\n");
    }
}

void __cdecl ScrCmd_MagicGrenadeManual(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    actor_s *actor; // r30
    int v3; // r29
    const char *v4; // r3
    const char *v5; // r3
    float v6[3]; // [sp+58h] [-58h] BYREF
    float v9[4]; // [sp+68h] [-48h] BYREF
    float v10[14]; // [sp+78h] [-38h] BYREF

    Entity = GetEntity(entref);
    actor = Entity->actor;
    if (actor)
    {
        if (Scr_GetNumParam() != 2 && Scr_GetNumParam() != 3)
            Scr_Error("<actor> MagicGrenadeManual <origin> <velocity> [time To Blow (seconds)].\n");
        Scr_GetVector(0, v6);
        v10[0] = v6[0];
        v10[1] = v6[1];
        v10[2] = v6[2];
        Scr_GetVector(1, v6);
        v9[0] = v6[0];
        v9[1] = v6[1];
        v9[2] = v6[2];
        if (Scr_GetNumParam() == 3)
            v3 = (int)(float)(Scr_GetFloat(2) * (float)1000.0);
        else
            v3 = 5000;
        if (!actor->iGrenadeWeaponIndex)
        {
            v4 = SL_ConvertToString(Entity->classname);
            v5 = va("Actor [%s] doesn't have a grenade weapon set.", v4);
            Scr_Error(v5);
        }
        G_FireGrenade(Entity, v10, v9, actor->iGrenadeWeaponIndex, 0, 1, v3);
    }
    else
    {
        Scr_Error("MagicGrenadeManual only supports actors.\n");
    }
}

// aislop
void __cdecl Scr_BulletSpread()
{
    float src[3], dst[3], dir[3], basis[3][3];
    float spread;
    float *weaponDef;
    float magnitude, invMag;
    float delta[3];

    // Get vectors and spread value from script arguments
    Scr_GetVector(0, src);        // Source point
    Scr_GetVector(1, dst);        // Destination point
    spread = Scr_GetFloat(2);     // Spread amount

    // Compute delta = dst - src
    delta[0] = dst[0] - src[0];
    delta[1] = dst[1] - src[1];
    delta[2] = dst[2] - src[2];

    // Normalize direction
    magnitude = sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
    if (magnitude == 0.0f) {
        Scr_Error("Zero-length vector in bullet spread calculation.");
        return;
    }

    invMag = 1.0f / magnitude;
    dir[0] = delta[0] * invMag;
    dir[1] = delta[1] * invMag;
    dir[2] = delta[2] * invMag;

    // Get weapon definition (placeholder index 0 used)
    weaponDef = (float *)BG_GetWeaponDef(0);

    // Build a left-handed basis for bullet spread
    Vec3Basis_LeftHanded(dir, basis[0], basis[1]);

    // Compute the final end position of the bullet
    Bullet_Endpos(level.time, spread, weaponDef, dst, 0, 8192.0f);

    // Return the final vector to the script
    Scr_AddVector(dst);
}


void __cdecl Scr_BulletTracer()
{
    gentity_s *v0; // r31
    bool v1; // r11
    float v2[4]; // [sp+50h] [-30h] BYREF
    float v3[4]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v3);
    Scr_GetVector(1u, v2);
    v0 = G_TempEntity(v3, 41);
    v0->s.lerp.u.turret.gunAngles[0] = v2[0];
    v0->s.lerp.u.turret.gunAngles[1] = v2[1];
    v0->s.lerp.u.turret.gunAngles[2] = v2[2];
    //v1 = Scr_GetNumParam() == 3 && (_cntlzw(Scr_GetInt(2)) & 0x20) == 0;
    v1 = Scr_GetNumParam() == 3 && Scr_GetInt(2);
    v0->s.eventParm = v1;
}

// aislop
void __cdecl Scr_MagicBullet()
{
    if (Scr_GetNumParam() != 3) {
        Scr_Error("MagicBullet weaponName sourceLoc destLoc.\n");
        return;
    }

    const char *weaponName = Scr_GetString(0);
    int weaponIndex = G_GetWeaponIndexForName(weaponName);
    if (!weaponIndex) {
        Scr_Error(va("MagicBullet called with unknown weapon name %s\n", weaponName));
        return;
    }

    float src[3], dst[3];
    Scr_GetVector(1, src);  // Source
    Scr_GetVector(2, dst);  // Destination

    // Prepare weapon parameters
    WeaponDef *weapDef = BG_GetWeaponDef(weaponIndex);
    weaponParms parms = { 0 };
    parms.weapDef = weapDef;

    // Set muzzle trace to source
    parms.muzzleTrace[0] = src[0];
    parms.muzzleTrace[1] = src[1];
    parms.muzzleTrace[2] = src[2];

    // Compute normalized direction vector (dst - src)
    float dx = dst[0] - src[0];
    float dy = dst[1] - src[1];
    float dz = dst[2] - src[2];
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    if (dist == 0.0f) {
        Scr_Error("MagicBullet(): Source and destination are the same.\n");
        return;
    }

    float invDist = 1.0f / dist;
    parms.forward[0] = dx * invDist;
    parms.forward[1] = dy * invDist;
    parms.forward[2] = dz * invDist;

    // Reject unsupported grenade-type weapons
    if (weapDef->weapType == WEAPTYPE_GRENADE) {
        Scr_Error("MagicBullet() does not work with grenade-type weapons.\n");
        return;
    }

    // Fire based on weapon type and class
    switch (weapDef->weapType) {
    case WEAPTYPE_BULLET:
        Bullet_Fire(&g_entities[ENTITYNUM_WORLD], 0.0f, &parms, NULL, level.time);
        break;

    case WEAPTYPE_PROJECTILE:
        switch (weapDef->weapClass) {
        case WEAPCLASS_GRENADE:
            Weapon_GrenadeLauncher_Fire(&g_entities[ENTITYNUM_WORLD], weaponIndex, 0, &parms);
            break;

        case WEAPCLASS_ROCKETLAUNCHER:
            Weapon_RocketLauncher_Fire(
                &g_entities[ENTITYNUM_WORLD],
                weaponIndex,
                0.0f,
                NULL,
                parms.forward,
                (gentity_s *)vec3_origin,
                0
            );
            break;

        default:
            Scr_Error("MagicBullet(): Unhandled projectile weapClass.\n");
            return;
        }
        break;

    default: {
        const char *typeName = BG_GetWeaponTypeName(weapDef->weapType);
        Scr_Error(va("MagicBullet(): Unhandled weapType \"%s\".\n", typeName));
        return;
    }
    }

    // Create temp entity for visual event tracking
    gentity_s *tempEnt = G_TempEntity(src, 26);
    tempEnt->s.weapon = weaponIndex;

    // Sanity check: weapon ID must fit in a byte
    if ((uint8_t)weaponIndex != weaponIndex) {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            3493,
            0,
            "%s",
            "tempEnt->s.weapon == weapon"
        );
    }

    tempEnt->s.eventParms[tempEnt->s.eventSequence & 3] = 0;
}


void __cdecl GScr_IsFiringTurret(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    int IsFiring; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    IsFiring = turret_IsFiring(v2);
    Scr_AddBool(IsFiring);
}

void __cdecl GScr_SetFriendlyChain(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    pathnode_t *Pathnode; // r3
    float *v3; // r31
    const char *v4; // r3

    Entity = GetEntity(entref);
    if (!Entity->client)
        Scr_Error("Entity must be a player");
    if (Scr_GetNumParam() != 1)
        Scr_Error("sentient SetFriendlyChain <node>, where <none> is on a friendly chain.\n");
    Pathnode = Scr_GetPathnode(0);
    v3 = (float *)Pathnode;
    if (Pathnode->constant.wChainId < 1)
    {
        Path_ConvertNodeToIndex(Pathnode);
        v4 = va(
            "Node %d (%.2f, %.2f, %.2f) is not a friendly chain node.\n",
            v3[5],
            v3[6],
            v3[7]
        );
        Scr_Error(v4);
    }
    Entity->sentient->pActualChainPos = (pathnode_t *)v3;
}

int __cdecl GScr_UpdateTagInternal(
    gentity_s *ent,
    unsigned int tagName,
    cached_tag_mat_t *cachedTag,
    int showScriptError)
{
    const char *v8; // r3
    const char *v9; // r3
    unsigned int v11; // r3
    const char *v12; // r31
    const char *v13; // r3
    const char *v14; // r3

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 3557, 0, "%s", "ent");
    if (ent->s.number == cachedTag->entnum && level.time == cachedTag->time && tagName == cachedTag->name)
        return 1;
    if (!SV_DObjExists(ent))
    {
        if (showScriptError)
        {
            v8 = SL_ConvertToString(ent->classname);
            v9 = va("entity has no model defined (classname '%s')", v8);
            Scr_ObjectError(v9);
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
        v11 = G_ModelName(ent->model);
        v12 = SL_ConvertToString(v11);
        v13 = SL_ConvertToString(tagName);
        v14 = va("tag '%s' does not exist in model '%s' (or any attached submodels)", v13, v12);
        Scr_ParamError(0, v14);
    }
    return 0;
}

void __cdecl GScr_GetTagOrigin(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    unsigned int ConstLowercaseString; // r3

    Entity = GetEntity(entref);
    ConstLowercaseString = Scr_GetConstLowercaseString(0);
    GScr_UpdateTagInternal(Entity, ConstLowercaseString, &level.cachedTagMat, 1);
    Scr_AddVector(level.cachedTagMat.tagMat[3]);
}

void __cdecl GScr_GetTagAngles(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    unsigned int ConstLowercaseString; // r3
    float v3[6]; // [sp+50h] [-30h] BYREF

    Entity = GetEntity(entref);
    ConstLowercaseString = Scr_GetConstLowercaseString(0);
    GScr_UpdateTagInternal(Entity, ConstLowercaseString, &level.cachedTagMat, 1);
    AxisToAngles((const mat3x3&)level.cachedTagMat.tagMat, v3);
    Scr_AddVector(v3);
}

void __cdecl ScrCmd_GetEye(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    float v5[4]; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->sentient)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("getEye must be called on an AI or player, not on a '%s'", v3);
        Scr_Error(v4);
    }
    Sentient_GetEyePosition(v2->sentient, v5);
    Scr_AddVector(v5);
}

void __cdecl ScrCmd_GetDebugEye(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    float v5[4]; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->sentient)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("getDebugEye must be called on an AI or player, not on a '%s'", v3);
        Scr_Error(v4);
    }
    Sentient_GetDebugEyePosition(v2->sentient, v5);
    Scr_AddVector(v5);
}

void __cdecl ScrCmd_UseBy(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    gentity_s *v2; // r3
    gentity_s *v3; // r30
    gclient_s *client; // r11
    void(__cdecl * use)(gentity_s *, gentity_s *, gentity_s *); // r11

    Entity = GetEntity(entref);
    v2 = Scr_GetEntity(0);
    v3 = v2;
    if (v2->active && (client = v2->client) != 0 && client->ps.viewlocked_entNum == Entity->s.number)
    {
        v2->active = (client->ps.eFlags & 0x300) == 0 ? 0 : 2;
    }
    else
    {
        Scr_AddEntity(v2);
        Scr_Notify(Entity, scr_const.trigger, 1u);
        use = entityHandlers[Entity->handler].use;
        if (use)
            use(Entity, v3, v3);
    }
}

void __cdecl ScrCmd_IsTouching(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *ent; // r31
    const gentity_s *v3; // r27
    gentity_s *v4; // r30
    gentity_s *v5; // r3
    const char *v6; // r3
    const char *v7; // r3
    const char *v8; // r3
    const char *v9; // r3
    const char *v10; // r3
    const char *v11; // r3
    int v12; // r3
    float v13[4]; // [sp+50h] [-50h] BYREF
    float v14[16]; // [sp+60h] [-40h] BYREF

    Entity = GetEntity(entref);
    ent = Entity;
    if (Entity->r.bmodel || (Entity->r.svFlags & 0x30) != 0)
    {
        v4 = Entity;
        v5 = Scr_GetEntity(0);
        ent = v5;
        if (v5->r.bmodel || (v5->r.svFlags & 0x30) != 0)
            Scr_Error("istouching cannot be called on 2 brush/cylinder entities");
        v3 = v4;
    }
    else
    {
        v3 = Scr_GetEntity(0);
    }

    iassert(ent->r.maxs[0] >= ent->r.mins[0]);
    iassert(ent->r.maxs[1] >= ent->r.mins[1]);
    iassert(ent->r.maxs[2] >= ent->r.mins[2]);

    v14[0] = ent->r.currentOrigin[0] + ent->r.mins[0];
    v14[1] = ent->r.currentOrigin[1] + ent->r.mins[1];
    v14[2] = ent->r.currentOrigin[2] + ent->r.mins[2];
    v13[0] = ent->r.currentOrigin[0] + ent->r.maxs[0];
    v13[1] = ent->r.currentOrigin[1] + ent->r.maxs[1];
    v13[2] = ent->r.currentOrigin[2] + ent->r.maxs[2];
    ExpandBoundsToWidth(v14, v13);
    v12 = SV_EntityContact(v14, v13, v3);
    Scr_AddInt(v12);
}

void __cdecl ParsePlaySoundCmd(scr_entref_t entref, int event, int notifyevent)
{
    gentity_s *Entity; // r30
    const char *String; // r31
    const char *v7; // r3
    unsigned __int16 v8; // r28
    int Int; // r29
    unsigned int ConstString; // r31
    unsigned int NumParam; // r3

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    if (!Com_FindSoundAlias(String))
    {
        v7 = va("unknown sound alias '%s'", String);
        Scr_ParamError(0, v7);
    }
    v8 = G_SoundAliasIndexTransient(String);
    Int = 0;
    ConstString = 0;
    Entity->r.svFlags &= ~1u;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            if (NumParam != 3)
            {
                Scr_Error("Sound Error");
                return;
            }
            Int = Scr_GetInt(2);
        }
        ConstString = Scr_GetConstString(1);
    }
    G_PlaySoundAliasWithNotify(Entity, v8, ConstString, Int, event, notifyevent);
}

void ScrCmd_SoundExists()
{
    const char *String; // r3
    snd_alias_list_t *SoundAlias; // r3

    String = Scr_GetString(0);
    SoundAlias = Com_TryFindSoundAlias(String);
    Scr_AddBool(SoundAlias != 0);
}

void __cdecl ScrCmd_PlaySound(scr_entref_t entref)
{
    ParsePlaySoundCmd(entref, 3, 42);
}

void __cdecl ScrCmd_PlaySoundAsMaster(scr_entref_t entref)
{
    ParsePlaySoundCmd(entref, 4, 43);
}

void __cdecl ScrCmd_PlayLoopSound(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    const char *String; // r31
    const char *v3; // r3
    unsigned __int16 v4; // r3
    unsigned __int8 svFlags; // r11

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    if (!Com_FindSoundAlias(String))
    {
        v3 = va("unknown sound alias '%s'", String);
        Scr_ParamError(0, v3);
    }
    v4 = G_SoundAliasIndexPermanent(String);
    svFlags = Entity->r.svFlags;
    Entity->s.loopSound = v4;
    Entity->r.svFlags = svFlags & 0xFE;
}

void __cdecl ScrCmd_StopLoopSound(scr_entref_t entref)
{
    GetEntity(entref)->s.loopSound = 0;
}

void __cdecl ScrCmd_StopSounds(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->r.svFlags &= ~1u;
    G_AddEvent(Entity, 5, 0);
}

void __cdecl ScrCmd_EqOn(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gclient_s *client; // r11

    Entity = GetEntity(entref);
    client = Entity->client;
    if (client)
        client->ps.eFlags &= ~0x200000u;
    else
        Entity->s.lerp.eFlags &= ~0x200000u;
}

void __cdecl ScrCmd_EqOff(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gclient_s *client; // r11

    Entity = GetEntity(entref);
    client = Entity->client;
    if (client)
        client->ps.eFlags |= 0x200000u;
    else
        Entity->s.lerp.eFlags |= 0x200000u;
}

void __cdecl ScrCmd_HasEq(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gclient_s *client; // r11
    int eFlags; // r11

    Entity = GetEntity(entref);
    client = Entity->client;
    if (client)
        eFlags = client->ps.eFlags;
    else
        eFlags = Entity->s.lerp.eFlags;
    Scr_AddInt(((unsigned int)~eFlags >> 21) & 1);
}

void __cdecl ScrCmd_IsWaitingOnSound(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Scr_AddInt(Entity->snd_wait.notifyString != 0);
}

void __cdecl ScrCmd_Delete(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (Entity->client)
        Scr_Error("Cannot delete a client entity");
    if (level.currentEntityThink == Entity->s.number)
        Scr_Error("Cannot delete entity during its think");
    Scr_Notify(Entity, scr_const.death, 0);
    G_FreeEntity(Entity);
}

void __cdecl ScrCmd_SetModel(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *String; // r30

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    G_SetModel(Entity, String);
    if (Entity->model && G_XModelBad(Entity->model) && Entity->actor)
    {
        Com_PrintWarning(23, "WARNING: actor model '%s' couldn't be found! switching to default actor model.\n", String);
        G_OverrideModel(Entity->model, "defaultactor");
    }
    G_DObjUpdate(Entity);
    SV_LinkEntity(Entity);
}

void __cdecl ScrCmd_SetShadowHint(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    unsigned int ConstString; // r3
    int v3; // r31

    Entity = GetEntity(entref);
    ConstString = Scr_GetConstString(0);
    v3 = 0;
    if (ConstString != scr_const.normal)
    {
        if (ConstString == scr_const.never)
        {
            v3 = 0x2000;
        }
        else if (ConstString == scr_const.high_priority)
        {
            v3 = 24576;
        }
        else if (ConstString == scr_const.low_priority)
        {
            v3 = 0x4000;
        }
        else if (ConstString == scr_const.always)
        {
            v3 = 0x8000;
        }
        else if (ConstString == scr_const.receiver)
        {
            v3 = 40960;
        }
        else
        {
            Scr_Error(
                "setshadowhint argument must be \"normal\", \"never\", \"high_priority\", \"low_priority\", \"always\", or \"receiver\".");
        }
    }
    Entity->s.lerp.eFlags = Entity->s.lerp.eFlags & 0xFFFF1FFF | v3;
}

// local variable allocation has failed, the output may be wrong!
void __cdecl ScrCmd_GetNormalHealth(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    //__int64 v3; // r11 OVERLAPPED
    gclient_s *client;
    __int64 v4; // r11
    __int64 v5; // [sp+50h] [-20h]

    Entity = GetEntity(entref);
    //HIDWORD(v3) = Entity->client;
    client = Entity->client;
    if (client)
    {
        if (Entity->health)
        {
            Scr_AddFloat(Entity->health / client->pers.maxHealth);
        }
        else
        {
            Scr_AddFloat(0.0);
        }
    }
    else
    {
        if (Entity->maxHealth)
        {
            Scr_AddFloat(Entity->health / Entity->maxHealth);
        }
        else
        {
            Scr_AddFloat(0.0);
            Com_PrintWarning(23, "WARNING: GetNormalHealth called on entity with 0 maxHealth.\n");
        }
    }
}

void __cdecl ScrCmd_SetNormalHealth(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    __int64 v2; // r11
    long double v3; // fp2
    double v4; // fp31
    const char *v5; // r6
    const char *v6; // r3
    gclient_s *client; // r11
    __int64 v8; // r11
    long double v9; // fp2
    const char *v10; // r3
    const char *v11; // r8
    long double v12; // fp2
    int v13; // r11
    const char *v14; // r5
    const char *v15; // r3
    int v16; // [sp+50h] [-30h]

    Entity = GetEntity(entref);
    *(double *)&v3 = Scr_GetFloat(0);
    v4 = *(double *)&v3;
    if (*(double *)&v3 <= 1.0)
    {
        if (*(double *)&v3 <= 0.0)
        {
            if (Entity->targetname)
                v5 = SL_ConvertToString(Entity->targetname);
            else
                v5 = "<not set>";
            v6 = va((const char *)HIDWORD(v4), LODWORD(v4), Entity->s.number, v5);
            Scr_Error(v6);
        }
    }
    else
    {
        v4 = 1.0;
    }
    client = Entity->client;
    if (client)
    {
        HIDWORD(v8) = 45900;
        LODWORD(v8) = client->pers.maxHealth;
        *(double *)&v3 = (float)((float)((float)v8 * (float)v4) + (float)0.5);
        v9 = floor(v3);
        v16 = (int)(float)*(double *)&v9;
        v10 = va("menu_show_notify \"%i\"", 0);
        SV_GameSendServerCommand(-1, v10);
    LABEL_17:
        v13 = v16;
        if (v16 < 1)
            v13 = 1;
        Entity->health = v13;
        return;
    }
    if (Entity->maxHealth > 0)
    {
        if (Entity->health <= 0)
        {
            if (Entity->targetname)
                v11 = SL_ConvertToString(Entity->targetname);
            else
                v11 = "<not set>";
            Com_DPrintf(
                23,
                "^2Cannot setNormalHealth on dead entities (health %i, max %i, ent %i, name %s)\n",
                Entity->health,
                Entity->maxHealth,
                Entity->s.number,
                v11);
        }
        LODWORD(v2) = Entity->maxHealth;
        *(double *)&v3 = (float)((float)((float)v2 * (float)v4) + (float)0.5);
        v12 = floor(v3);
        v16 = (int)(float)*(double *)&v12;
        goto LABEL_17;
    }
    if (Entity->targetname)
        v14 = SL_ConvertToString(Entity->targetname);
    else
        v14 = "<not set>";
    v15 = va(
        "entity's max health must be greater than 0 to call setNormalHealth (ent %i, name %s)\n",
        Entity->s.number,
        v14);
    Scr_Error(v15);
}

void __cdecl ScrCmd_DoDamage(scr_entref_t entref)
{
    gentity_s *attacker; // r27
    gentity_s *inflictor; // r26
    unsigned int NumParam; // r3
    gentity_s *ent; // r28
    double damage; // fp31
    const char *v10; // r3
    float *p_commandTime; // r11
    float *v12; // r11
    double v13; // fp12
    double v14; // fp13
    double v15; // fp0
    double v16; // fp0
    const float *dir; // r6
    double v18; // fp11
    bool v20; // mr_fpscr50
    double v22; // fp11
    int v23; // [sp+8h] [-E8h]
    hitLocation_t v24; // [sp+Ch] [-E4h]
    unsigned int v25; // [sp+10h] [-E0h]
    unsigned int v26; // [sp+14h] [-DCh]
    int v27; // [sp+18h] [-D8h]
    int v28; // [sp+1Ch] [-D4h]
    int v29; // [sp+20h] [-D0h]
    int v30; // [sp+24h] [-CCh]
    int v31; // [sp+28h] [-C8h]
    int v32; // [sp+2Ch] [-C4h]
    int v33; // [sp+30h] [-C0h]
    int v34; // [sp+34h] [-BCh]
    int v35; // [sp+38h] [-B8h]
    int v36; // [sp+3Ch] [-B4h]
    int v37; // [sp+40h] [-B0h]
    int v38; // [sp+44h] [-ACh]
    int v39; // [sp+48h] [-A8h]
    int v40; // [sp+4Ch] [-A4h]
    int v41; // [sp+50h] [-A0h]
    int v42; // [sp+58h] [-98h]
    int v43; // [sp+60h] [-90h]
    int v44; // [sp+68h] [-88h]
    float v48; // [sp+80h] [-70h] BYREF
    float v49; // [sp+84h] [-6Ch]
    float v50; // [sp+88h] [-68h]
    float source[3]; // v50, v51, v52
    float from[3]; // v45, v46, v47

    attacker = 0;
    inflictor = 0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 2)
    {
        if (NumParam != 3)
        {
            if (NumParam != 4)
            {
                Scr_Error("Usage: doDamage( <health>, <source position>, <attacker>, <inflictor> )\n");
                return;
            }
            inflictor = Scr_GetEntity(2);
        }
        attacker = Scr_GetEntity(2);
    }
    ent = GetEntity(entref);
    damage = Scr_GetFloat(0);
    Scr_GetVector(1, source);

    if (IS_NAN(source[0]) || IS_NAN(source[1]) || IS_NAN(source[2]))
    {
        Scr_Error(va("Source Damage vector is invalid : %f %f %f", source[0], source[1], source[2]));
    }
    if (ent->client)
    {
        iassert(!IS_NAN((ent->client->ps.origin)[0]) && !IS_NAN((ent->client->ps.origin)[1]) && !IS_NAN((ent->client->ps.origin)[2]));
        
        Vec3Sub(ent->client->ps.origin, source, from);
        //v12 = (float *)&ent->client->ps.commandTime;
        //v13 = (float)(v12[7] - source);
        //v48 = v12[7] - source;
        //v14 = (float)(v12[8] - v52);
        //v49 = v12[8] - v52;
        //v15 = v12[9];
    }
    else
    {
        iassert(!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2]));
        
        Vec3Sub(ent->r.currentOrigin, source, from);
        //v13 = (float)(ent->r.currentOrigin[0] - source);
        //v48 = ent->r.currentOrigin[0] - source;
        //v14 = (float)(ent->r.currentOrigin[1] - v52);
        //v49 = ent->r.currentOrigin[1] - v52;
        //v15 = ent->r.currentOrigin[2];
    }
    //v45 = v13;
    //v16 = (float)((float)v15 - v53);
    //v50 = v16;

    iassert(!IS_NAN((from)[0]) && !IS_NAN((from)[1]) && !IS_NAN((from)[2]));

    //dir = 0;
    //v18 = sqrtf((float)((float)((float)v13 * (float)v13)
    //    + (float)((float)((float)v16 * (float)v16) + (float)((float)v14 * (float)v14))));
    //_FP9 = -v18;
    //v20 = v18 == 0.0;
    //__asm { fsel      f11, f9, f10, f11 }
    //v22 = (float)((float)1.0 / (float)_FP11);
    //v48 = (float)v22 * (float)v13;
    //v49 = (float)v22 * (float)v14;
    //v50 = (float)v22 * (float)v16;
    //if (!v20)
    //    dir = &v48;

    if (Vec3Normalize(from) == 0.0f)
    {
        dir = NULL;
    }
    else
    {
        dir = from;
    }

    G_Damage(
        ent,
        inflictor,
        attacker,
        dir,
        source,
        (int)damage,
        0, //dflags
        0, // mod
        0xFFFFFFFF, // weapon
        HITLOC_HEAD, // KISAKTODO (weird?)
        0,
        0);
}

void __cdecl ScrCmd_Show(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags &= ~0x20u;
}

void __cdecl ScrCmd_Hide(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags |= 0x20u;
}

void __cdecl ScrCmd_LaserOn(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags |= 0x4000u;
}

void __cdecl ScrCmd_LaserOff(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Entity->s.lerp.eFlags &= ~0x4000u;
}

void __cdecl ScrCmd_SetContents(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    int Int; // r30

    Entity = GetEntity(entref);
    Int = Scr_GetInt(0);
    Scr_AddInt(Entity->r.contents);
    Entity->r.contents = Int;
    SV_LinkEntity(Entity);
}

void __cdecl Scr_SetStableMissile(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    int Int; // r3
    int eType; // r11
    int v4; // r30
    gentityFlags_t flags; // r11
    gentityFlags_t v6; // r11

    Entity = GetEntity(entref);
    Int = Scr_GetInt(0);
    eType = Entity->s.eType;
    v4 = Int;
    if (eType != 14 && eType != 11 && eType != 1)
        Scr_Error("Type should be a sentient or a vehicle");
    flags = Entity->flags;
    if (v4)
        v6 = flags | FL_STABLE_MISSILES;
    else
        v6 = flags & ~(FL_STABLE_MISSILES);
    Entity->flags = v6;
}

void __cdecl GScr_DisconnectPaths(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *v2; // r3
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (!Path_IsDynamicBlockingEntity(Entity))
    {
        if (Entity->classname == scr_const.script_brushmodel)
            Scr_Error("script_brushmodel must have DYNAMICPATH set to disconnect paths");
        v2 = SL_ConvertToString(Entity->classname);
        v3 = va("entity of type '%s' cannot disconnect paths.\n \n ", v2);
        Scr_Error(v3);
    }
    Path_DisconnectPathsForEntity(Entity);
}

void __cdecl GScr_ConnectPaths(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *v2; // r3
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (!Path_IsDynamicBlockingEntity(Entity))
    {
        if (Entity->classname == scr_const.script_brushmodel)
            Scr_Error("script_brushmodel must have DYNAMICPATH set to connect paths");
        v2 = SL_ConvertToString(Entity->classname);
        v3 = va("entity of type '%s' cannot connect paths \n\n", v2);
        Scr_Error(v3);
    }
    Path_ConnectPathsForEntity(Entity);
}

void __cdecl GScr_StartFiring(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    TurretInfo *pTurretInfo; // r31

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    pTurretInfo = v2->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4502, 0, "%s", "pTurretInfo");
    pTurretInfo->flags |= 4u;
}

void __cdecl GScr_StopFiring(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    TurretInfo *pTurretInfo; // r31

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    pTurretInfo = v2->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4528, 0, "%s", "pTurretInfo");
    pTurretInfo->flags &= ~4u;
}

void __cdecl GScr_ShootTurret(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    turret_shoot(v2);
}

void __cdecl GScr_SetMode(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    TurretInfo *pTurretInfo; // r31
    unsigned int ConstString; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    pTurretInfo = v2->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4580, 0, "%s", "pTurretInfo");
    ConstString = Scr_GetConstString(0);
    if (ConstString == scr_const.auto_ai)
    {
        pTurretInfo->flags |= 3u;
    }
    else if (ConstString == scr_const.manual)
    {
        pTurretInfo->flags &= 0xFFFFFFFC;
    }
    else if (ConstString == scr_const.manual_ai)
    {
        pTurretInfo->flags = pTurretInfo->flags & 0xFFFFFFFC | 1;
    }
    else if (ConstString == scr_const.auto_nonai)
    {
        pTurretInfo->flags = __ROL4__(1, 1) & 3 | pTurretInfo->flags & 0xFFFFFFFC;
    }
    else
    {
        Scr_Error("Error setting the mode of a turret.\n");
    }
}

void __cdecl GScr_GetTurretOwner(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    EntHandle *p_ownerNum; // r31
    gentity_s *v6; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (v2->active)
    {
        p_ownerNum = &v2->r.ownerNum;
        if (p_ownerNum->isDefined())
            v6 = p_ownerNum->ent();
        else
            v6 = &g_entities[ENTITYNUM_NONE];
        Scr_AddEntity(v6);
    }
}

void __cdecl GScr_SetTargetEntity(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    gentity_s *v5; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4650, 0, "%s", "ent->pTurretInfo");
    v5 = Scr_GetEntity(0);
    v2->pTurretInfo->manualTarget.setEnt(v5);
}

void __cdecl GScr_SetAiSpread(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4675, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->aiSpread = Scr_GetFloat(0);
}

void __cdecl GScr_SetPlayerSpread(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4700, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->playerSpread = Scr_GetFloat(0);
}

void __cdecl GScr_SetConvergenceTime(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r30
    const char *v3; // r3
    const char *v4; // r3
    int v5; // r31
    const char *String; // r11
    const char *v7; // r9
    const char *v8; // r10
    int v9; // r7
    const char *v10; // r10
    int v11; // r8

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4728, 0, "%s", "ent->pTurretInfo");
    v5 = 1;
    if ((int)Scr_GetNumParam() > 1)
    {
        String = Scr_GetString(1);
        v7 = "yaw";
        v8 = String;
        do
        {
            v9 = *(unsigned __int8 *)v8 - *(unsigned __int8 *)v7;
            if (!*v8)
                break;
            ++v8;
            ++v7;
        } while (!v9);
        if (v9)
        {
            v10 = "pitch";
            do
            {
                v11 = *(unsigned __int8 *)String - *(unsigned __int8 *)v10;
                if (!*String)
                    break;
                ++String;
                ++v10;
            } while (!v11);
            if (v11)
                Scr_Error("Convergence type should be either 'pitch' or 'yaw'");
            else
                v5 = 0;
        }
        else
        {
            v5 = 1;
        }
    }
    v2->pTurretInfo->convergenceTime[v5] = (int)(float)(Scr_GetFloat(0) * (float)1000.0);
}

void __cdecl GScr_SetSuppressionTime(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4768, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->suppressTime = (int)(float)(Scr_GetFloat(0) * (float)1000.0);
}

void __cdecl GScr_ClearTargetEntity(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4792, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->manualTarget.setEnt(NULL);
}

void __cdecl GScr_SetTurretTeam(scr_entref_t entref)
{
    const char *String; // r31
    gentity_s *Entity; // r3
    gentity_s *v4; // r30
    const char *v5; // r3
    const char *v6; // r3
    const char *v7; // r3

    String = Scr_GetString(0);
    Entity = GetEntity(entref);
    v4 = Entity;
    if (!Entity->pTurretInfo)
    {
        v5 = SL_ConvertToString(Entity->classname);
        v6 = va("entity type '%s' is not a turret", v5);
        Scr_Error(v6);
    }
    if (I_stricmp(String, "axis"))
    {
        if (I_stricmp(String, "allies"))
        {
            v7 = va("unknown team '%s', should be 'axis' or 'allies'\n", String);
            Scr_Error(v7);
        }
        else
        {
            v4->pTurretInfo->eTeam = TEAM_ALLIES;
        }
    }
    else
    {
        v4->pTurretInfo->eTeam = TEAM_AXIS;
    }
}

void __cdecl GScr_SetTurretIgnoreGoals(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3
    int Int; // r3
    TurretInfo *pTurretInfo; // r11
    int flags; // r10
    unsigned int v8; // r10

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4846, 0, "%s", "ent->pTurretInfo");
    Int = Scr_GetInt(0);
    pTurretInfo = v2->pTurretInfo;
    flags = pTurretInfo->flags;
    if (Int)
        v8 = flags | 0x2000;
    else
        v8 = flags & 0xFFFFDFFF;
    pTurretInfo->flags = v8;
}

void __cdecl GScr_MakeTurretUsable(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4875, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->flags |= 0x1000u;
}

void __cdecl GScr_MakeTurretUnusable(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    if (!v2->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 4898, 0, "%s", "ent->pTurretInfo");
    v2->pTurretInfo->flags &= ~0x1000u;
}

void __cdecl GScr_SetTurretAccuracy(scr_entref_t entref)
{
    Com_PrintWarning(23, "WARNING: Turret Accuracy no longer has any effect\n");
}

void __cdecl GScr_GetTurretTarget(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *ent; // r31
    TurretInfo *pTurretInfo; // r11

    Entity = GetEntity(entref);
    ent = Entity;

    if (!Entity->pTurretInfo)
    {
        Scr_Error(va("entity type '%s' is not a turret", SL_ConvertToString(Entity->classname)));
    }

    iassert(ent->pTurretInfo);

    if (ent->pTurretInfo->target.isDefined())
    {
        pTurretInfo = ent->pTurretInfo;
        if ((pTurretInfo->flags & 0x40) != 0)
        {
            Scr_AddEntity(pTurretInfo->target.ent());
        }
    }
}

void __cdecl GScr_SetCursorHint(scr_entref_t entref)
{
    gentity_s *ent; // r27
    const char *String; // r3
    int classname; // r11
    const char *v4; // r26
    int v5; // r30
    const char **v6; // r31
    int v7; // r11
    const char **v8; // r31
    const char *v9; // r3

    ent = GetEntity(entref);
    iassert(ent->s.eType != ET_MISSILE);
    String = Scr_GetString(0);
    classname = ent->classname;
    v4 = String;
    if ((classname == scr_const.trigger_use || classname == scr_const.trigger_use_touch)
        && !I_stricmp(String, "HINT_INHERIT"))
    {
        *(unsigned int *)ent->s.un2 = -1;
    }
    else
    {
        v5 = 1;
        v6 = &hintStrings[1];
        do
        {
            if (!I_stricmp(v4, *v6))
            {
                *(unsigned int *)ent->s.un2 = v5;
                return;
            }
            ++v6;
            ++v5;
        } while ((uintptr_t)v6 < (uintptr_t)&hintStrings[4]);
        Com_Printf(23, "List of valid hint type strings\n");
        v7 = ent->classname;
        if (v7 == scr_const.trigger_use || v7 == scr_const.trigger_use_touch)
            Com_Printf(23, "HINT_INHERIT (for trigger_use or trigger_use_touch entities only)\n");
        v8 = &hintStrings[1];
        do
        {
            if (!*v8)
                break;
            Com_Printf(23, "%s\n", *v8++);
        } while ((int)v8 < (int)&functions[9].actionFunc);
        v9 = va("%s is not a valid hint type. See above for list of valid hint types\n", v4);
        Scr_Error(v9);
    }
}

int __cdecl G_GetHintStringIndex(int *piIndex, const char *pszString)
{
    int v4; // r31
    char *v5; // r9
    const char *v6; // r10
    int v7; // r8
    int result; // r3
    char v9[1056]; // [sp+50h] [-420h] BYREF

    v4 = 0;
    while (1)
    {
        SV_GetConfigstring(v4 + 59, v9, 1024);
        if (!v9[0])
            break;
        v5 = v9;
        v6 = pszString;
        do
        {
            v7 = *(unsigned __int8 *)v6 - (unsigned __int8)*v5;
            if (!*v6)
                break;
            ++v6;
            ++v5;
        } while (!v7);
        if (!v7)
            goto LABEL_10;
        if (++v4 >= 32)
        {
            result = 0;
            *piIndex = -1;
            return result;
        }
    }
    SV_SetConfigstring(v4 + 59, pszString);
LABEL_10:
    result = 1;
    *piIndex = v4;
    return result;
}

void __cdecl GScr_SetHintString(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r29
    int classname; // r11
    const char *String; // r3
    int v5; // r11
    unsigned int NumParam; // r3
    const char *v7; // r3
    int v8; // r11
    int v9[4]; // [sp+50h] [-440h] BYREF
    char v10[1072]; // [sp+60h] [-430h] BYREF

    Entity = GetEntity(entref);
    v2 = Entity;
    classname = Entity->classname;
    if (classname != scr_const.trigger_use && classname != scr_const.trigger_use_touch && !Entity->actor)
        Scr_Error("The setHintString command only works on trigger_use entities and actors.\n");
    if (Scr_GetType(0) != 2 || (String = Scr_GetString(0), I_stricmp(String, "")))
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(0, NumParam - 1, "Hint String", v10, 0x400u);
        if (!G_GetHintStringIndex(v9, v10))
        {
            v7 = va("Too many different hintstring values. Max allowed is %i different strings", 32);
            Scr_Error(v7);
        }
        v8 = v2->classname;
        if (v8 == scr_const.trigger_use || v8 == scr_const.trigger_use_touch)
        {
            v2->s.un1.scale = v9[0];
        }
        else
        {
            if (!v2->actor)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 5070, 0, "%s", "ent->actor");
            v2->actor->iUseHintString = v9[0];
        }
    }
    else
    {
        v5 = v2->classname;
        if (v5 == scr_const.trigger_use || v5 == scr_const.trigger_use_touch)
        {
            v2->s.un1.scale = -1;
        }
        else
        {
            if (!v2->actor)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 5051, 0, "%s", "ent->actor");
            v2->actor->iUseHintString = -1;
        }
    }
}

void __cdecl GScr_UseTriggerRequireLookAt(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (Entity->classname != scr_const.trigger_use)
        Scr_Error("The UseTriggerRequireLookAt command only works on trigger_use entities.\n");
    Entity->trigger.requireLookAt = 1;
}

void __cdecl G_InitObjectives()
{
    int i; // r31

    for (i = 0; i < 16; ++i)
        SV_SetConfigstring(i + 11, 0);
}

int __cdecl PrintObjectiveUpdate(unsigned int state, const char *objectiveDesc)
{
    const char *v2; // r3

    if (state == scr_const.active || state == scr_const.current)
    {
        v2 = va("obj_update \"%s\"", objectiveDesc);
    }
    else if (state == scr_const.done)
    {
        v2 = va("obj_complete \"GAME_OBJECTIVECOMPLETED\"", objectiveDesc);
    }
    else
    {
        if (state != scr_const.failed)
            return 0;
        v2 = va("obj_failed \"GAME_OBJECTIVEFAILED\"", objectiveDesc);
    }
    SV_GameSendServerCommand(-1, v2);
    return 1;
}

int __cdecl ObjectiveStateIndexFromString(int *piStateIndex, unsigned int stateString)
{
    int result; // r3

    if (stateString == scr_const.empty)
    {
        result = 1;
        *piStateIndex = 0;
    }
    else if (stateString == scr_const.active)
    {
        result = 1;
        *piStateIndex = 1;
    }
    else if (stateString == scr_const.invisible)
    {
        result = 1;
        *piStateIndex = 2;
    }
    else if (stateString == scr_const.done)
    {
        result = 1;
        *piStateIndex = 3;
    }
    else if (stateString == scr_const.current)
    {
        result = 1;
        *piStateIndex = 4;
    }
    else if (stateString == scr_const.failed)
    {
        result = 1;
        *piStateIndex = 5;
    }
    else
    {
        result = 0;
        *piStateIndex = 0;
    }
    return result;
}

void __cdecl SetObjectiveIconIntoConfigString(char *objConfigString, unsigned int paramNum)
{
    const char *String; // r31
    int v4; // r30
    char v5; // r11
    const char *v6; // r3
    const char *v7; // r3
    int v8; // r3
    const char *v9; // r3

    String = Scr_GetString(paramNum);
    v4 = 0;
    if (*String)
    {
        do
        {
            v5 = String[v4];
            if (v5 <= 31 || v5 >= 127)
            {
                v6 = va(
                    "Illegal character '%c'(ascii %i) in objective icon name: %s\n",
                    String[v4],
                    (unsigned __int8)String[v4],
                    String);
                Scr_ParamError(3u, v6);
            }
            ++v4;
        } while (String[v4]);
        if (v4 >= 64)
        {
            v7 = va("Objective icon name is too long (> %i): %s\n", 63, String);
            Scr_ParamError(3u, v7);
        }
    }
    v8 = G_MaterialIndex(String);
    v9 = va("%i", v8);
    Info_SetValueForKey(objConfigString, "icon", v9);
}

int Scr_Objective_Add()
{
    int NumParam; // r27
    unsigned int Int; // r31
    const char *v2; // r3
    int v3; // r26
    unsigned int ConstString; // r25
    const char *String; // r3
    const char *v6; // r3
    const char *v7; // r3
    const char *IString; // r3
    const char *v9; // r31
    const char *v10; // r11
    int v12; // r29
    int i; // r30
    const char *v14; // r3
    const char *v15; // r3
    const char *v16; // r11
    unsigned int v18; // r30
    const char *v19; // r3
    const char *v20; // r3
    int j; // r30
    unsigned int v22; // r5
    const char *v23; // r3
    const char *v24; // r3
    const char *v25; // r3
    int v27; // [sp+50h] [-860h] BYREF
    float v28[6]; // [sp+58h] [-858h] BYREF
    char v29[1024]; // [sp+70h] [-840h] BYREF
    char v30[1088]; // [sp+470h] [-440h] BYREF

    NumParam = Scr_GetNumParam();
    if (NumParam < 2)
        Scr_Error(
            "objective_add needs at least the first two parameters out of its parameter list of: index state [string] [position]\n");
    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v2 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v2);
    }
    v3 = Int + 11;
    SV_GetConfigstring(Int + 11, v29, 1024);
    ConstString = Scr_GetConstString(1);
    if (!ObjectiveStateIndexFromString(&v27, ConstString))
    {
        String = Scr_GetString(1);
        v6 = va(
            "Illegal objective state \"%s\". Valid states are \"empty\", \"active\", \"invisible\", \"done\", \"current\", \"failed\"\n",
            String);
        Scr_Error(v6);
    }
    v7 = va("%i", v27);
    Info_SetValueForKey(v29, "state", v7);
    if (NumParam <= 2)
    {
        v9 = "GAME_OBJECTIVESUPDATED";
    }
    else
    {
        if (Scr_GetType(2) == 3)
        {
            IString = Scr_GetIString(2);
            v9 = IString;
            v10 = IString;
            while (*(unsigned __int8 *)v10++)
                ;
            v12 = v10 - IString - 1;
            if (v12 > 1)
            {
                for (i = 0; i < v12; ++i)
                {
                    if (!isalnum(v9[i]) && v9[i] != 95)
                    {
                        v14 = va(
                            "Illegal localized string reference: %s (must contain only alpha-numeric characters and underscores",
                            v9);
                        Scr_ParamError(2u, v14);
                    }
                }
            }
        }
        else
        {
            v15 = Scr_GetString(2);
            v9 = v15;
            v16 = v15;
            while (*(unsigned __int8 *)v16++)
                ;
            v12 = v16 - v15 - 1;
            if (!(unsigned __int8)Scr_ValidateNonLocalizedStringRef(2u, v15, v12, "Objective String"))
            {
                v18 = Scr_NonLocalizedStringErrorPrefix(2u, v12, "Objective String", 0, 0x400u, v30);
                memcpy(&v30[v18], v9, v12);
                v9 = v30;
                v30[v18 + v12] = 0;
            }
        }
        if (v12 >= 1024)
        {
            v19 = va("Objective strings is too long (> %i): %s\n", 1023, v9);
            Scr_Error(v19);
        }
        if (!Info_Validate(v9) || strchr(v9, 92))
        {
            v20 = va("Objective strings can not have a \", a ;, or a \\ in them. Illegal objective string: %s\n", v9);
            Scr_Error(v20);
        }
        for (j = 0; j < v12; ++j)
        {
            v22 = (unsigned __int8)v9[j];
            if (v9[j] && v22 <= 0x1F || v22 >= 0x7F)
            {
                v23 = va("Illegal character '%c'(ascii %i) in objective string: %s\n", v9[j], v22, v9);
                Scr_Error(v23);
            }
        }
        Info_SetValueForKey(v29, "str", v9);
        if (NumParam > 3)
        {
            Scr_GetVector(3u, v28);
            v27 = (int)v28[0];
            v24 = va("%i %i %i", v27, (int)v28[1], (int)v28[2]);
            Info_SetValueForKey(v29, "org0", v24);
            if (NumParam > 4)
                SetObjectiveIconIntoConfigString(v29, 4u);
        }
    }
    SV_SetConfigstring(v3, v29);
    v25 = va("menu_show_notify \"%i\"", 2);
    SV_GameSendServerCommand(-1, v25);
    return PrintObjectiveUpdate(ConstString, v9);
}

void Scr_Objective_Delete()
{
    unsigned int Int; // r31
    const char *v1; // r3
    int v2; // r31
    char v3[16]; // [sp+50h] [-20h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v1);
    }
    v2 = Int + 11;
    SV_GetConfigstring(v2, v3, 4);
    v3[0] = 0;
    SV_SetConfigstring(v2, v3);
}

int Scr_Objective_State()
{
    unsigned int Int; // r31
    const char *v1; // r3
    int v2; // r31
    unsigned int ConstString; // r30
    const char *String; // r3
    const char *v5; // r3
    const char *v6; // r3
    int v8[4]; // [sp+50h] [-430h] BYREF
    char v9[1032]; // [sp+60h] [-420h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v1);
    }
    v2 = Int + 11;
    SV_GetConfigstring(v2, v9, 1024);
    ConstString = Scr_GetConstString(1);
    if (!ObjectiveStateIndexFromString(v8, ConstString))
    {
        String = Scr_GetString(1);
        v5 = va(
            "Illegal objective state \"%s\". Valid states are \"empty\", \"active\", \"invisible\", \"done\", \"current\", \"failed\"\n",
            String);
        Scr_Error(v5);
    }
    v6 = va("%i", v8[0]);
    Info_SetValueForKey(v9, "state", v6);
    SV_SetConfigstring(v2, v9);
    return PrintObjectiveUpdate(ConstString, "GAME_OBJECTIVESUPDATED");
}

void __cdecl Scr_Objective_String_Internal(int makeUpdateMessage)
{
    unsigned int Int; // r31
    const char *v3; // r3
    int v4; // r28
    unsigned int NumParam; // r3
    char *v6; // r11
    int v8; // r29
    const char *v9; // r3
    const char *v10; // r3
    int i; // r31
    unsigned int v12; // r11
    const char *v13; // r3
    const char *v14; // r3
    int v15; // r3
    unsigned __int16 empty; // r11
    char v17[1024]; // [sp+50h] [-830h] BYREF
    char v18[1072]; // [sp+450h] [-430h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v3 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v3);
    }
    v4 = Int + 11;
    SV_GetConfigstring(Int + 11, v18, 1024);
    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(1, NumParam - 1, "Objective String", v17, 0x400u);
    v6 = v17;
    while (*v6++)
        ;
    v8 = v6 - v17 - 1;
    if (v8 >= 1024)
    {
        v9 = va("Objective strings is too long (> %i): %s\n", 1023, v17);
        Scr_Error(v9);
    }
    if (!Info_Validate(v17) || strchr(v17, 92))
    {
        v10 = va("Objective strings can not have a \", a ;, or a \\ in them. Illegal objective string: %s\n", v17);
        Scr_Error(v10);
    }
    for (i = 0; i < v8; ++i)
    {
        v12 = (unsigned __int8)v17[i];
        if (v17[i] && v12 <= 0x1F && v12 != 20 && v12 != 21 || v12 >= 0x7F)
        {
            v13 = va("Illegal character '%c'(ascii %i) in objective string: %s\n", (char)v12, (unsigned __int8)v17[i], v17);
            Scr_Error(v13);
        }
    }
    Info_SetValueForKey(v18, "str", v17);
    SV_SetConfigstring(v4, v18);
    v14 = Info_ValueForKey(v18, "state");
    if (*v14)
        v15 = atol(v14);
    else
        v15 = 0;
    switch (v15)
    {
    case 0:
        empty = scr_const.empty;
        goto LABEL_28;
    case 1:
        empty = scr_const.active;
        goto LABEL_28;
    case 2:
        empty = scr_const.invisible;
        goto LABEL_28;
    case 3:
        empty = scr_const.done;
        goto LABEL_28;
    case 4:
        empty = scr_const.current;
        goto LABEL_28;
    case 5:
        empty = scr_const.failed;
    LABEL_28:
        if (makeUpdateMessage)
        {
            if (empty != scr_const.done)
                PrintObjectiveUpdate(empty, v17);
        }
        break;
    default:
        return;
    }
}

void Scr_Objective_String()
{
    Scr_Objective_String_Internal(1);
}

void Scr_Objective_String_NoMessage()
{
    Scr_Objective_String_Internal(0);
}

void Scr_Objective_Icon()
{
    unsigned int Int; // r31
    const char *v1; // r3
    int v2; // r31
    char v3[1024]; // [sp+50h] [-410h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_ParamError(0, v1);
    }
    v2 = Int + 11;
    SV_GetConfigstring(v2, v3, 1024);
    SetObjectiveIconIntoConfigString(v3, 1u);
    SV_SetConfigstring(v2, v3);
}

void Scr_Objective_Position()
{
    unsigned int Int; // r31
    const char *v1; // r3
    int v2; // r31
    const char *v3; // r3
    int v4; // [sp+50h] [-430h]
    float v5[6]; // [sp+58h] [-428h] BYREF
    char v6[1024]; // [sp+70h] [-410h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v1);
    }
    v2 = Int + 11;
    SV_GetConfigstring(v2, v6, 1024);
    Scr_GetVector(1u, v5);
    v4 = (int)v5[0];
    v3 = va("%i %i %i", v4, (int)v5[1], (int)v5[2]);
    Info_SetValueForKey(v6, "org0", v3);
    SV_SetConfigstring(v2, v6);
}

void Scr_Objective_AdditionalPosition()
{
    unsigned int Int; // r31
    const char *v1; // r3
    unsigned int v2; // r30
    const char *v3; // r3
    int v4; // r31
    const char *v5; // r3
    int v6; // [sp+50h] [-440h]
    float v7[4]; // [sp+58h] [-438h] BYREF
    char v8[8]; // [sp+68h] [-428h] BYREF
    char v9[1032]; // [sp+70h] [-420h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v1);
    }
    v2 = Scr_GetInt(1);
    if (v2 >= 8)
    {
        v3 = va("index %i is an illegal position index. Valid indexes are 0 to %i\n", v2, 7);
        Scr_Error(v3);
    }
    v4 = Int + 11;
    SV_GetConfigstring(v4, v9, 1024);
    Scr_GetVector(2u, v7);
    sprintf(v8, "org%d", v2);
    v6 = (int)v7[0];
    v5 = va("%i %i %i", v6, (int)v7[1], (int)v7[2]);
    Info_SetValueForKey(v9, v8, v5);
    SV_SetConfigstring(v4, v9);
}

int Scr_Objective_Current()
{
    int NumParam; // r3
    signed int v1; // r29
    _QWORD *v2; // r11
    __int64 v3; // r9
    int v4; // ctr
    signed int v5; // r31
    unsigned int Int; // r3
    unsigned int v7; // r30
    const char *v8; // r3
    int v9; // r31
    unsigned int *v10; // r27
    int v11; // r28
    const char *v12; // r3
    int v13; // r3
    int v14; // r4
    const char *v15; // r3
    unsigned int v17[16]; // [sp+50h] [-470h] BYREF
    char v18[1072]; // [sp+90h] [-430h] BYREF

    NumParam = Scr_GetNumParam();
    v1 = NumParam;
    v2 = (_QWORD*)v17;
    LODWORD(v3) = 0;
    v4 = 8;
    do
    {
        *v2++ = v3;
        --v4;
    } while (v4);
    v5 = 0;
    if (NumParam > 0)
    {
        do
        {
            Int = Scr_GetInt(v5);
            v7 = Int;
            if (Int >= 0x10)
            {
                v8 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
                Scr_Error(v8);
            }
            ++v5;
            v17[v7] = 1;
        } while (v5 < v1);
    }
    v9 = 11;
    v10 = v17;
    v11 = 16;
    do
    {
        SV_GetConfigstring(v9, v18, 1024);
        v12 = Info_ValueForKey(v18, "state");
        if (*v12)
            v13 = atol(v12);
        else
            v13 = 0;
        if (*v10)
        {
            if (v13 == 4)
                goto LABEL_17;
            v14 = 4;
        }
        else
        {
            if (v13 != 4)
                goto LABEL_17;
            v14 = 1;
        }
        v15 = va("%i", v14);
        Info_SetValueForKey(v18, "state", v15);
        SV_SetConfigstring(v9, v18);
    LABEL_17:
        --v11;
        ++v10;
        ++v9;
    } while (v11);
    return PrintObjectiveUpdate(scr_const.current, "GAME_OBJECTIVESUPDATED");
}

int Scr_Objective_AdditionalCurrent()
{
    int NumParam; // r3
    signed int v1; // r29
    _QWORD *v2; // r11
    __int64 v3; // r9
    int v4; // ctr
    signed int v5; // r31
    unsigned int Int; // r3
    unsigned int v7; // r30
    const char *v8; // r3
    int v9; // r31
    unsigned int *v10; // r28
    int v11; // r29
    const char *v12; // r3
    int v13; // r3
    const char *v14; // r3
    unsigned int v16[16]; // [sp+50h] [-470h] BYREF
    char v17[1072]; // [sp+90h] [-430h] BYREF

    NumParam = Scr_GetNumParam();
    v1 = NumParam;
    v2 = (_QWORD*)v16;
    LODWORD(v3) = 0;
    v4 = 8;
    do
    {
        *v2++ = v3;
        --v4;
    } while (v4);
    v5 = 0;
    if (NumParam > 0)
    {
        do
        {
            Int = Scr_GetInt(v5);
            v7 = Int;
            if (Int >= 0x10)
            {
                v8 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
                Scr_Error(v8);
            }
            ++v5;
            v16[v7] = 1;
        } while (v5 < v1);
    }
    v9 = 11;
    v10 = v16;
    v11 = 16;
    do
    {
        SV_GetConfigstring(v9, v17, 1024);
        v12 = Info_ValueForKey(v17, "state");
        if (*v12)
            v13 = atol(v12);
        else
            v13 = 0;
        if (*v10 && v13 != 4)
        {
            v14 = va("%i", 4);
            Info_SetValueForKey(v17, "state", v14);
            SV_SetConfigstring(v9, v17);
        }
        --v11;
        ++v10;
        ++v9;
    } while (v11);
    return PrintObjectiveUpdate(scr_const.current, "GAME_OBJECTIVESUPDATED");
}

void Scr_Objective_Ring()
{
    unsigned int Int; // r31
    const char *v1; // r3
    int v2; // r31
    const char *v3; // r3
    int v4; // r3
    const char *v5; // r3
    char v6[1032]; // [sp+50h] [-420h] BYREF

    Int = Scr_GetInt(0);
    if (Int >= 0x10)
    {
        v1 = va("index %i is an illegal objective index. Valid indexes are 0 to %i\n", Int, 15);
        Scr_Error(v1);
    }
    v2 = Int + 11;
    SV_GetConfigstring(v2, v6, 1024);
    v3 = Info_ValueForKey(v6, "ring");
    if (*v3)
        v4 = atol(v3);
    else
        v4 = 0;
    v5 = va("%i", v4 == 0);
    Info_SetValueForKey(v6, "ring", v5);
    SV_SetConfigstring(v2, v6);
}

void Scr_BulletTrace()
{
    int number; // r31
    int v1; // r30
    int EntityHitId; // r11
    const char *v3; // r3
    double v6; // fp11
    float v7[3]; // [sp+50h] [-90h] BYREF
    //float v8; // [sp+54h] [-8Ch]
    //float v9; // [sp+58h] [-88h]
    float v10[3]; // [sp+60h] [-80h] BYREF
    //float v11; // [sp+64h] [-7Ch]
    //float v12; // [sp+68h] [-78h]
    float v13[4]; // [sp+70h] [-70h] BYREF
    float v14[4]; // [sp+80h] [-60h] BYREF
    trace_t v15; // [sp+90h] [-50h] BYREF

    number = ENTITYNUM_NONE;
    v1 = 42002481;
    Scr_GetVector(0, v7);
    Scr_GetVector(1, v10);
    if (!Scr_GetInt(2))
        v1 = 8398897;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
        number = Scr_GetEntity(3)->s.number;
    G_LocationalTrace(&v15, v7, v10, number, v1, 0);
    Scr_MakeArray();
    Scr_AddFloat(v15.fraction);
    Scr_AddArrayStringIndexed(scr_const.fraction);
    v13[0] = (float)((float)(v10[0] - v7[0]) * v15.fraction) + v7[0];
    v13[1] = (float)((float)(v10[1] - v7[1]) * v15.fraction) + v7[1];
    v13[2] = (float)((float)(v10[2] - v7[2]) * v15.fraction) + v7[2];
    Scr_AddVector(v13);
    Scr_AddArrayStringIndexed(scr_const.position);
    EntityHitId = Trace_GetEntityHitId(&v15);
    if (EntityHitId == ENTITYNUM_NONE || EntityHitId == ENTITYNUM_WORLD)
        Scr_AddUndefined();
    else
        Scr_AddEntity(&g_entities[EntityHitId]);
    Scr_AddArrayStringIndexed(scr_const.entity);
    if (v15.fraction >= 1.0)
    {
        // aislop
        //_FP9 = -sqrtf((float)((float)((float)(v10 - v7) * (float)(v10 - v7))
        //    + (float)((float)((float)(v12 - v9) * (float)(v12 - v9))
        //        + (float)((float)(v11 - v8) * (float)(v11 - v8)))));
        //__asm { fsel      f11, f9, f10, f11 }
        //v6 = (float)((float)1.0 / (float)_FP11);
        {
            // Compute magnitude of vector difference (negated)
            float mag = -sqrtf(
                (v10[0] - v7[0]) * (v10[0] - v7[0]) +
                (v10[2] - v7[2]) * (v10[2] - v7[2]) +
                (v10[1] - v7[1]) * (v10[1] - v7[1])
            );

            // Use a fallback (e.g. 1.0f) if mag is negative
            float safeMag = (mag >= 0.0f) ? mag : 1.0f;

            // Compute inverse magnitude
            v6 = 1.0f / safeMag;
        }

        v14[0] = (float)v6 * (float)(v10[0] - v7[0]);
        v14[1] = (float)(v10[1] - v7[1]) * (float)v6;
        v14[2] = (float)(v10[2] - v7[2]) * (float)v6;
        Scr_AddVector(v14);
        Scr_AddArrayStringIndexed(scr_const.normal);
        Scr_AddConstString(scr_const.none);
    }
    else
    {
        Scr_AddVector(v15.normal);
        Scr_AddArrayStringIndexed(scr_const.normal);
        v3 = Com_SurfaceTypeToName((v15.surfaceFlags >> 20) & 0x1F);
        Scr_AddString(v3);
    }
    Scr_AddArrayStringIndexed(scr_const.surfacetype);
}

void Scr_BulletTracePassed()
{
    int number; // r31
    int v1; // r30
    int v2; // r3
    float v3[4]; // [sp+50h] [-40h] BYREF
    float v4[6]; // [sp+60h] [-30h] BYREF

    number = ENTITYNUM_NONE;
    v1 = 42002481;
    Scr_GetVector(0, v4);
    Scr_GetVector(1, v3);
    if (!Scr_GetInt(2))
        v1 = 8398897;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
        number = Scr_GetEntity(3)->s.number;
    v2 = G_LocationalTracePassed(v4, v3, number, ENTITYNUM_NONE, v1, 0);
    Scr_AddBool(v2);
}

void __cdecl Scr_SightTracePassed()
{
    int number; // r31
    int v1; // r30
    int v2[2]; // [sp+50h] [-40h] BYREF
    float v3[4]; // [sp+58h] [-38h] BYREF
    float v4[4]; // [sp+68h] [-28h] BYREF

    number = ENTITYNUM_NONE;
    v1 = 41998339;
    Scr_GetVector(0, v4);
    Scr_GetVector(1, v3);
    if (!Scr_GetInt(2))
        v1 = 8394755;
    if (Scr_GetType(3) == 1 && Scr_GetPointerType(3) == 20)
        number = Scr_GetEntity(3)->s.number;
    G_SightTrace(v2, v4, v3, number, v1);
    if (!v2[0])
        v2[0] = SV_FX_GetVisibility(v4, v3) < 0.2;
    Scr_AddBool(!v2[0]);
}

void Scr_PhysicsTrace()
{
    float v0[3]; // [sp+50h] [-70h] BYREF
    //float v1; // [sp+54h] [-6Ch]
    //float v2; // [sp+58h] [-68h]
    float v3[4]; // [sp+60h] [-60h] BYREF
    float v4[4]; // [sp+70h] [-50h] BYREF
    trace_t v5; // [sp+80h] [-40h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v3);
    G_TraceCapsule(&v5, v0, vec3_origin, vec3_origin, v3, ENTITYNUM_NONE, 8519697);
    v4[0] = (float)((float)(v3[0] - v0[0]) * v5.fraction) + v0[0];
    v4[1] = (float)((float)(v3[1] - v0[1]) * v5.fraction) + v0[1];
    v4[2] = (float)((float)(v3[2] - v0[2]) * v5.fraction) + v0[2];
    Scr_AddVector(v4);
}

void Scr_PlayerPhysicsTrace()
{
    float v0[3]; // [sp+50h] [-70h] BYREF
    //float v1; // [sp+54h] [-6Ch]
    //float v2; // [sp+58h] [-68h]
    float v3[4]; // [sp+60h] [-60h] BYREF
    float v4[4]; // [sp+70h] [-50h] BYREF
    trace_t v5; // [sp+80h] [-40h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v3);
    G_TraceCapsule(&v5, v0, playerMins, playerMaxs, v3, ENTITYNUM_NONE, 8519697);
    v4[0] = (float)((float)(v3[0] - v0[0]) * v5.fraction) + v0[0];
    v4[1] = (float)((float)(v3[1] - v0[1]) * v5.fraction) + v0[1];
    v4[2] = (float)((float)(v3[2] - v0[2]) * v5.fraction) + v0[2];
    Scr_AddVector(v4);
}

void Scr_RandomInt()
{
    int Int; // r3
    int v1; // r3

    Int = Scr_GetInt(0);
    if (Int > 0)
    {
        v1 = G_irand(0, Int);
        Scr_AddInt(v1);
    }
    else
    {
        Com_Printf(23, "RandomInt parm: %d  ", Int);
        Scr_Error("RandomInt parm must be positive integer.\n");
    }
}

void Scr_RandomFloat()
{
    double Float; // fp1
    double v1; // fp1

    Float = Scr_GetFloat(0);
    v1 = G_flrand(0.0, Float);
    Scr_AddFloat(v1);
}

void Scr_RandomIntRange()
{
    int Int; // r31
    int v1; // r3
    int v2; // r30
    int v3; // r3

    Int = Scr_GetInt(0);
    v1 = Scr_GetInt(1);
    v2 = v1;
    if (v1 <= Int)
    {
        Com_Printf(23, "RandomIntRange parms: %d %d ", Int, v1);
        Scr_Error("RandomIntRange's second parameter must be greater than the first.\n");
    }
    v3 = G_irand(Int, v2);
    Scr_AddInt(v3);
}

void Scr_RandomFloatRange()
{
    double Float; // fp31
    double v1; // fp30
    double v2; // fp1

    Float = Scr_GetFloat(0);
    v1 = Scr_GetFloat(1);
    if (v1 <= Float)
    {
        Com_Printf(23, (const char *)HIDWORD(Float), LODWORD(Float), LODWORD(v1));
        Scr_Error("Scr_RandomFloatRange's second parameter must be greater than the first.\n");
    }
    v2 = G_flrand(Float, v1);
    Scr_AddFloat(v2);
}

void GScr_sin()
{
    long double v0; // fp2
    long double v1; // fp2

    *(double *)&v0 = (float)(Scr_GetFloat(0) * (float)0.017453292);
    v1 = sin(v0);
    Scr_AddFloat((float)*(double *)&v1);
}

void GScr_cos()
{
    long double v0; // fp2
    long double v1; // fp2

    *(double *)&v0 = (float)(Scr_GetFloat(0) * (float)0.017453292);
    v1 = cos(v0);
    Scr_AddFloat((float)*(double *)&v1);
}

void GScr_tan()
{
    double v0; // fp31
    long double v1; // fp2
    long double v2; // fp2
    double v3; // fp30
    long double v4; // fp2
    double v5; // fp31

    v0 = (float)(Scr_GetFloat(0) * (float)0.017453292);
    *(double *)&v1 = v0;
    v2 = sin(v1);
    v3 = (float)*(double *)&v2;
    *(double *)&v2 = v0;
    v4 = cos(v2);
    v5 = (float)*(double *)&v4;
    if (v5 == 0.0)
        Scr_Error("divide by 0");
    Scr_AddFloat((float)((float)v3 / (float)v5));
}

void GScr_asin()
{
    long double v0; // fp2
    double v1; // fp31
    const char *v2; // r3
    long double v3; // fp2

    *(double *)&v0 = Scr_GetFloat(0);
    v1 = *(double *)&v0;
    if (*(double *)&v0 < -1.0 || *(double *)&v0 > 1.0)
    {
        v2 = va("%g out of range", *(double *)&v0);
        Scr_Error(v2);
    }
    *(double *)&v0 = v1;
    v3 = asin(v0);
    Scr_AddFloat((float)((float)*(double *)&v3 * (float)57.295776));
}

void GScr_acos()
{
    long double v0; // fp2
    double v1; // fp31
    const char *v2; // r3
    long double v3; // fp2

    *(double *)&v0 = Scr_GetFloat(0);
    v1 = *(double *)&v0;
    if (*(double *)&v0 < -1.0 || *(double *)&v0 > 1.0)
    {
        v2 = va("%g out of range", *(double *)&v0);
        Scr_Error(v2);
    }
    *(double *)&v0 = v1;
    v3 = acos(v0);
    Scr_AddFloat((float)((float)*(double *)&v3 * (float)57.295776));
}

void GScr_atan()
{
    long double v0; // fp2
    long double v1; // fp2

    *(double *)&v0 = Scr_GetFloat(0);
    v1 = atan(v0);
    Scr_AddFloat((float)((float)*(double *)&v1 * (float)57.295776));
}

void GScr_CastInt()
{
    int Type; // r3
    const char *TypeName; // r3
    const char *v2; // r3
    int Int; // r3
    const char *String; // r3
    int v5; // r3
    int Float; // [sp+50h] [-10h]

    Type = Scr_GetType(0);
    switch (Type)
    {
    case 2:
        String = Scr_GetString(0);
        v5 = atol(String);
        Scr_AddInt(v5);
        break;
    case 5:
        Float = (int)Scr_GetFloat(0);
        Scr_AddInt(Float);
        break;
    case 6:
        Int = Scr_GetInt(0);
        Scr_AddInt(Int);
        break;
    default:
        TypeName = Scr_GetTypeName(0);
        v2 = va("cannot cast %s to int", TypeName);
        Scr_ParamError(0, v2);
        break;
    }
}

void GScr_abs()
{
    double Float; // fp1

    Float = Scr_GetFloat(0);
    Scr_AddFloat(I_fabs(Float));
}

void GScr_min()
{
    //double f1; // fp31
    //
    //f1 = Scr_GetFloat(1);
    //_FP0 = (float)((float)f1 - Scr_GetFloat(0));
    //__asm { fsel      f1, f0, f1, f31# value }
    //
    //Scr_AddFloat(_FP1);

    float f1;
    float f2;
    float value;

    f1 = Scr_GetFloat(0);
    f2 = Scr_GetFloat(1);
    if (f2 - f1 < 0.0f)
        value = f2;
    else
        value = f1;
    Scr_AddFloat(value);
}

void GScr_max()
{
    //double Float; // fp31
    //
    //Float = Scr_GetFloat(1);
    //_FP0 = (float)(Scr_GetFloat(0) - (float)Float);
    //__asm { fsel      f1, f0, f1, f31# value }
    //Scr_AddFloat(_FP1);

    float f1;
    float f2;
    float value;

    f1 = Scr_GetFloat(0);
    f2 = Scr_GetFloat(1);
    if (f1 - f2 < 0.0f)
    {
        value = f2;
    }
    else
    {
        value = f1;
    }

    Scr_AddFloat(value);
}

void GScr_floor()
{
    long double v0; // fp2
    long double v1; // fp2

    *(double *)&v0 = Scr_GetFloat(0);
    v1 = floor(v0);
    Scr_AddFloat((float)*(double *)&v1);
}

void GScr_ceil()
{
    long double v0; // fp2
    long double v1; // fp2

    *(double *)&v0 = Scr_GetFloat(0);
    v1 = ceil(v0);
    Scr_AddFloat((float)*(double *)&v1);
}

void GScr_sqrt()
{
    double Float; // fp1

    Float = Scr_GetFloat(0);
    Scr_AddFloat(sqrtf(Float));
}

void GScr_VectorFromLineToPoint()
{
    double v0; // fp31
    double v1; // fp30
    double v2; // fp29
    double v3; // fp28
    double v4; // fp0
    float v5[4]; // [sp+50h] [-70h] BYREF
    float v6[4]; // [sp+60h] [-60h] BYREF
    float v7[4]; // [sp+70h] [-50h] BYREF
    float v8[14]; // [sp+80h] [-40h] BYREF

    Scr_GetVector(0, v5);
    Scr_GetVector(1u, v6);
    Scr_GetVector(2u, v7);
    v0 = (float)(v6[2] - v5[2]);
    v1 = (float)(v6[1] - v5[1]);
    v2 = (float)(v6[0] - v5[0]);
    v3 = (float)((float)((float)(v6[0] - v5[0]) * (float)(v6[0] - v5[0]))
        + (float)((float)((float)(v6[2] - v5[2]) * (float)(v6[2] - v5[2]))
            + (float)((float)(v6[1] - v5[1]) * (float)(v6[1] - v5[1]))));
    if (v3 == 0.0)
        Scr_ParamError(0, "The two points on the line must be different from each other");
    v4 = -(float)((float)((float)((float)(v7[0] - v5[0]) * (float)v2)
        + (float)((float)((float)(v7[2] - v5[2]) * (float)v0) + (float)((float)(v7[1] - v5[1]) * (float)v1)))
        / (float)v3);
    v8[0] = (float)((float)v2 * (float)v4) + (float)(v7[0] - v5[0]);
    v8[1] = (float)((float)v1 * (float)v4) + (float)(v7[1] - v5[1]);
    v8[2] = (float)((float)v0 * (float)v4) + (float)(v7[2] - v5[2]);
    Scr_AddVector(v8);
}

void GScr_PointOnSegmentNearestToPoint()
{
    double v0; // fp13
    double v1; // fp31
    double v2; // fp11
    double v3; // fp30
    double v4; // fp12
    double v5; // fp29
    double v6; // fp28
    double v7; // fp0
    float *v8; // r3
    float v9[4]; // [sp+50h] [-70h] BYREF
    float v10[4]; // [sp+60h] [-60h] BYREF
    float v11[4]; // [sp+70h] [-50h] BYREF
    float v12[14]; // [sp+80h] [-40h] BYREF

    Scr_GetVector(0, v9);
    Scr_GetVector(1u, v10);
    Scr_GetVector(2u, v11);
    v0 = v9[2];
    v1 = (float)(v10[2] - v9[2]);
    v2 = v9[0];
    v3 = (float)(v10[0] - v9[0]);
    v4 = v9[1];
    v5 = (float)(v10[1] - v9[1]);
    v6 = (float)((float)((float)(v10[1] - v9[1]) * (float)(v10[1] - v9[1]))
        + (float)((float)((float)(v10[2] - v9[2]) * (float)(v10[2] - v9[2]))
            + (float)((float)(v10[0] - v9[0]) * (float)(v10[0] - v9[0]))));
    if (v6 == 0.0)
    {
        Scr_ParamError(0, "Line segment must not have zero length");
        v0 = v9[2];
        v4 = v9[1];
        v2 = v9[0];
    }
    v7 = (float)((float)((float)((float)(v11[1] - (float)v4) * (float)v5)
        + (float)((float)((float)(v11[2] - (float)v0) * (float)v1)
            + (float)((float)(v11[0] - (float)v2) * (float)v3)))
        / (float)v6);
    if (v7 >= 0.0)
    {
        if (v7 <= 1.0)
        {
            v8 = v12;
            v12[0] = (float)((float)v3
                * (float)((float)((float)((float)(v11[1] - (float)v4) * (float)v5)
                    + (float)((float)((float)(v11[2] - (float)v0) * (float)v1)
                        + (float)((float)(v11[0] - (float)v2) * (float)v3)))
                    / (float)v6))
                + (float)v2;
            v12[1] = (float)((float)v5
                * (float)((float)((float)((float)(v11[1] - (float)v4) * (float)v5)
                    + (float)((float)((float)(v11[2] - (float)v0) * (float)v1)
                        + (float)((float)(v11[0] - (float)v2) * (float)v3)))
                    / (float)v6))
                + (float)v4;
            v12[2] = (float)((float)v1
                * (float)((float)((float)((float)(v11[1] - (float)v4) * (float)v5)
                    + (float)((float)((float)(v11[2] - (float)v0) * (float)v1)
                        + (float)((float)(v11[0] - (float)v2) * (float)v3)))
                    / (float)v6))
                + (float)v0;
        }
        else
        {
            v8 = v10;
        }
    }
    else
    {
        v8 = v9;
    }
    Scr_AddVector(v8);
}

void Scr_Distance()
{
    float v0[3]; // [sp+50h] [-30h] BYREF
    float v1[3]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v1);
    Scr_AddFloat(sqrtf((float)((float)((float)(v1[1] - v0[1]) * (float)(v1[1] - v0[1]))
        + (float)((float)((float)(v1[2] - v0[2]) * (float)(v1[2] - v0[2]))
            + (float)((float)(v1[0] - v0[0]) * (float)(v1[0] - v0[0]))))));
}

void Scr_Distance2D()
{
    double v0; // fp1
    float v1[3]; // [sp+50h] [-30h] BYREF
    float v2[3]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v2);
    Scr_GetVector(1, v1);
    v0 = Vec2Distance(v2, v1);
    Scr_AddFloat(v0);
}

void Scr_DistanceSquared()
{
    float v0[3]; // [sp+50h] [-30h] BYREF
    float v1[3]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v1);
    Scr_AddFloat((float)((float)((float)(v1[1] - v0[1]) * (float)(v1[1] - v0[1]))
        + (float)((float)((float)(v1[2] - v0[2]) * (float)(v1[2] - v0[2]))
            + (float)((float)(v1[0] - v0[0]) * (float)(v1[0] - v0[0])))));
}

void Scr_Length()
{
    float v0[3]; // [sp+50h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_AddFloat(sqrtf((float)((float)(v0[2] * v0[2]) + (float)((float)(v0[1] * v0[1]) + (float)(v0[0] * v0[0])))));
}

void Scr_LengthSquared()
{
    float v0[3]; // [sp+50h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_AddFloat((float)((float)(v0[2] * v0[2]) + (float)((float)(v0[1] * v0[1]) + (float)(v0[0] * v0[0]))));
}

void Scr_Closer()
{
    float v0[3]; // [sp+50h] [-40h] BYREF
    float v1[3]; // [sp+60h] [-30h] BYREF
    float v2[3]; // [sp+70h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v1);
    Scr_GetVector(2, v2);
    Scr_AddInt((float)((float)((float)(v0[1] - v1[1]) * (float)(v0[1] - v1[1]))
        + (float)((float)((float)(v0[2] - v1[2]) * (float)(v0[2] - v1[2]))
            + (float)((float)(v0[0] - v1[0]) * (float)(v0[0] - v1[0])))) 
        < (double)(float)((float)((float)(v0[0] - v2[0]) 
            * (float)(v0[0] - v2[0])) + (float)((float)((float)(v0[2] - v2[2]) 
            * (float)(v0[2] - v2[2])) + (float)((float)(v0[1] - v2[1]) * (float)(v0[1] - v2[1])))));
}

void Scr_VectorDot()
{
    float v0[3]; // [sp+50h] [-30h] BYREF
    float v1[3]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v0);
    Scr_GetVector(1, v1);
    Scr_AddFloat((float)((float)(v0[2] * v1[2]) + (float)((float)(v0[1] * v1[1]) + (float)(v1[0] * v0[0]))));
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
    float v0[3]; // [sp+50h] [-30h] BYREF
    float v1[3]; // [sp+60h] [-20h] BYREF

    if (Scr_GetNumParam() != 1)
        Scr_Error("wrong number of arguments to vectortoangle!");

    Scr_GetVector(0, v0);

    vectoangles(v0, v1);
    Scr_AddVector(v1);
}

void Scr_VectorLerp()
{
    double Float; // fp1
    float v1[3]; // [sp+50h] [-40h] BYREF
    //float v2; // [sp+54h] [-3Ch]
    //float v3; // [sp+58h] [-38h]
    float v4[4]; // [sp+60h] [-30h] BYREF
    float v5[6]; // [sp+70h] [-20h] BYREF

    if (Scr_GetNumParam() != 3)
        Scr_Error("wrong number of arguments to vectorlerp");
    Scr_GetVector(0, v1);
    Scr_GetVector(1, v4);
    Float = Scr_GetFloat(2);
    v5[0] = (float)((float)(v4[0] - v1[0]) * (float)Float) + v1[0];
    v5[1] = (float)((float)(v4[1] - v1[1]) * (float)Float) + v1[1];
    v5[2] = (float)((float)(v4[2] - v1[2]) * (float)Float) + v1[2];
    Scr_AddVector(v5);
}

void Scr_AnglesToUp()
{
    float v0[4]; // [sp+50h] [-30h] BYREF
    float v1[6]; // [sp+60h] [-20h] BYREF

    if (Scr_GetNumParam() != 1)
        Scr_Error("wrong number of arguments to anglestoup!");
    Scr_GetVector(0, v0);
    AngleVectors(v0, 0, 0, v1);
    Scr_AddVector(v1);
}

void Scr_AnglesToRight()
{
    float v0[4]; // [sp+50h] [-30h] BYREF
    float v1[6]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v0);
    AngleVectors(v0, 0, v1, 0);
    Scr_AddVector(v1);
}

void Scr_AnglesToForward()
{
    float v0[4]; // [sp+50h] [-30h] BYREF
    float v1[6]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v0);
    AngleVectors(v0, v1, 0, 0);
    Scr_AddVector(v1);
}

void Scr_CombineAngles()
{
    float v0[3]; // [sp+50h] [-C0h] BYREF
    float v1[3]; // [sp+60h] [-B0h] BYREF
    float v2[3]; // [sp+70h] [-A0h] BYREF
    float v3[4][3]; // [sp+80h] [-90h] BYREF
    float v4[4][3]; // [sp+B0h] [-60h] BYREF
    float v5[3][3]; // [sp+E0h] [-30h] BYREF

    Scr_GetVector(0, v2);
    Scr_GetVector(1, v1);

    AnglesToAxis(v2, v3);
    AnglesToAxis(v1, v4);
    MatrixMultiply((const mat3x3&)v4, (const mat3x3&)v3, v5);
    AxisToAngles(v5, v0);
    Scr_AddVector(v0);
}

void Scr_IsSubStr()
{
    Scr_AddBool(strstr(Scr_GetString(0), Scr_GetString(1)) != 0);
}

void Scr_GetSubStr()
{
    const char *String; // r26
    int Int; // r31
    int v2; // r28
    int v3; // r30
    char *v4; // r29
    char v5[1088]; // [sp+50h] [-440h] BYREF

    String = Scr_GetString(0);
    Int = Scr_GetInt(1);
    if (Scr_GetNumParam() < 3)
        v2 = 0x7FFFFFFF;
    else
        v2 = Scr_GetInt(2);
    v3 = 0;
    if (Int < v2)
    {
        v4 = &v5[-Int];
        do
        {
            if (v3 >= 1024)
                Scr_Error("string too long");
            if (!String[Int])
                break;
            v4[Int] = String[Int];
            ++Int;
            ++v3;
        } while (Int < v2);
    }
    v5[v3] = 0;
    Scr_AddString(v5);
}

void Scr_ToLower()
{
    const char *String; // r30
    int v1; // r31
    char v2; // r3
    char v3[1032]; // [sp+50h] [-420h] BYREF

    String = Scr_GetString(0);
    v1 = 0;
    while (1)
    {
        v2 = tolower(*String);
        v3[v1] = v2;
        if (!v2)
            break;
        ++v1;
        ++String;
        if (v1 >= 1024)
        {
            Scr_Error("string too long");
            return;
        }
    }
    Scr_AddString(v3);
}

void Scr_StrTok()
{
    unsigned int ConstString; // r26
    unsigned int v1; // r25
    const char *v2; // r31
    const char *v3; // r28
    const char *v4; // r11
    int v6; // r30
    int v7; // r29
    char v8; // r9
    int v9; // r10
    int v10; // r11
    char v11[1104]; // [sp+50h] [-450h] BYREF

    ConstString = Scr_GetConstString(0);
    v1 = Scr_GetConstString(1);
    v2 = SL_ConvertToString(ConstString);
    v3 = SL_ConvertToString(v1);
    SL_AddRefToString(ConstString);
    SL_AddRefToString(v1);
    v4 = v3;
    while (*(unsigned __int8 *)v4++)
        ;
    v6 = 0;
    v7 = v4 - v3 - 1;
    Scr_MakeArray();
    v8 = *v2;
    v9 = *v2;
    if (*v2)
    {
        do
        {
            v10 = 0;
            if (v7 <= 0)
            {
            LABEL_7:
                v11[v6] = v8;
                if (++v6 >= 1024)
                {
                    SL_RemoveRefToString(ConstString);
                    SL_RemoveRefToString(v1);
                    Scr_Error("string too long");
                }
            }
            else
            {
                while (v9 != v3[v10])
                {
                    if (++v10 >= v7)
                        goto LABEL_7;
                }
                if (v6)
                {
                    v11[v6] = 0;
                    Scr_AddString(v11);
                    Scr_AddArray();
                    v6 = 0;
                }
            }
            v8 = *++v2;
            v9 = *v2;
        } while (*v2);
        if (v6)
        {
            v11[v6] = 0;
            Scr_AddString(v11);
            Scr_AddArray();
        }
    }
    SL_RemoveRefToString(ConstString);
    SL_RemoveRefToString(v1);
}

void Scr_SetBlur()
{
    double Float; // fp29
    double v1; // fp31
    const char *v2; // r3

    Float = Scr_GetFloat(0);
    v1 = Scr_GetFloat(1);
    if (v1 < 0.0)
        Scr_ParamError(1u, "Time must be positive");
    if (Float < 0.0)
        Scr_ParamError(0, "Blur value must be greater than 0");
    v2 = va("scr_blur %i %f %i %i", (int)(float)((float)v1 * (float)1000.0), Float, LODWORD(Float), 0);
    SV_GameSendServerCommand(-1, v2);
}

void Scr_MusicPlay()
{
    int Int; // r30
    unsigned int NumParam; // r3
    const char *String; // r31
    const char *v3; // r3
    const char *v4; // r3
    const char *v5; // r3

    if (!chaplinCheat->current.enabled || Scr_GetNumParam() >= 3 && Scr_GetInt(2))
    {
        Int = 1;
        NumParam = Scr_GetNumParam();
        if (NumParam != 1)
        {
            if (NumParam - 2 > 1)
            {
                v5 = va("Incorrect number of parameters\n");
                Scr_Error(v5);
                return;
            }
            Int = Scr_GetInt(1);
        }
        String = Scr_GetString(0);
        if (!Com_FindSoundAlias(String))
        {
            v3 = va("unknown sound alias '%s'", String);
            Scr_ParamError(0, v3);
        }
        v4 = va("mu_play %s %i", String, Int);
        SV_GameSendServerCommand(-1, v4);
    }
}

void Scr_MusicStop()
{
    unsigned int NumParam; // r3
    const char *v1; // r3
    long double v2; // fp2
    long double v3; // fp2
    int v4; // r31
    const char *v5; // r3
    const char *v6; // r3

    if (!chaplinCheat->current.enabled || Scr_GetNumParam() >= 2 && Scr_GetInt(1))
    {
        NumParam = Scr_GetNumParam();
        if (NumParam)
        {
            if (NumParam >= 3)
            {
                v1 = va("Incorrect number of parameters\n");
                Scr_Error(v1);
                return;
            }
            *(double *)&v2 = (float)((float)(Scr_GetFloat(0) * (float)1000.0) + (float)0.5);
            v3 = floor(v2);
            v4 = (int)(float)*(double *)&v3;
            if (v4 < 0)
            {
                v5 = va("musicStop: fade time must be >= 0\n");
                Scr_Error(v5);
            }
        }
        else
        {
            v4 = 0;
        }
        v6 = va("mu_stop %i", v4);
        SV_GameSendServerCommand(-1, v6);
    }
}

void Scr_SoundFade()
{
    double Float; // fp31
    const char *v1; // r3

    Float = Scr_GetFloat(0);
    if (Scr_GetNumParam() > 1)
        Scr_GetFloat(1);
    v1 = va("snd_fade %f %i\n", Float, LODWORD(Float));
    SV_GameSendServerCommand(-1, v1);
}

void Scr_Amplify()
{
    unsigned int NumParam; // r3
    const char *v1; // r3
    float falloff; // fp1
    const char *v3; // r3
    int minrange; // r3
    int minrange_; // r31
    const char *v6; // r3
    int maxrange; // r30
    const char *v8; // r3
    float minvol; // fp1
    float minvol_; // fp31
    const char *v11; // r3
    float maxvol; // fp1
    float maxvol_; // fp30
    const char *v14; // r3
    const char *v15; // r3
    float origin[4]; // [sp+60h] [-40h] BYREF

    NumParam = Scr_GetNumParam();
    if (NumParam != 5)
    {
        if (NumParam != 6)
        {
            v1 = va("amplify takes 5 or 6 arguments: called with %i\n", NumParam);
            Scr_Error(v1);
            return;
        }
        falloff = Scr_GetFloat(5);
        if (falloff < 0.0)
        {
            v3 = va("amplify: falloff parameter must be >= 0: %f\n", falloff);
            Scr_ParamError(6u, v3);
        }
    }
    Scr_GetVector(0, origin);
    minrange = Scr_GetInt(1);
    minrange_ = minrange;
    if (minrange < 0)
    {
        v6 = va("amplify: min_range parameter must be >= 0: %i\n", minrange);
        Scr_ParamError(1u, v6);
    }
    maxrange = Scr_GetInt(2);
    if (maxrange < minrange_)
    {
        v8 = va("amplify: max_range parameter must be >= min_range: %i >= %i\n", maxrange, minrange_);
        Scr_ParamError(2u, v8);
    }
    minvol = Scr_GetFloat(3);
    minvol_ = minvol;
    if (minvol < 0.0)
    {
        v11 = va("amplify: min_vol parameter must be >= 0: %f\n", minvol);
        Scr_ParamError(3u, v11);
    }
    maxvol = Scr_GetFloat(4);
    maxvol_ = maxvol;
    if (maxvol < minvol_)
    {
        Scr_ParamError(4, va("amplify: max_vol parameter must be >= min_vol: %f >= %f\n", maxvol, minvol_));
    }
    // float *org, int min_r, int max_r, double min_vol, double max_vol, double falloff
    v15 = va(
        "amp %f %f %f %i %i %f %f %f\n",
        origin[0],
        origin[1],
        origin[2],
        minrange,
        maxrange,
        minvol,
        maxvol,
        falloff);

    SV_GameSendServerCommand(-1, v15);
}

void Scr_AmplifyStop()
{
    SV_GameSendServerCommand(-1, "amp_stop");
}

void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, const char *assetName)
{
    const char *XAssetTypeName; // r3

    DB_FindXAssetHeader(type, assetName);
    if (DB_IsXAssetDefault(type, assetName))
    {
        XAssetTypeName = DB_GetXAssetTypeName((int)type);
        Scr_NeverTerminalError(va("precache %s '%s' failed", XAssetTypeName, assetName));
    }
}

int Scr_PrecacheModel()
{
    const char *String; // r31
    const char *v1; // r5

    if (!level.initializing)
        Scr_Error("precacheModel must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    if (!*String)
        Scr_ParamError(0, "Model name string is empty");
    Scr_ErrorOnDefaultAsset(ASSET_TYPE_XMODEL, String);
    return G_ModelIndex(String);
}

void Scr_PrecacheShellShock()
{
    const char *String; // r31
    int v1; // r30
    shellshock_parms_t *ShellshockParms; // r3

    if (!level.initializing)
        Scr_Error("precacheShellShock must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    v1 = G_ShellShockIndex(String);
    if (!BG_LoadShellShockDvars(String))
        Com_Error(ERR_DROP, "couldn't find shellshock '%s' -- see console", String);
    ShellshockParms = BG_GetShellshockParms(v1);
    BG_SetShellShockParmsFromDvars(ShellshockParms);
}

void Scr_PrecacheItem()
{
    const char *String; // r31
    const char *v1; // r3

    if (!level.initializing)
        Scr_Error("precacheItem must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    if (!G_FindItem(String, 0))
    {
        v1 = va("unknown item '%s'", String);
        Scr_ParamError(0, v1);
    }
}

int Scr_PrecacheMaterial()
{
    const char *String; // r31

    if (!level.initializing)
        Scr_Error("precacheShader must be called before any wait statements in the gametype or level script\n");
    String = Scr_GetString(0);
    if (!*String)
        Scr_ParamError(0, "Shader name string is empty");
    Scr_ErrorOnDefaultAsset(ASSET_TYPE_MATERIAL, String);
    return G_MaterialIndex(String);
}

void Scr_PrecacheString()
{
    char v0[1032]; // [sp+50h] [-410h] BYREF

    if (!level.initializing)
        Scr_Error("precacheString must be called before any wait statements in the gametype or level script\n");
    Scr_ConstructMessageString(0, 0, "Precache String", v0, 0x400u);
    if (v0[0])
        G_LocalizedStringIndex(v0);
}

int Scr_PrecacheRumble()
{
    const char *String; // r3

    if (!level.initializing)
        Scr_Error("precacheRumble must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    return G_RumbleIndex(String);
}

void GScr_PrecacheLocationSelector()
{
    const char *String; // r30
    int i; // r31
    int j; // r31
    const char *v3; // r3
    char v4[1032]; // [sp+50h] [-420h] BYREF

    String = Scr_GetString(0);
    for (i = 0; i < 3; ++i)
    {
        SV_GetConfigstring(i + 1152, v4, 1024);
        if (!I_stricmp(v4, String))
        {
            Com_DPrintf(23, "Script tried to precache the location selector '%s' more than once\n", String);
            return;
        }
    }
    for (j = 0; j < 3; ++j)
    {
        SV_GetConfigstring(j + 1152, v4, 1024);
        if (!v4[0])
            break;
    }
    if (j == 3)
    {
        v3 = va("Too many location selectors precached. Max allowed is %i", 3);
        Scr_Error(v3);
    }
    SV_SetConfigstring(j + 1152, String);
}

void Scr_PrecacheNightvisionCodeAssets()
{
    if (level.initializing)
        SV_SetConfigstring(1151, "1");
    else
        Scr_Error("PrecacheNightvisionCodeAssets() must be called during level initialization.\n");
}

int __cdecl GScr_GetLocSelIndex(const char *mtlName)
{
    int v2; // r31
    const char *v3; // r3
    char v5[1032]; // [sp+50h] [-420h] BYREF

    if (!mtlName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7315, 0, "%s", "mtlName");
    if (!*mtlName)
        return 0;
    v2 = 0;
    while (1)
    {
        SV_GetConfigstring(v2 + 1152, v5, 1024);
        if (!I_stricmp(v5, mtlName))
            break;
        if (++v2 >= 3)
        {
            v3 = va("Location selector '%s' was not precached\n", mtlName);
            Scr_Error(v3);
            return 0;
        }
    }
    return v2 + 1;
}

void Scr_AmbientPlay()
{
    int v0; // r30
    unsigned int NumParam; // r3
    const char *v2; // r3
    long double v3; // fp2
    long double v4; // fp2
    const char *String; // r31
    const char *v6; // r3
    const char *v7; // r3
    const char *v8; // r3
    const char *v9; // r3

    v0 = 0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            v2 = va("Incorrect number of parameters");
            Scr_Error(v2);
            return;
        }
        *(double *)&v3 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
        v4 = floor(v3);
        v0 = (int)(float)*(double *)&v4;
    }
    String = Scr_GetString(0);
    if (!*String)
    {
        v6 = va("ambientPlay: alias name cannot be the empty string... use stop or fade version\n");
        Scr_Error(v6);
    }
    if (v0 < 0)
    {
        v7 = va("ambientPlay: fade time must be >= 0\n");
        Scr_Error(v7);
    }
    if (!Com_FindSoundAlias(String))
    {
        v8 = va("unknown sound alias '%s'", String);
        Scr_ParamError(0, v8);
    }
    v9 = va("n\\%s\\t\\%i", String, level.time + v0);
    SV_SetConfigstring(1114, v9);
}

void Scr_AmbientStop()
{
    unsigned int NumParam; // r3
    const char *v1; // r3
    long double v2; // fp2
    long double v3; // fp2
    int v4; // r31
    const char *v5; // r3
    const char *v6; // r3

    NumParam = Scr_GetNumParam();
    if (NumParam)
    {
        if (NumParam != 1)
        {
            v1 = va("Incorrect number of parameters\n");
            Scr_Error(v1);
            return;
        }
        *(double *)&v2 = (float)((float)(Scr_GetFloat(0) * (float)1000.0) + (float)0.5);
        v3 = floor(v2);
        v4 = (int)(float)*(double *)&v3;
        if (v4 < 0)
        {
            v5 = va("ambientStop: fade time must be >= 0\n");
            Scr_Error(v5);
        }
    }
    else
    {
        v4 = 0;
    }
    v6 = va("t\\%i", level.time + v4);
    SV_SetConfigstring(1114, v6);
}

void __cdecl Scr_CheckForSaveErrors(int saveId)
{
    switch (saveId)
    {
    case -2:
        Scr_Error("Attempting to save multiple times in a single frame.\n");
        break;
    case -1:
        Scr_Error("Attempting to save during a restart.\n");
        break;
    case -3:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            7425,
            0,
            "%s",
            "saveId != INVALID_SAVE_PENDING_SAVE_BUFFER_EXCEEDED");
        break;
    }
}

void Scr_SaveGame()
{
    const char *IString; // r27
    const char *String; // r28
    const char *v2; // r3
    const char *v3; // r31
    const char *v4; // r11
    int NumParam; // r30
    const char *v7; // r29
    bool v8; // r30
    int v9; // r3
    char v10[112]; // [sp+50h] [-70h] BYREF

    IString = "";
    if (g_entities[0].health <= 0)
        Scr_Error("Attempting to save while dead\n");
    String = Dvar_GetString("mapname");
    v2 = Scr_GetString(0);
    v3 = v2;
    v4 = v2;
    while (*(unsigned __int8 *)v4++)
        ;
    if (v4 - v2 == 1)
        v3 = "auto";
    NumParam = Scr_GetNumParam();
    if (NumParam > 1)
        IString = Scr_GetIString(1);
    if (NumParam <= 2)
        v7 = "$default";
    else
        v7 = Scr_GetString(2);
    v8 = NumParam > 3 && Scr_GetInt(3) != 0;
    if (I_stricmp(v3, "levelstart"))
        Com_sprintf(v10, 64, "autosave\\%s-%s", String, v3);
    else
        Com_sprintf(v10, 64, "autosave\\%s", String);
    SaveMemory_ClearForcedCommitFlag();
    v9 = SV_AddPendingSave(v10, IString, v7, SAVE_TYPE_AUTOSAVE, 2u, v8);
    switch (v9)
    {
    case -2:
        Scr_Error("Attempting to save multiple times in a single frame.\n");
        break;
    case -1:
        Scr_Error("Attempting to save during a restart.\n");
        break;
    case -3:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            7425,
            0,
            "%s",
            "saveId != INVALID_SAVE_PENDING_SAVE_BUFFER_EXCEEDED");
        break;
    }
}

void Scr_SaveGameNoCommit()
{
    const char *IString; // r27
    const char *String; // r28
    const char *v2; // r3
    const char *v3; // r31
    const char *v4; // r11
    int NumParam; // r30
    const char *v7; // r29
    bool v8; // r30
    int v9; // r3
    int v10; // r31
    char v11[112]; // [sp+50h] [-70h] BYREF

    IString = "";
    if (g_entities[0].health <= 0)
        Scr_Error("Attempting to save while dead\n");
    String = Dvar_GetString("mapname");
    v2 = Scr_GetString(0);
    v3 = v2;
    v4 = v2;
    while (*(unsigned __int8 *)v4++)
        ;
    if (v4 - v2 == 1)
        v3 = "auto";
    NumParam = Scr_GetNumParam();
    if (NumParam > 1)
        IString = Scr_GetIString(1);
    if (NumParam <= 2)
        v7 = "$default";
    else
        v7 = Scr_GetString(2);
    v8 = NumParam > 3 && Scr_GetInt(3) != 0;
    if (I_stricmp(v3, "levelstart"))
        Com_sprintf(v11, 64, "autosave\\%s-%s", String, v3);
    else
        Com_sprintf(v11, 64, "autosave\\%s", String);
    SaveMemory_ClearForcedCommitFlag();
    v9 = SV_AddPendingSave(v11, IString, v7, SAVE_TYPE_AUTOSAVE, 1u, v8);
    v10 = v9;
    if (v9 == -2)
    {
        Scr_Error("Attempting to save multiple times in a single frame.\n");
        Scr_AddInt(-2);
    }
    else if (v9 == -1)
    {
        Scr_Error("Attempting to save during a restart.\n");
        Scr_AddInt(-1);
    }
    else
    {
        if (v9 == -3)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
                7425,
                0,
                "%s",
                "saveId != INVALID_SAVE_PENDING_SAVE_BUFFER_EXCEEDED");
        Scr_AddInt(v10);
    }
}

void Scr_IsSaveSuccessful()
{
    SaveGame *SaveHandle; // r3
    bool IsSuccessful; // r3

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    IsSuccessful = SaveMemory_IsSuccessful(SaveHandle);
    Scr_AddBool(IsSuccessful);
}

void Scr_IsRecentlyLoaded()
{
    bool IsRecentlyLoaded; // r3

    IsRecentlyLoaded = SV_SaveMemory_IsRecentlyLoaded();
    Scr_AddBool(IsRecentlyLoaded);
}

void Scr_CommitSave()
{
    SaveGame *SaveHandle; // r30
    int Int; // r26
    const SaveHeader *Header; // r31
    int saveId; // r11
    const SaveHeader *v4; // r31
    int v5; // r5
    const char *v6; // r3

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7598, 0, "%s", "save");
    Int = Scr_GetInt(0);
    if (SaveMemory_IsCommitForced())
    {
        SaveMemory_ClearForcedCommitFlag();
        return;
    }
    if (SaveMemory_IsWaitingForCommit(SaveHandle))
        goto LABEL_14;
    SaveHandle = SaveMemory_GetSaveHandle(SAVE_LAST_COMMITTED);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7613, 0, "%s", "save");
    Header = SaveMemory_GetHeader(SaveHandle);
    if (!Header)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7615, 0, "%s", "header");
    saveId = Header->saveId;
    if (Int != saveId && saveId && !sv.demo.playing)
    {
        Scr_Error("Save Commit attempted with no valid committable save available");
    LABEL_14:
        v4 = SaveMemory_GetHeader(SaveHandle);
        if (!v4)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7625, 0, "%s", "header");
        v5 = v4->saveId;
        if (Int != v5)
        {
            v6 = va("Save Commit attempted on incorrect save currentsave:%i, commit:%i", Int, v5);
            Scr_Error(v6);
        }
        if (!G_CommitSavedGame(Int))
            Scr_Error("Attempting to commit an invalid save");
    }
}

void __cdecl GScr_RadiusDamageInternal(gentity_s *inflictor)
{
    double range; // fp31
    double maxDamage; // fp30
    double minDamage; // fp29
    gentity_s *attacker; // r29
    int mod; // r28
    unsigned int weapon; // r30
    float origin[4]; // [sp+70h] [-50h] BYREF

    Scr_GetVector(0, origin);
    range = Scr_GetFloat(1);
    maxDamage = Scr_GetFloat(2);
    minDamage = Scr_GetFloat(3);
    attacker = &g_entities[ENTITYNUM_WORLD];
    if (Scr_GetNumParam() > 4 && Scr_GetType(4))
        attacker = Scr_GetEntity(4);
    mod = MOD_EXPLOSIVE;
    if (Scr_GetNumParam() > 5 && Scr_GetType(5))
        mod = G_MeansOfDeathFromScriptParam(5);
    weapon = -1;
    if (Scr_GetNumParam() > 6 && Scr_GetType(6))
    {
        weapon = G_GetWeaponIndexForName(Scr_GetString(6));
    }
    level.bPlayerIgnoreRadiusDamage = level.bPlayerIgnoreRadiusDamageLatched;
    G_RadiusDamage(origin, inflictor, attacker, maxDamage, minDamage, range, 1.0f, 0, inflictor, mod, weapon);
    level.bPlayerIgnoreRadiusDamage = 0;
}

void GScr_RadiusDamage()
{
    GScr_RadiusDamageInternal(0);
}

void __cdecl GScr_EntityRadiusDamage(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    GScr_RadiusDamageInternal(Entity);
}

void __cdecl GScr_Detonate(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    WeaponDef *WeaponDef; // r3
    gentity_s *v3; // r31

    Entity = GetEntity(entref);
    WeaponDef = BG_GetWeaponDef(Entity->s.weapon);
    if (Entity->s.eType != 3 || !WeaponDef || WeaponDef->weapType != WEAPTYPE_GRENADE)
        Scr_ObjectError("entity is not a grenade");
    if (Scr_GetNumParam())
    {
        if (Scr_GetType(0))
        {
            v3 = Scr_GetEntity(0);
            if (!v3->client)
                Scr_ParamError(0, "Entity is not a player");
        }
        else
        {
            v3 = &g_entities[ENTITYNUM_WORLD];
        }
        Entity->parent.setEnt(v3);
    }
    G_ExplodeMissile(Entity);
}

int GScr_SetPlayerIgnoreRadiusDamage()
{
    int result; // r3

    result = Scr_GetInt(0);
    level.bPlayerIgnoreRadiusDamageLatched = result;
    return result;
}

void __cdecl GScr_DamageConeTraceInternal(scr_entref_t entref, int contentMask)
{
    gentity_s *target; // r31
    gentity_s *inflictor; // r30
    float damageAngles[4]; // [sp+60h] [-40h] BYREF

    target = GetEntity(entref);
    inflictor = 0;
    if (Scr_GetNumParam())
        inflictor = Scr_GetEntity(1);
    Scr_GetVector(0, damageAngles);
    if (G_CanRadiusDamageFromPos(
        target,
        target->r.currentOrigin,
        inflictor,
        damageAngles,
        FLT_MAX,
        1.0, // coneanglecos
        NULL, // conedirection
        0.0f, // maxheight
        true, // use eye offset
        contentMask))
    {
        Scr_AddFloat(1.0);
    }
    else
    {
        Scr_AddFloat(0.0);
    }
}

void __cdecl GScr_DamageConeTrace(scr_entref_t entref)
{
    GScr_DamageConeTraceInternal(entref, 8396817);
}

void __cdecl GScr_SightConeTrace(scr_entref_t entref)
{
    GScr_DamageConeTraceInternal(entref, 2049);
}

void GScr_ChangeLevel()
{
    gentity_s *v0; // r31
    unsigned int NumParam; // r3
    long double v2; // fp2
    long double v3; // fp2
    const char *String; // r30
    long double v5; // fp2
    long double v6; // fp2

    v0 = G_Find(0, 284, scr_const.player);
    if (!v0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 7856, 0, "%s", "player");
    if (v0->health > 0 && !g_reloading->current.integer)
    {
        NumParam = Scr_GetNumParam();
        if (NumParam != 1)
        {
            if (NumParam != 2)
            {
                *(double *)&v2 = (float)((float)(Scr_GetFloat(2) * (float)1000.0) + (float)0.5);
                v3 = floor(v2);
                level.exitTime = (int)(float)*(double *)&v3;
                if ((int)(float)*(double *)&v3 < 0)
                    Scr_ParamError(1u, "exitTime cannot be negative");
            }
            level.savepersist = Scr_GetInt(1);
        }
        String = Scr_GetString(0);
        if (Scr_GetNumParam() > 2 && g_changelevel_time->current.value > 0.0)
        {
            *(double *)&v5 = (float)((float)(g_changelevel_time->current.value * (float)1000.0) + (float)0.5);
            v6 = floor(v5);
            level.exitTime = (int)(float)*(double *)&v6;
        }
        level.changelevel = 1;
        G_SetNextMap(String);
    }
}

void GScr_MissionSuccess()
{
    if (Scr_GetNumParam() > 2)
        Scr_Error("missionSuccess only takes two parameters: missionSuccess(nextMap, persistent)\n");
    //LSP_LogString(cl_controller_in_use, "mission success");
    //LSP_SendLogRequest(cl_controller_in_use);
    GScr_ChangeLevel();
    level.bMissionSuccess = 1;
}

void GScr_MissionFailed()
{
    gentity_s *v0; // r3

    v0 = G_Find(0, 284, scr_const.player);
    if (v0)
    {
        respawn(v0);
        level.bMissionFailed = 1;
    }
}

void __cdecl GScr_SetMissionDvar()
{
    const char *String; // r31
    const char **v1; // r29
    const char *v2; // r10
    const char *v3; // r11
    int v4; // r8
    const dvar_s *Var; // r30
    const char *v6; // r3
    int type; // r5
    const char *v8; // r3
    const char *v9; // r3
    int Int; // r3
    const char *VariantString; // r3
    const char *v12; // r3
    const char *v13; // r3

    String = Scr_GetString(0);
    v1 = missionDvarList;
    if (!missionDvarList[0])
    {
    LABEL_12:
        v9 = va("Invalid mission dvar name: %s\n", String);
        Scr_Error(v9);
        return;
    }
    while (1)
    {
        v2 = *v1;
        v3 = String;
        do
        {
            v4 = *(unsigned __int8 *)v3 - *(unsigned __int8 *)v2;
            if (!*v3)
                break;
            ++v3;
            ++v2;
        } while (!v4);
        if (!v4)
            goto LABEL_11;
        Var = Dvar_FindVar(String);
        if (!Var)
        {
            v6 = va("Mission dvar has valid name, but was not found: %s\n", String);
            Scr_Error(v6);
        }
        type = Var->type;
        if (type == 5)
        {
            Int = Scr_GetInt(1);
            Dvar_SetIntByName(String, Int);
            VariantString = Dvar_GetVariantString(String);
            Com_Printf(16, " * SetMissionDvar(): \"%s\" set to \"%s\".\n", String, VariantString);
            return;
        }
        if (type == 7)
            break;
        v8 = va("Mission dvar has valid name, but invalid type: %s is of type %i\n", String, type);
        Scr_Error(v8);
    LABEL_11:
        if (!*++v1)
            goto LABEL_12;
    }
    v12 = Scr_GetString(1);
    Dvar_SetStringByName(String, v12);
    v13 = Dvar_GetVariantString(String);
    Com_Printf(16, " * SetMissionDvar(): \"%s\" set to \"%s\".\n", String, v13);
}

void GScr_Cinematic()
{
    long double v0; // fp2
    long double v1; // fp2
    const char *String; // r30
    long double v3; // fp2
    long double v4; // fp2

    if (!g_reloading->current.integer)
    {
        if (Scr_GetNumParam() != 1)
        {
            *(double *)&v0 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
            v1 = floor(v0);
            level.exitTime = (int)(float)*(double *)&v1;
            if ((int)(float)*(double *)&v1 < 0)
                Scr_ParamError(1u, "exitTime cannot be negative");
        }
        String = Scr_GetString(0);
        if (Scr_GetNumParam() > 2 && g_changelevel_time->current.value > 0.0)
        {
            *(double *)&v3 = (float)((float)(g_changelevel_time->current.value * (float)1000.0) + (float)0.5);
            v4 = floor(v3);
            level.exitTime = (int)(float)*(double *)&v4;
        }
        level.changelevel = 1;
        I_strncpyz(level.cinematic, String, 64);
        level.bMissionSuccess = 1;
    }
}

void GScr_CinematicInGame()
{
    const char *String; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("cinematicingame takes one parameter: cinematicingame(<cinematic name>)\n");
    String = Scr_GetString(0);
    R_Cinematic_SetNextPlayback(String, 0);
}

void GScr_CinematicInGameSync()
{
    const char *String; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("cinematicingamesync takes one parameter: cinematicingamesync(<cinematic name>)\n");
    String = Scr_GetString(0);
    R_Cinematic_SetNextPlayback(String, 5u);
}

void GScr_CinematicInGameLoop()
{
    const char *String; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("cinematicingameloop takes one parameter: cinematicingameloop(<cinematic name>)\n");
    String = Scr_GetString(0);
    R_Cinematic_SetNextPlayback(String, 2u);
}

void GScr_CinematicInGameLoopResident()
{
    const char *String; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("CinematicInGameLoopResident takes one parameter: CinematicInGameLoopResident(<cinematic name>)\n");
    String = Scr_GetString(0);
    R_Cinematic_SetNextPlayback(String, 0x1Au);
}

void GScr_CinematicInGameLoopFromFastfile()
{
    const char *String; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("CinematicInGameLoopFromFastfile takes one parameter: CinematicInGameLoopFromFastfile(<cinematic name>)\n");
    String = Scr_GetString(0);
    R_Cinematic_SetNextPlayback(String, 0x3Au);
}

void GScr_StopCinematicInGame()
{
    if (Scr_GetNumParam())
        Scr_Error("stopcinematicingame takes no parameters: stopcinematicingame()\n");
    R_Cinematic_UnsetNextPlayback();
    R_Cinematic_StopPlayback();
}

void GScr_Earthquake()
{
    double Float; // fp30
    long double v1; // fp2
    long double v2; // fp2
    double v3; // fp29
    gentity_s *v4; // r3
    int v5; // [sp+50h] [-40h]
    float v6[4]; // [sp+58h] [-38h] BYREF

    Float = Scr_GetFloat(0);
    *(double *)&v1 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
    v2 = floor(v1);
    v5 = (int)(float)*(double *)&v2;
    Scr_GetVector(2u, v6);
    v3 = Scr_GetFloat(3);
    if (Float <= 0.0)
        Scr_ParamError(0, "Scale must be greater than 0");
    if (v5 <= 0)
        Scr_ParamError(1u, "duration must be greater than 0");
    if (v3 <= 0.0)
        Scr_ParamError(3u, "Radius must be greater than 0");
    if (g_earthquakeEnable->current.enabled)
    {
        v4 = G_TempEntity(v6, 65);
        v4->s.lerp.u.turret.gunAngles[0] = Float;
        v4->s.lerp.u.turret.gunAngles[1] = v3;
        v4->s.lerp.u.earthquake.duration = v5;
    }
}

int GScr_DrawCompassFriendlies()
{
    int result; // r3

    result = Scr_GetInt(0);
    level.bDrawCompassFriendlies = result;
    return result;
}

void GScr_WeaponFireTime()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    __int64 v2; // r11

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    LODWORD(v2) = BG_GetWeaponDef(WeaponIndexForName)->iFireTime;
    Scr_AddFloat((float)((float)v2 * (float)0.001));
}

void GScr_WeaponClipSize()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddInt(WeaponDef->iClipSize);
}

void __cdecl GScr_GetAmmoCount(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r29
    const char *String; // r31
    int WeaponIndexForName; // r30
    int v4; // r3

    PlayerEntity = GetPlayerEntity(entref);
    if (!PlayerEntity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 8290, 0, "%s", "ent");
    if (!PlayerEntity->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 8291, 0, "%s", "ent->client");
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    v4 = BG_WeaponAmmo(&PlayerEntity->client->ps, WeaponIndexForName);
    Scr_AddInt(v4);
}

void GScr_WeaponIsSemiAuto()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddInt(WeaponDef->fireType == WEAPON_FIRETYPE_SINGLESHOT);
}

void GScr_WeaponIsBoltAction()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddInt(WeaponDef->bBoltAction);
}

void GScr_WeaponType()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3
    const char *WeaponTypeName; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    WeaponTypeName = BG_GetWeaponTypeName(WeaponDef->weapType);
    Scr_AddString(WeaponTypeName);
}

void GScr_WeaponClass()
{
    const char *String; // r3
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3
    const char *WeaponClassName; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (WeaponIndexForName)
    {
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        WeaponClassName = BG_GetWeaponClassName(WeaponDef->weapClass);
        Scr_AddString(WeaponClassName);
    }
    else
    {
        Scr_AddString("none");
    }
}

void GScr_WeaponInventoryType()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3
    const char *WeaponInventoryTypeName; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    WeaponInventoryTypeName = BG_GetWeaponInventoryTypeName(WeaponDef->inventoryType);
    Scr_AddString(WeaponInventoryTypeName);
}

void GScr_WeaponStartAmmo()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddInt(WeaponDef->iStartAmmo);
}

void GScr_WeaponMaxAmmo()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddInt(WeaponDef->iMaxAmmo);
}

void GScr_WeaponAltWeaponName()
{
    const char *String; // r31
    int WeaponIndexForName; // r30
    unsigned int altWeaponIndex; // r3
    WeaponDef *WeaponDef; // r31

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    altWeaponIndex = BG_GetWeaponDef(WeaponIndexForName)->altWeaponIndex;
    if (altWeaponIndex)
    {
        WeaponDef = BG_GetWeaponDef(altWeaponIndex);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 8515, 0, "%s", "altWeapDef");
        Scr_AddString(WeaponDef->szInternalName);
    }
    else
    {
        Scr_AddConstString(scr_const.none);
    }
}

WeaponDef *__cdecl GScr_GetWeaponDef()
{
    const char *String; // r3
    unsigned int WeaponIndexForName; // r31

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (!WeaponIndexForName)
        Scr_Error("unknown weapon");
    return BG_GetWeaponDef(WeaponIndexForName);
}

void GScr_WeaponFightDist()
{
    const char *String; // r3
    unsigned int WeaponIndexForName; // r31
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (!WeaponIndexForName)
        Scr_Error("unknown weapon");
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddFloat(WeaponDef->fightDist);
}

void GScr_WeaponMaxDist()
{
    const char *String; // r3
    unsigned int WeaponIndexForName; // r31
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (!WeaponIndexForName)
        Scr_Error("unknown weapon");
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    Scr_AddFloat(WeaponDef->maxDist);
}

void GScr_IsTurretActive()
{
    gentity_s *Entity; // r3

    if (Scr_GetNumParam() == 1)
    {
        Entity = Scr_GetEntity(0);
        if (Entity)
            Scr_AddBool(Entity->active);
        else
            Scr_Error("NULL turret passed to isturretactive");
    }
    else
    {
        Scr_Error("illegal call to isturretactive()\n");
    }
}

void GScr_IsWeaponClipOnly()
{
    const char *String; // r3
    int WeaponIndexForName; // r3
    int IsClipOnly; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (WeaponIndexForName)
    {
        IsClipOnly = BG_WeaponIsClipOnly(WeaponIndexForName);
        Scr_AddBool(IsClipOnly);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void GScr_IsWeaponDetonationTimed()
{
    const char *String; // r3
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3

    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    if (WeaponIndexForName)
    {
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        Scr_AddBool(WeaponDef->timedDetonation);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void GScr_GetMoveDelta()
{
    double v0; // fp31
    double v1; // fp29
    XAnimTree_s *v2; // r5
    unsigned int NumParam; // r3
    double Float; // fp1
    double v5; // fp1
    const XAnim_s *Anims; // r3
    scr_anim_s Anim; // [sp+50h] [-60h]
    float v8[2]; // [sp+58h] [-58h] BYREF
    float v9[14]; // [sp+60h] [-50h] BYREF

    v0 = 0.0;
    v1 = 1.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Float = Scr_GetFloat(2);
            v1 = Float;
            if (Float < 0.0 || Float > 1.0)
                Scr_ParamError(2u, "end time must be between 0 and 1");
        }
        v5 = Scr_GetFloat(1);
        v0 = v5;
        if (v5 < 0.0 || v5 > 1.0)
            Scr_ParamError(1u, "start time must be between 0 and 1");
    }
    Anim = Scr_GetAnim(0, 0);
    Anims = Scr_GetAnims(Anim.tree);
    XAnimGetRelDelta(Anims, Anim.index, v8, v9, v0, v1);
    Scr_AddVector(v9);
}

void GScr_GetAngleDelta()
{
    double v0; // fp31
    double v1; // fp29
    XAnimTree_s *v2; // r5
    unsigned int NumParam; // r3
    double Float; // fp1
    double v5; // fp1
    const XAnim_s *Anims; // r3
    double v7; // fp1
    scr_anim_s Anim; // [sp+50h] [-60h]
    float v9[2]; // [sp+58h] [-58h] BYREF
    float v10[14]; // [sp+60h] [-50h] BYREF

    v0 = 0.0;
    v1 = 1.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Float = Scr_GetFloat(2);
            v1 = Float;
            if (Float < 0.0 || Float > 1.0)
                Scr_ParamError(2u, "end time must be between 0 and 1");
        }
        v5 = Scr_GetFloat(1);
        v0 = v5;
        if (v5 < 0.0 || v5 > 1.0)
            Scr_ParamError(1u, "start time must be between 0 and 1");
    }
    Anim = Scr_GetAnim(0, 0);
    Anims = Scr_GetAnims(Anim.tree);
    XAnimGetRelDelta(Anims, Anim.index, v9, v10, v0, v1);
    v7 = RotationToYaw(v9);
    Scr_AddFloat(v7);
}

void GScr_GetNorthYaw()
{
    long double v0; // fp2
    char v1[40]; // [sp+50h] [-30h] BYREF

    SV_GetConfigstring(1147, v1, 32);
    v0 = atof(v1);
    Scr_AddFloat((float)*(double *)&v0);
}

// aislop
void __cdecl Scr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles)
{
    if (givenAxisCount > 2)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            8767,
            0,
            "givenAxisCount not in [0, 2]\n\t%i not in [%i, %i]",
            givenAxisCount,
            0,
            2);
    }

    if (givenAxisCount == 1)
    {
        vectoangles((const float *)axis, angles);
    }
    else if (givenAxisCount == 2)
    {
        // Orthogonalize the forward vector to the up vector
        float dot = (*axis)[6] * (*axis)[0]
            + (*axis)[7] * (*axis)[1]
            + (*axis)[8] * (*axis)[2];

        (*axis)[6] = (*axis)[6] - dot * (*axis)[0];
        (*axis)[7] = (*axis)[7] - dot * (*axis)[1];
        (*axis)[8] = (*axis)[8] - dot * (*axis)[2];

        // Normalize the orthogonalized forward vector
        float x = (*axis)[6];
        float y = (*axis)[7];
        float z = (*axis)[8];

        float length = sqrtf(x * x + y * y + z * z);
        if (length == 0.0f)
        {
            Scr_Error(va("forward and up vectors are the same direction or exact opposite directions"));
            return;
        }

        float invLength = 1.0f / length;
        (*axis)[6] = x * invLength;
        (*axis)[7] = y * invLength;
        (*axis)[8] = z * invLength;

        // Compute the right vector using cross product
        Vec3Cross(&(*axis)[6], (const float *)axis, &(*axis)[3]);

        // Convert final axis to angles
        AxisToAngles((const mat3x3 &)axis, angles);
    }
    else
    {
        angles[0] = 270.0f;
        angles[1] = 0.0f;
        angles[2] = 0.0f;
    }
}


void Scr_LoadFX()
{
    const char *String; // r31
    int v1; // r31

    String = Scr_GetString(0);
    if (!I_strncmp(String, "fx/", 3))
        Scr_ParamError(0, "effect name should start after the 'fx' folder.");
    v1 = G_EffectIndex(String);
    if (!v1 && !level.initializing)
        Scr_Error("loadFx must be called before any wait statements in the level script, or on an already loaded effect\n");
    Scr_AddInt(v1);
}

void __cdecl Scr_FxParamError(unsigned int paramIndex, const char *errorString, int fxId)
{
    const char *v6; // r3
    char v7[1056]; // [sp+50h] [-420h] BYREF

    if (!errorString)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 8822, 0, "%s", "errorString");
    if (fxId)
        SV_GetConfigstring(fxId + 2179, v7, 1024);
    else
        strcpy(v7, "not successfully loaded");
    v6 = va("%s (effect = %s)\n", errorString, v7);
    Scr_ParamError(paramIndex, v6);
}

// aislop
void Scr_PlayFX()
{
    int numParams = Scr_GetNumParam();
    if (numParams < 2 || numParams > 4)
    {
        Scr_Error("Incorrect number of parameters");
        return;
    }

    int effectId = Scr_GetInt(0);
    if (effectId <= 0 || effectId >= 100)
    {
        Scr_Error(va("Scr_PlayFX: invalid effect id %d", effectId));
        return;
    }

    float origin[3];
    Scr_GetVector(1u, origin);

    gentity_s *ent = G_TempEntity(origin, 59);
    if (ent->s.lerp.apos.trType != TR_STATIONARY)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            8870,
            1,
            "%s",
            "ent->s.lerp.apos.trType == TR_STATIONARY");
    }

    ent->s.eventParm = (unsigned char)effectId;

    if (numParams == 2)
    {
        ent->s.lerp.apos.trBase[0] = 270.0f;
        ent->s.lerp.apos.trBase[1] = 0.0f;
        ent->s.lerp.apos.trBase[2] = 0.0f;
    }
    else
    {
        float forward[3];
        Scr_GetVector(2u, forward);

        float forwardLen = sqrtf(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
        if (forwardLen == 0.0f)
        {
            Scr_FxParamError(2u, "playFx called with (0 0 0) forward direction", effectId);
            return;
        }

        float invForwardLen = 1.0f / forwardLen;
        forward[0] *= invForwardLen;
        forward[1] *= invForwardLen;
        forward[2] *= invForwardLen;

        if (numParams == 3)
        {
            vectoangles(forward, ent->s.lerp.apos.trBase);
        }
        else
        {
            float up[3];
            Scr_GetVector(3u, up);

            float upLen = sqrtf(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
            if (upLen == 0.0f)
            {
                Scr_FxParamError(3u, "playFx called with (0 0 0) up direction", effectId);
                return;
            }

            float invUpLen = 1.0f / upLen;
            up[0] *= invUpLen;
            up[1] *= invUpLen;
            up[2] *= invUpLen;

            float axis[3][3];
            axis[0][0] = forward[0];
            axis[0][1] = forward[1];
            axis[0][2] = forward[2];
            axis[1][0] = up[0];
            axis[1][1] = up[1];
            axis[1][2] = up[2];

            Scr_SetFxAngles(2u, (float(*)[3])axis, ent->s.lerp.apos.trBase);
        }
    }
}


void Scr_PlayFXOnTag()
{
    int Int; // r3
    int v1; // r30
    const char *v2; // r3
    gentity_s *Entity; // r29
    unsigned int ConstLowercaseString; // r31
    const char *v5; // r3
    unsigned int v6; // r3
    const char *v7; // r28
    const char *v8; // r3
    const char *v9; // r3
    const char *v10; // r3
    const char *v11; // r3
    int ConfigstringIndex; // r3
    unsigned int v13; // r31

    if (Scr_GetNumParam() != 3)
        Scr_Error("Incorrect number of parameters");
    Int = Scr_GetInt(0);
    v1 = Int;
    if (Int <= 0 || Int >= 100)
    {
        v2 = va("effect id %i is invalid\n", Int);
        Scr_ParamError(0, v2);
    }
    Entity = Scr_GetEntity(1);
    if (!Entity->model)
        Scr_ParamError(1u, "cannot play fx on entity with no model");
    ConstLowercaseString = Scr_GetConstLowercaseString(2);
    v5 = SL_ConvertToString(ConstLowercaseString);
    if (strchr(v5, 34))
        Scr_ParamError(2u, "cannot use \" characters in tag names\n");
    if (SV_DObjGetBoneIndex(Entity, ConstLowercaseString) < 0)
    {
        SV_DObjDumpInfo(Entity);
        v6 = G_ModelName(Entity->model);
        v7 = SL_ConvertToString(v6);
        v8 = SL_ConvertToString(ConstLowercaseString);
        v9 = va("tag '%s' does not exist on entity with model '%s'", v8, v7);
        Scr_ParamError(2u, v9);
    }
    v10 = SL_ConvertToString(ConstLowercaseString);
    v11 = va("%02d%s", v1, v10);
    ConfigstringIndex = G_FindConfigstringIndex(v11, 2279, 256, 1, 0);
    v13 = ConfigstringIndex;
    if (ConfigstringIndex <= 0 || ConfigstringIndex >= 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            8945,
            0,
            "%s",
            "csIndex > 0 && csIndex < MAX_EFFECT_TAGS");
    G_AddEvent(Entity, 60, v13);
}

// aislop
void Scr_PlayLoopedFX()
{
    unsigned int numParams = Scr_GetNumParam();
    if (numParams < 3 || numParams > 6)
    {
        Scr_Error("Incorrect number of parameters");
        return;
    }

    int fxId = Scr_GetInt(0);
    float repeatTimeMs = Scr_GetFloat(1) * 1000.0f;
    int repeat = (int)(repeatTimeMs + 0.5f);
    if (repeat <= 0)
    {
        Scr_FxParamError(1, "playLoopedFx called with repeat < 0.001 seconds", fxId);
        return;
    }

    float origin[3];
    Scr_GetVector(2, origin);

    int axisCount = 0;
    float axis[3][3] = { 0 };

    float cullDist = 0.0f;
    if (numParams >= 5)
    {
        Scr_GetVector(4, axis[0]);

        float forwardLen = sqrtf(axis[0][0] * axis[0][0] +
            axis[0][1] * axis[0][1] +
            axis[0][2] * axis[0][2]);

        if (forwardLen == 0.0f)
        {
            Scr_FxParamError(4, "playLoopedFx called with (0 0 0) forward direction", fxId);
            return;
        }

        float invLen = 1.0f / forwardLen;
        axis[0][0] *= invLen;
        axis[0][1] *= invLen;
        axis[0][2] *= invLen;

        axisCount++;
    }

    if (numParams == 6)
    {
        Scr_GetVector(5, axis[2]);

        float upLen = sqrtf(axis[2][0] * axis[2][0] +
            axis[2][1] * axis[2][1] +
            axis[2][2] * axis[2][2]);

        if (upLen == 0.0f)
        {
            Scr_FxParamError(5, "playLoopedFx called with (0 0 0) up direction", fxId);
            return;
        }

        float invUpLen = 1.0f / upLen;
        axis[2][0] *= invUpLen;
        axis[2][1] *= invUpLen;
        axis[2][2] *= invUpLen;

        axisCount++;
    }

    if (numParams >= 4)
    {
        cullDist = Scr_GetFloat(3);
    }

    gentity_s *ent = G_Spawn();
    ent->s.eType = ET_LOOP_FX; // originally 8 or 9, using symbolic name is clearer
    ent->s.un1.scale = fxId;

    if ((unsigned char)fxId != fxId)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            9009,
            0,
            "%s",
            "ent->s.un1.eventParm2 == fxId"
        );
    }

    G_SetOrigin(ent, origin);
    Scr_SetFxAngles(axisCount, axis, ent->s.lerp.apos.trBase);
    ent->s.lerp.u.turret.gunAngles[0] = cullDist;
    ent->s.lerp.u.loopFx.period = repeat;

    SV_LinkEntity(ent);
    Scr_AddEntity(ent);
}

// aislop
void Scr_SpawnFX()
{
    unsigned int numParams = Scr_GetNumParam();
    if (numParams < 2 || numParams > 4)
    {
        Scr_Error("Incorrect number of parameters");
        return;
    }

    int fxId = Scr_GetInt(0);
    int axisCount = 0;

    float axis[3][3] = { 0 };
    float origin[3];

    // Optional up vector (arg 3)
    if (numParams == 4)
    {
        Scr_GetVector(3, axis[2]);

        float upLen = sqrtf(axis[2][0] * axis[2][0] +
            axis[2][1] * axis[2][1] +
            axis[2][2] * axis[2][2]);

        if (upLen == 0.0f)
        {
            Scr_FxParamError(3, "spawnFx called with (0 0 0) up direction", fxId);
            return;
        }

        float invLen = 1.0f / upLen;
        axis[2][0] *= invLen;
        axis[2][1] *= invLen;
        axis[2][2] *= invLen;

        axisCount++;
    }

    // Required forward vector (arg 2)
    Scr_GetVector(2, axis[0]);

    float forwardLen = sqrtf(axis[0][0] * axis[0][0] +
        axis[0][1] * axis[0][1] +
        axis[0][2] * axis[0][2]);

    if (forwardLen == 0.0f)
    {
        Scr_FxParamError(2, "spawnFx called with (0 0 0) forward direction", fxId);
        return;
    }

    float invLen = 1.0f / forwardLen;
    axis[0][0] *= invLen;
    axis[0][1] *= invLen;
    axis[0][2] *= invLen;

    axisCount++;

    // Position vector (arg 1)
    Scr_GetVector(1, origin);

    gentity_s *ent = G_Spawn();
    ent->s.un1.scale = fxId;
    ent->s.eType = ET_FX;

    if ((unsigned char)fxId != fxId)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            9068,
            0,
            "ent->s.un1.eventParm2 == fxId\n\t%i, %i",
            (unsigned char)fxId,
            fxId
        );
    }

    G_SetOrigin(ent, origin);
    Scr_SetFxAngles(axisCount, axis, ent->s.lerp.apos.trBase);

    if (ent->s.time2)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            9071,
            1,
            "%s\n\t(ent->s.time2) = %i",
            "(ent->s.time2 == 0)",
            ent->s.time2
        );
    }

    SV_LinkEntity(ent);
    Scr_AddEntity(ent);
}



void Scr_TriggerFX()
{
    gentity_s *Entity; // r31
    long double v1; // fp2
    long double v2; // fp2

    if (!Scr_GetNumParam() || Scr_GetNumParam() > 2)
        Scr_Error("Incorrect number of parameters");
    Entity = Scr_GetEntity(0);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 9098, 0, "%s", "ent");
    if (Entity->s.eType != 7)
        Scr_ParamError(0, "entity wasn't created with 'newFx'");
    if (Scr_GetNumParam() == 2)
    {
        *(double *)&v1 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
        v2 = floor(v1);
        Entity->s.time2 = (int)(float)*(double *)&v2;
    }
    else
    {
        Entity->s.time2 = level.time;
    }
}

void Scr_GetFXVis()
{
    double Visibility; // fp1
    float v1[4]; // [sp+50h] [-30h] BYREF
    float v2[6]; // [sp+60h] [-20h] BYREF

    Scr_GetVector(0, v2);
    Scr_GetVector(1u, v1);
    Visibility = SV_FX_GetVisibility(v2, v1);
    Scr_AddFloat(Visibility);
}

void Scr_PhysicsExplosionSphere()
{
    gentity_s *v0; // r31
    __int64 v1; // r11
    double Float; // fp1
    float v3[6]; // [sp+58h] [-28h] BYREF

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, v3);
    v0 = G_TempEntity(v3, 61);
    v0->s.eventParm = Scr_GetInt(1);
    Float = Scr_GetFloat(2);
    v0->s.lerp.u.turret.gunAngles[0] = Float;
    if (Float < 0.0)
        Scr_ParamError(2u, "Radius is negative");
    LODWORD(v1) = v0->s.eventParm;
    if (v0->s.lerp.u.turret.gunAngles[0] > (double)(float)v1)
        Scr_Error("Inner radius is outside the outer radius");
    v0->s.lerp.u.turret.gunAngles[1] = Scr_GetFloat(3);
}

void Scr_PhysicsRadiusJolt()
{
    gentity_s *v0; // r31
    __int64 v1; // r11
    double Float; // fp1
    float *v3; // r31
    float v4[4]; // [sp+58h] [-28h] BYREF

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, v4);
    v0 = G_TempEntity(v4, 63);
    v0->s.eventParm = Scr_GetInt(1);
    Float = Scr_GetFloat(2);
    v0->s.lerp.u.turret.gunAngles[0] = Float;
    if (Float < 0.0)
        Scr_ParamError(2u, "Radius is negative");
    LODWORD(v1) = v0->s.eventParm;
    if (v0->s.lerp.u.turret.gunAngles[0] > (double)(float)v1)
        Scr_Error("Inner radius is outside the outer radius");
    v3 = &v0->s.lerp.u.turret.gunAngles[1];
    Scr_GetVector(3u, v3);
    if (*v3 == 0.0 && v3[1] == 0.0 && v3[2] == 0.0)
        *v3 = 1.1754944e-38;
}

void Scr_PhysicsRadiusJitter()
{
    gentity_s *v0; // r31
    __int64 v1; // r11
    double Float; // fp1
    double v3; // fp1
    double v4; // fp0
    float v5[6]; // [sp+58h] [-28h] BYREF

    if (Scr_GetNumParam() != 5)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, v5);
    v0 = G_TempEntity(v5, 64);
    v0->s.eventParm = Scr_GetInt(1);
    Float = Scr_GetFloat(2);
    v0->s.lerp.u.turret.gunAngles[0] = Float;
    if (Float < 0.0)
        Scr_ParamError(2u, "Radius is negative");
    LODWORD(v1) = v0->s.eventParm;
    if (v0->s.lerp.u.turret.gunAngles[0] > (double)(float)v1)
        Scr_Error("Inner radius is outside the outer radius");
    v0->s.lerp.u.turret.gunAngles[1] = Scr_GetFloat(3);
    v3 = Scr_GetFloat(4);
    v4 = v0->s.lerp.u.turret.gunAngles[1];
    v0->s.lerp.u.turret.gunAngles[2] = v3;
    if (v4 > v3)
        Scr_Error("Maximum jitter is less than minimum jitter");
}

void Scr_PhysicsExplosionCylinder()
{
    gentity_s *v0; // r31
    __int64 v1; // r11
    double Float; // fp1
    float v3[6]; // [sp+58h] [-28h] BYREF

    if (Scr_GetNumParam() != 4)
        Scr_Error("Incorrect number of parameters");
    Scr_GetVector(0, v3);
    v0 = G_TempEntity(v3, 62);
    v0->s.eventParm = Scr_GetInt(1);
    Float = Scr_GetFloat(2);
    v0->s.lerp.u.turret.gunAngles[0] = Float;
    if (Float < 0.0)
        Scr_ParamError(2u, "Radius is negative");
    LODWORD(v1) = v0->s.eventParm;
    if (v0->s.lerp.u.turret.gunAngles[0] > (double)(float)v1)
        Scr_Error("Inner radius is outside the outer radius");
    v0->s.lerp.u.turret.gunAngles[1] = Scr_GetFloat(3);
}

void __cdecl Scr_SetFog(const char *cmd, double start, double density, double r, double g, double b, double time)
{
    if (start < 0.0)
    {
        Scr_Error(va("%s: near distance must be >= 0", cmd));
    }
    if (r < 0.0 || r > 1.0 || g < 0.0 || g > 1.0 || b < 0.0 || b > 1.0)
    {
        Scr_Error(va("%s: red/green/blue color components must be in the range [0, 1]", cmd));
    }
    if (time < 0.0)
    {
        Scr_Error(va("%s: transition time must be >= 0 seconds", cmd));
    }
    if (level.loading != LOADING_SAVEGAME)
    {
        G_setfog(va(
            "%g %g %g %g %g %.0f",
            start,
            density,
            r,
            g,
            b,
            time * 1000.0f));
    }
}

void Scr_SetExponentialFog()
{
    double startDist; // fp24
    double halfwayDist; // fp30
    double density; // fp31
    double v3; // fp29
    double v4; // fp28
    double v5; // fp27
    double v6; // fp23

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
    density = (float)((float)0.69314718 / (float)halfwayDist);
    v3 = Scr_GetFloat(2);
    v4 = Scr_GetFloat(3);
    v5 = Scr_GetFloat(4);
    v6 = Scr_GetFloat(5);
    Dvar_SetColor((dvar_s*)g_fogColorReadOnly, v3, v4, v5, 1.0);
    Dvar_SetFloat((dvar_s*)g_fogStartDistReadOnly, startDist);
    Dvar_SetFloat((dvar_s*)g_fogHalfDistReadOnly, halfwayDist);

    iassert(density > 0.0f && density <= 1.0f);
    
    Scr_SetFog("setExpFog", startDist, density, v3, v4, v5, v6);
}

void Scr_VisionSetNaked()
{
    unsigned int NumParam; // r3
    long double v1; // fp2
    long double v2; // fp2
    const char *String; // r3
    const char *v4; // r3
    int v5; // [sp+50h] [-10h]

    v5 = 1000;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error("USAGE: VisionSetNaked( <visionset name>, <transition time> )\n");
            return;
        }
        *(double *)&v1 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
        v2 = floor(v1);
        v5 = (int)(float)*(double *)&v2;
    }
    String = Scr_GetString(0);
    v4 = va("\"%s\" %i", String, v5);
    SV_SetConfigstring(1149, v4);
}

void Scr_VisionSetNight()
{
    unsigned int NumParam; // r3
    long double v1; // fp2
    long double v2; // fp2
    const char *String; // r3
    const char *v4; // r3
    int v5; // [sp+50h] [-10h]

    v5 = 1000;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error("USAGE: VisionSetNight( <visionset name>, <transition time> )\n");
            return;
        }
        *(double *)&v1 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
        v2 = floor(v1);
        v5 = (int)(float)*(double *)&v2;
    }
    String = Scr_GetString(0);
    v4 = va("\"%s\" %i", String, v5);
    SV_SetConfigstring(1150, v4);
}

void Scr_SetCullDist()
{
    const char *v0; // r3
    double Float; // [sp+18h] [-48h]

    if (Scr_GetNumParam() != 1)
        Scr_Error("Incorrect number of parameters\n");
    Float = Scr_GetFloat(0);
    v0 = va("%g", Float);
    SV_SetConfigstring(6, v0);
}

void Scr_GetMapSunLight()
{
    if (Scr_GetNumParam())
        Scr_Error("Incorrect number of parameters\n");
    Scr_MakeArray();
    Scr_AddFloat(level.mapSunColor[0]);
    Scr_AddArray();
    Scr_AddFloat(level.mapSunColor[1]);
    Scr_AddArray();
    Scr_AddFloat(level.mapSunColor[2]);
    Scr_AddArray();
}

void Scr_SetSunLight()
{
    if (Scr_GetNumParam() != 3)
        Scr_Error("Incorrect number of parameters\n");

    float f2 = Scr_GetFloat(2);
    float f1 = Scr_GetFloat(1);
    float f0 = Scr_GetFloat(0);

    SV_SetConfigstring(7, va("%g %g %g", f0, f1, f2));
}

void Scr_ResetSunLight()
{
    if (Scr_GetNumParam())
        Scr_Error("Incorrect number of parameters\n");
    SV_SetConfigstring(7, "");
}

void Scr_GetMapSunDirection()
{
    if (Scr_GetNumParam())
        Scr_Error("Incorrect number of parameters\n");
    Scr_AddVector(level.mapSunDirection);
}

// aislop
void Scr_SetSunDirection()
{
    // Ensure exactly one parameter is passed
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("Incorrect number of parameters\n");
        return;
    }

    float sunDir[3];
    Scr_GetVector(0, sunDir);

    // Normalize the vector
    float len = sqrtf(sunDir[0] * sunDir[0] + sunDir[1] * sunDir[1] + sunDir[2] * sunDir[2]);

    if (len == 0.0f)
    {
        Scr_Error("Sun direction cannot be a zero vector\n");
        return;
    }

    float invLen = 1.0f / len;
    sunDir[0] *= invLen;
    sunDir[1] *= invLen;
    sunDir[2] *= invLen;

    // Format and store sun direction as a config string
    const char *sunDirStr = va("%f %f %f", sunDir[0], sunDir[1], sunDir[2]);
    SV_SetConfigstring(8, sunDirStr);
}

// aislop
void Scr_LerpSunDirection()
{
    if (Scr_GetNumParam() != 3)
    {
        Scr_Error("Incorrect number of parameters\n");
        return;
    }

    float sunDirBegin[3];
    float sunDirEnd[3];

    Scr_GetVector(0, sunDirBegin);
    Scr_GetVector(1, sunDirEnd);
    float lerpTime = Scr_GetFloat(2);

    // Validate and normalize start direction
    float lenBegin = sqrtf(sunDirBegin[0] * sunDirBegin[0] +
        sunDirBegin[1] * sunDirBegin[1] +
        sunDirBegin[2] * sunDirBegin[2]);

    if (lenBegin == 0.0f)
    {
        Scr_Error("Start sun direction cannot be a zero vector\n");
        return;
    }

    float invLenBegin = 1.0f / lenBegin;
    sunDirBegin[0] *= invLenBegin;
    sunDirBegin[1] *= invLenBegin;
    sunDirBegin[2] *= invLenBegin;

    // Validate and normalize end direction
    float lenEnd = sqrtf(sunDirEnd[0] * sunDirEnd[0] +
        sunDirEnd[1] * sunDirEnd[1] +
        sunDirEnd[2] * sunDirEnd[2]);

    if (lenEnd == 0.0f)
    {
        Scr_Error("End sun direction cannot be a zero vector\n");
        return;
    }

    float invLenEnd = 1.0f / lenEnd;
    sunDirEnd[0] *= invLenEnd;
    sunDirEnd[1] *= invLenEnd;
    sunDirEnd[2] *= invLenEnd;

    if (lerpTime <= 0.001f)
    {
        Scr_Error("Lerp time must be greater than 1 ms\n");
        return;
    }

    int lerpBeginTime = level.time;
    int lerpEndTime = lerpBeginTime + (int)(lerpTime * 1000.0f);

    const char *configStr = va(
        "%f %f %f %f %f %f %d %d",
        sunDirBegin[0], sunDirBegin[1], sunDirBegin[2],
        sunDirEnd[0], sunDirEnd[1], sunDirEnd[2],
        lerpBeginTime, lerpEndTime);

    SV_SetConfigstring(8, configStr);
}


void Scr_ResetSunDirection()
{
    if (Scr_GetNumParam())
        Scr_Error("Incorrect number of parameters\n");
    SV_SetConfigstring(8, "");
}

void Scr_BadPlace_Delete()
{
    unsigned int ConstString; // r3

    if (Scr_GetNumParam() == 1)
    {
        if (*Scr_GetString(0))
        {
            ConstString = Scr_GetConstString(0);
            Path_RemoveBadPlace(ConstString);
        }
        else
        {
            Scr_Error("badplace_delete called with name \"\"");
        }
    }
    else
    {
        Scr_Error("Incorrect number of parameters\n");
    }
}

void Scr_BadPlace_Cylinder()
{
    unsigned int ConstString; // r31
    long double v1; // fp2
    long double v2; // fp2
    double Float; // fp1
    int v4; // r8
    int v5; // r7
    int TeamFlags; // r5
    int v7; // [sp+50h] [-40h]
    badplace_arc_t v8; // [sp+60h] [-30h] BYREF

    if (Scr_GetNumParam() >= 5)
    {
        if (*Scr_GetString(0))
            ConstString = Scr_GetConstString(0);
        else
            ConstString = 0;
        *(double *)&v1 = (float)(Scr_GetFloat(1) * (float)1000.0);
        v2 = ceil(v1);
        v7 = (int)(float)*(double *)&v2;
        Scr_GetVector(2u, v8.origin);
        v8.radius = Scr_GetFloat(3);
        Float = Scr_GetFloat(4);
        v8.angle0 = 0.0;
        v8.angle1 = 360.0;
        v8.halfheight = (float)Float * (float)0.5;
        TeamFlags = Scr_GetTeamFlags(5u, "badplace_cylinder");
        if (!TeamFlags)
            TeamFlags = 14;
        Path_MakeArcBadPlace(ConstString, v7, TeamFlags, &v8);
    }
    else
    {
        Scr_Error("Incorrect badplace_cylinder() call.\n");
    }
}

void Scr_BadPlace_Arc()
{
    unsigned int ConstString; // r31
    long double v1; // fp2
    long double v2; // fp2
    double v3; // fp31
    double Float; // fp1
    double angle1; // fp0
    int v6; // r8
    int v7; // r7
    int TeamFlags; // r5
    int v9; // [sp+50h] [-60h]
    float v10[6]; // [sp+58h] [-58h] BYREF
    badplace_arc_t v11; // [sp+70h] [-40h] BYREF

    if (Scr_GetNumParam() >= 8)
    {
        if (*Scr_GetString(0))
            ConstString = Scr_GetConstString(0);
        else
            ConstString = 0;
        *(double *)&v1 = (float)(Scr_GetFloat(1) * (float)1000.0);
        v2 = ceil(v1);
        v9 = (int)(float)*(double *)&v2;
        Scr_GetVector(2u, v11.origin);
        v11.radius = Scr_GetFloat(3);
        v11.halfheight = Scr_GetFloat(4) * (float)0.5;
        Scr_GetVector(5u, v10);
        v3 = vectoyaw(v10);
        v11.angle0 = (float)v3 - Scr_GetFloat(6);
        if (v11.angle0 > v3)
            Scr_Error("left angle < 0 in badplace_arc\n");
        Float = Scr_GetFloat(7);
        angle1 = (float)((float)Float + (float)v3);
        v11.angle1 = (float)Float + (float)v3;
        if (angle1 < v3)
        {
            Scr_Error("right angle < 0 in badplace_arc\n");
            angle1 = v11.angle1;
        }
        if ((float)((float)angle1 - v11.angle0) < 360.0)
        {
            v11.angle0 = AngleNormalize360(v11.angle0);
            v11.angle1 = AngleNormalize360(v11.angle1);
        }
        else
        {
            v11.angle1 = 360.0;
            v11.angle0 = 0.0;
        }
        TeamFlags = Scr_GetTeamFlags(8u, "badplace_arc");
        if (!TeamFlags)
            TeamFlags = 14;
        Path_MakeArcBadPlace(ConstString, v9, TeamFlags, &v11);
    }
    else
    {
        Scr_Error("Incorrect usage for badplace_arc()\n");
    }
}

void Scr_BadPlace_Brush()
{
    unsigned int ConstString; // r30
    long double v1; // fp2
    long double v2; // fp2
    gentity_s *Entity; // r31
    int TeamFlags; // r5
    int v5; // [sp+50h] [-20h]

    if (Scr_GetNumParam() >= 3)
    {
        if (*Scr_GetString(0))
            ConstString = Scr_GetConstString(0);
        else
            ConstString = 0;
        *(double *)&v1 = (float)(Scr_GetFloat(1) * (float)1000.0);
        v2 = ceil(v1);
        v5 = (int)(float)*(double *)&v2;
        Entity = Scr_GetEntity(2);
        TeamFlags = Scr_GetTeamFlags(3u, "badplace_brush");
        if (!TeamFlags)
            TeamFlags = 14;
        Path_MakeBrushBadPlace(ConstString, v5, TeamFlags, Entity);
    }
    else
    {
        Scr_Error("Incorrect badplace_cylinder() call.\n");
    }
}

void Scr_ClearAllCorpses()
{
    G_RemoveActorCorpses(0);
}

void __cdecl GScr_GetNumParts()
{
    const XModel *v1; // r3
    int v2; // r3

    if (Scr_GetNumParam() != 1)
        Scr_Error("Incorrect use of getnumparts()");
    v1 = SV_XModelGet((char*)Scr_GetString(0));
    v2 = XModelNumBones(v1);
    Scr_AddInt(v2);
}

void __cdecl GScr_GetPartName()
{
    XModel *v1; // r31
    unsigned int Int; // r30
    unsigned int v3; // r3
    const char *v4; // r3
    unsigned __int16 *v5; // r3
    unsigned int v6; // r31

    if (Scr_GetNumParam() != 2)
        Scr_Error("Incorrect usage for getpartname()");
    v1 = SV_XModelGet((char*)Scr_GetString(0));
    Int = Scr_GetInt(1);
    v3 = XModelNumBones(v1);
    if (Int >= v3)
    {
        v4 = va("index out of range (0 - %d)", v3 - 1);
        Scr_ParamError(1u, v4);
    }
    v5 = XModelBoneNames(v1);
    v6 = v5[Int];
    if (!v5[Int])
        Scr_ParamError(0, "bad model");
    Scr_AddConstString(v6);
}

XAnimTree_s *__cdecl GScr_GetEntAnimTree(gentity_s *ent)
{
    XAnimTree_s *EntAnimTree; // r30
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    const char *EntityTypeName; // r3
    const char *v7; // r3

    EntAnimTree = G_GetEntAnimTree(ent);
    if (!EntAnimTree)
    {
        v3 = ent->r.currentOrigin[2];
        v4 = ent->r.currentOrigin[1];
        v5 = ent->r.currentOrigin[0];
        SL_ConvertToString(ent->classname);
        EntityTypeName = G_GetEntityTypeName(ent);
        v7 = va(
            "entity of type '%s', classname '%s', origin (%f, %f, %f) does not have an animation tree",
            EntityTypeName,
            (const char *)HIDWORD(v5),
            v5,
            v4,
            v3);
        Scr_Error(v7);
    }
    return EntAnimTree;
}

void __cdecl G_FlagAnimForUpdate(gentity_s *ent)
{
    if ((ent->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
        ent->flags |= FL_REPEAT_ANIM_UPDATE;
}

void __cdecl Scr_AnimRelative(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    XAnimTree_s *EntAnimTree; // r30
    __int16 v3; // r25
    unsigned __int16 v4; // r26
    unsigned int v5; // r29
    XAnimTree_s *v6; // r5
    unsigned int ConstString; // r3
    const char *v8; // r3
    const char *v9; // r3
    unsigned int v10; // r3
    actor_s *actor; // r11
    unsigned int v12; // r28
    const char *v13; // r30
    const char *v14; // r22
    double v15; // fp31
    double v16; // fp30
    double v17; // fp29
    const char *v18; // r3
    actor_s *v19; // r11
    scr_anim_s Anim; // [sp+50h] [-A0h]
    scr_anim_s v21; // [sp+50h] [-A0h]
    float v22[4]; // [sp+58h] [-98h] BYREF
    float v23[6]; // [sp+68h] [-88h] BYREF

    Entity = GetEntity(entref);
    if (!Com_GetServerDObj(Entity->s.number))
        Scr_ObjectError("No model exists.");
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    if (!EntAnimTree)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10119, 0, "%s", "pAnimTree");
    v3 = 0;
    v4 = 0;
    v5 = 0;
    if (Scr_GetNumParam() > 4)
    {
        ConstString = Scr_GetConstString(4);
        v5 = ConstString;
        if (ConstString != scr_const.normal && ConstString != scr_const.deathplant)
        {
            v8 = SL_ConvertToString(ConstString);
            v9 = va("Illegal mode %s for animScripted. Valid modes are normal and deathplant", v8);
            Scr_Error(v9);
        }
        if (Scr_GetNumParam() > 5)
        {
            Anim = Scr_GetAnim(5, EntAnimTree);
            v3 = Anim.tree;
            v4 = Anim.index;
        }
    }
    v21 = Scr_GetAnim(3, EntAnimTree);
    Scr_GetVector(2u, v22);
    Scr_GetVector(1u, v23);
    v10 = Scr_GetConstString(0);
    actor = Entity->actor;
    v12 = v10;
    if (actor && !actor->Physics.bIsAlive)
    {
        if (Entity->targetname)
            v13 = SL_ConvertToString(Entity->targetname);
        else
            v13 = "<undefined>";
        v14 = SL_ConvertToString(Entity->classname);
        v15 = Entity->r.currentOrigin[2];
        v16 = Entity->r.currentOrigin[1];
        v17 = Entity->r.currentOrigin[0];
        Sentient_NameForTeam(Entity->sentient->eTeam);
        v18 = va(
            "tried to play a scripted animation on a dead AI; entity %i team %s origin %g %g %g targetname %s classname %s\n",
            Entity->s.number,
            HIDWORD(v17),
            HIDWORD(v16),
            v15,
            v13,
            v14);
        Scr_Error(v18);
    }
    if (!v3 && v4)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10147, 0, "%s", "root.tree || !root.index");
    G_Animscripted(Entity, v23, v22, v21.index, v4, v12, scr_const.deathplant == v5);
    v19 = Entity->actor;
    if (v19)
    {
        v19->eScriptSetAnimMode = AI_ANIM_POINT_RELATIVE;
        Entity->actor->eAnimMode = AI_ANIM_POINT_RELATIVE;
    }
}

void __cdecl ScrCmd_animrelative(scr_entref_t entref)
{
    if (Scr_GetNumParam() > 6)
        Scr_Error("too many parameters");
    if (Scr_GetNumParam() > 6 || Scr_GetNumParam() < 4)
        Scr_Error("Incorrect number of parameters for ScrCmd_animrelative command");
    Scr_AnimRelative(entref);
}

void __cdecl DumpAnimCommand(
    const char *funcName,
    XAnimTree_s *tree,
    unsigned int anim,
    int root,
    double weight,
    double time,
    double rate)
{
    const XAnim_s *Anims; // r29
    unsigned int v12; // r4
    const char *v13; // r30
    int v14; // [sp+60h] [-50h] BYREF
    const char *v15; // [sp+64h] [-4Ch] BYREF

    Anims = XAnimGetAnims(tree);
    Scr_GetLastScriptPlace(&v14, &v15);
    v12 = anim;
    v13 = (const char *)level.time;
    XAnimGetAnimDebugName(Anims, v12);
    Com_Printf(
        19,
        "^3%s  ^7weight=^5%.2f ^7time=^5%.2f ^7rate=^5%.2f   ^7level time:%d  %s:%d   %s\n",
        (const char *)HIDWORD(weight),
        weight,
        time,
        rate,
        HIDWORD(time),
        (const char *)HIDWORD(rate),
        LODWORD(rate),
        v13);
}

void __cdecl GScr_ClearAnim(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    XAnimTree_s *EntAnimTree; // r31
    XAnimTree_s *v3; // r5
    unsigned int v4; // r29
    double Float; // fp1
    double v6; // fp31

    Entity = GetEntity(entref);
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    //v4 = (unsigned int)Scr_GetAnim(0, EntAnimTree) >> 16;
    v4 = (unsigned int)Scr_GetAnim(0, EntAnimTree).index;
    Float = Scr_GetFloat(1);
    v6 = Float;
    if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        DumpAnimCommand("ClearAnim", EntAnimTree, v4, -1, 0.0, Float, 1.0);
    XAnimClearTreeGoalWeights(EntAnimTree, v4, v6);
}

void __cdecl GScr_HandleAnimError(int error)
{
    if (error == 1)
    {
        Scr_Error("root anim is not an ancestor of the anim");
    LABEL_4:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            10255,
            0,
            "%s",
            "error == XANIM_ERROR_BAD_NOTIFY");
        goto LABEL_5;
    }
    if (error != 2)
        goto LABEL_4;
LABEL_5:
    Scr_Error("cannot flag anim since it has 0 effective goal weight");
}

void __cdecl GScr_SetAnimKnobInternal(scr_entref_t entref, unsigned int flags)
{
    gentity_s *pSelf; // r28
    double rate; // fp30
    double goalWeight; // fp31
    double goalTime; // fp29
    XAnimTree_s *tree; // r31
    unsigned int anim; // r30
    const char *funcName; // r3
    DObj_s *obj; // r31
    int error; // r3

    pSelf = GetEntity(entref);
    rate = 1.0;
    goalWeight = 1.0;
    goalTime = 0.2;
    tree = GScr_GetEntAnimTree(pSelf);

    switch (Scr_GetNumParam())
    {
    case 1:
        break;
    case 2:
        goalWeight = Scr_GetFloat(1);
        if (goalWeight < 0.0)
            Scr_ParamError(1, "must set nonnegative weight");
        break;
    case 3:
        goalTime = Scr_GetFloat(2);
        if (goalTime < 0.0)
            Scr_ParamError(2, "must set nonnegative goal time");
        break;
    case 4:
        rate = Scr_GetFloat(3);
        if (rate < 0.0)
            Scr_ParamError(3, "must set nonnegative rate");
        break;
    default:
        Scr_Error("too many parameters");
        break;
    }

    anim = (unsigned int)Scr_GetAnim(0, tree).index;

    if (g_dumpAnimsCommands->current.integer == pSelf->s.number)
    {
        switch (flags)
        {
        case 1:
            funcName = "SetAnimKnob";
            break;
        case 2:
            funcName = "SetAnimKnobLimitedRestart";
            break;
        case 3:
            funcName = "SetAnimKnobRestart";
            break;
        default:
            iassert(flags == 0);
            funcName = "SetAnimKnobLimited";
            break;
        }
        DumpAnimCommand(funcName, tree, anim, -1, goalWeight, goalTime, rate);
    }

    // KISAKTODO: blops has some interesting extra cases here involving goalWeight < 0.001

    obj = Com_GetServerDObj(pSelf->s.number);
    if (!obj)
        Scr_ObjectError("No model exists.");

    if ((flags & 1) != 0)
        error = XAnimSetCompleteGoalWeightKnob(obj, anim, goalWeight, goalTime, rate, 0, (flags & 2) != 0);
    else
        error = XAnimSetGoalWeightKnob(obj, anim, goalWeight, goalTime, rate, 0, (flags & 2) != 0);

    if (error)
    {
        GScr_HandleAnimError(error);
    }
    else
    {
        if ((pSelf->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
            pSelf->flags |= FL_REPEAT_ANIM_UPDATE;
    }
}

void __cdecl GScr_SetAnimKnob(scr_entref_t entref)
{
    GScr_SetAnimKnobInternal(entref, 1u);
}

void __cdecl GScr_SetAnimKnobLimited(scr_entref_t entref)
{
    GScr_SetAnimKnobInternal(entref, 0);
}

void __cdecl GScr_SetAnimKnobRestart(scr_entref_t entref)
{
    GScr_SetAnimKnobInternal(entref, 3u);
}

void __cdecl GScr_SetAnimKnobLimitedRestart(scr_entref_t entref)
{
    GScr_SetAnimKnobInternal(entref, 2u);
}

void __cdecl GScr_SetAnimKnobAllInternal(scr_entref_t entref, unsigned int flags)
{
    gentity_s *Entity; // r27
    double Float; // fp30
    double v5; // fp31
    double v6; // fp29
    XAnimTree_s *EntAnimTree; // r31
    XAnimTree_s *v8; // r5
    XAnimTree_s *v9; // r5
    const char *v10; // r3
    DObj_s *ServerDObj; // r31
    int v12; // r7
    unsigned int v13; // r6
    int v14; // r3
    int v15; // r11
    scr_anim_s rootanim; // [sp+50h] [-60h]
    scr_anim_s anim; // [sp+54h] [-5Ch]

    Entity = GetEntity(entref);
    Float = 1.0;
    v5 = 1.0;
    v6 = 0.2;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_9;
    case 3u:
        goto LABEL_7;
    case 4u:
        goto LABEL_5;
    case 5u:
        goto LABEL_3;
    default:
        Scr_Error("incorrect number of parameters");
    LABEL_3:
        Float = Scr_GetFloat(4);
        if (Float < 0.0)
            Scr_ParamError(4u, "must set nonnegative rate");
    LABEL_5:
        v6 = Scr_GetFloat(3);
        if (v6 < 0.0)
            Scr_ParamError(3u, "must set nonnegative goal time");
    LABEL_7:
        v5 = Scr_GetFloat(2);
        if (v5 < 0.0)
            Scr_ParamError(2u, "must set nonnegative weight");
    LABEL_9:
        anim = Scr_GetAnim(1, EntAnimTree);
        rootanim = Scr_GetAnim(0, EntAnimTree);

        if (anim.tree != rootanim.tree)
            Scr_Error("root anim is not in the same anim tree");
        if (anim.index == rootanim.index)
            Scr_Error("root anim is not an ancestor of the anim");
        if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        {
            switch (flags)
            {
            case 1u:
                v10 = "SetAnimKnobAll";
                break;
            case 2u:
                v10 = "SetAnimKnobAllLimitedRestart";
                break;
            case 3u:
                v10 = "SetAnimKnobAllRestart";
                break;
            default:
                if (flags)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10527, 0, "%s", "flags == 0");
                v10 = "SetAnimKnobAllLimited";
                break;
            }
            DumpAnimCommand(v10, EntAnimTree, rootanim.index, anim.index, v5, v6, Float);
        }
        ServerDObj = Com_GetServerDObj(Entity->s.number);
        if (!ServerDObj)
            Scr_ObjectError("No model exists.");
        if ((flags & 1) != 0)
            v14 = XAnimSetCompleteGoalWeightKnobAll(ServerDObj, rootanim.index, anim.index, v5, v6, Float, v13, v12);
        else
            v14 = XAnimSetGoalWeightKnobAll(ServerDObj, rootanim.index, anim.index, v5, v6, Float, v13, v12);
        if (v14)
        {
            GScr_HandleAnimError(v14);
        }
        else
        {
            if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                Entity->flags |= FL_REPEAT_ANIM_UPDATE;
        }
        return;
    }
}

void __cdecl GScr_SetAnimKnobAll(scr_entref_t entref)
{
    GScr_SetAnimKnobAllInternal(entref, 1u);
}

void __cdecl GScr_SetAnimKnobAllLimited(scr_entref_t entref)
{
    GScr_SetAnimKnobAllInternal(entref, 0);
}

void __cdecl GScr_SetAnimKnobAllRestart(scr_entref_t entref)
{
    GScr_SetAnimKnobAllInternal(entref, 3u);
}

void __cdecl GScr_SetAnimKnobAllLimitedRestart(scr_entref_t entref)
{
    GScr_SetAnimKnobAllInternal(entref, 2u);
}

void __cdecl GScr_SetAnimInternal(scr_entref_t entref, unsigned int flags)
{
    gentity_s *Entity; // r28
    double Float; // fp30
    double v5; // fp31
    double v6; // fp29
    XAnimTree_s *EntAnimTree; // r31
    XAnimTree_s *v8; // r5
    unsigned __int16 v9; // r30
    const char *v10; // r3
    DObj_s *ServerDObj; // r31
    int v12; // r7
    unsigned int v13; // r6
    unsigned int v14; // r5
    int v15; // r3
    int v16; // r11

    Entity = GetEntity(entref);
    Float = 1.0;
    v5 = 1.0;
    v6 = 0.2;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    switch (Scr_GetNumParam())
    {
    case 1u:
        goto LABEL_9;
    case 2u:
        goto LABEL_7;
    case 3u:
        goto LABEL_5;
    case 4u:
        goto LABEL_3;
    default:
        Scr_Error("too many parameters");
    LABEL_3:
        Float = Scr_GetFloat(3);
        if (Float < 0.0)
            Scr_ParamError(3u, "must set nonnegative rate");
    LABEL_5:
        v6 = Scr_GetFloat(2);
        if (v6 < 0.0)
            Scr_ParamError(2u, "must set nonnegative goal time");
    LABEL_7:
        v5 = Scr_GetFloat(1);
        if (v5 < 0.0)
            Scr_ParamError(1u, "must set nonnegative weight");
    LABEL_9:
        //v9 = Scr_GetAnim(0, EntAnimTree) >> 16;
        v9 = Scr_GetAnim(0, EntAnimTree).index;
        if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        {
            switch (flags)
            {
            case 1u:
                v10 = "SetAnim";
                break;
            case 2u:
                v10 = "SetAnimLimitedRestart";
                break;
            case 3u:
                v10 = "SetAnimRestart";
                break;
            default:
                if (flags)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10712, 0, "%s", "flags == 0");
                v10 = "SetAnimLimited";
                break;
            }
            DumpAnimCommand(v10, EntAnimTree, v9, -1, v5, v6, Float);
        }
        ServerDObj = Com_GetServerDObj(Entity->s.number);
        if (!ServerDObj)
            Scr_ObjectError("No model exists.");
        if ((flags & 1) != 0)
            v15 = XAnimSetCompleteGoalWeight(ServerDObj, v9, v5, v6, Float, v14, v13, v12);
        else
            v15 = XAnimSetGoalWeight(ServerDObj, v9, v5, v6, Float, v14, v13, v12);
        if (v15)
        {
            GScr_HandleAnimError(v15);
        }
        else
        {
            if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                Entity->flags |= FL_REPEAT_ANIM_UPDATE;
        }
        return;
    }
}

void __cdecl GScr_SetAnim(scr_entref_t entref)
{
    GScr_SetAnimInternal(entref, 1u);
}

void __cdecl GScr_SetAnimLimited(scr_entref_t entref)
{
    GScr_SetAnimInternal(entref, 0);
}

void __cdecl GScr_SetAnimRestart(scr_entref_t entref)
{
    GScr_SetAnimInternal(entref, 3u);
}

void __cdecl GScr_SetAnimLimitedRestart(scr_entref_t entref)
{
    GScr_SetAnimInternal(entref, 2u);
}

void __cdecl GScr_GetAnimTime(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    XAnimTree_s *EntAnimTree; // r31
    XAnimTree_s *v3; // r5
    unsigned int v4; // r30
    const XAnim_s *Anims; // r3
    double Time; // fp1

    Entity = GetEntity(entref);
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    //v4 = (unsigned int)Scr_GetAnim(0, (unsigned int)EntAnimTree, v3) >> 16;
    v4 = Scr_GetAnim(0, EntAnimTree).index;
    Anims = XAnimGetAnims(EntAnimTree);
    if (!XAnimHasTime(Anims, v4))
        Scr_ParamError(0, "blended nonsynchronized animation has no concept of time");
    Time = XAnimGetTime(EntAnimTree, v4);
    Scr_AddFloat(Time);
}

void __cdecl GScr_GetAnimAssetType(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    XAnimTree_s *EntAnimTree; // r31
    XAnimTree_s *v3; // r5
    unsigned __int8 AssetType; // r3
    scr_anim_s Anim; // [sp+50h] [-20h]

    Entity = GetEntity(entref);
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    Anim = Scr_GetAnim(0, EntAnimTree);
    AssetType = XAnimGetAssetType(EntAnimTree, Anim.index);
    Scr_AddInt(AssetType);
}

void __cdecl GScr_SetFlaggedAnimKnobInternal(scr_entref_t entref, unsigned int flags)
{
    gentity_s *Entity; // r25
    double Float; // fp30
    double v5; // fp31
    double v6; // fp29
    XAnimTree_s *EntAnimTree; // r27
    XAnimTree_s *v8; // r5
    unsigned int v9; // r29
    const XAnim_s *Anims; // r3
    const char *v11; // r3
    DObj_s *ServerDObj; // r31
    int v13; // r6
    unsigned int v14; // r5
    int v15; // r3
    int v16; // r11

    Entity = GetEntity(entref);
    Float = 1.0;
    v5 = 1.0;
    v6 = 0.2;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_9;
    case 3u:
        goto LABEL_7;
    case 4u:
        goto LABEL_5;
    case 5u:
        goto LABEL_3;
    default:
        Scr_Error("too many parameters");
    LABEL_3:
        Float = Scr_GetFloat(4);
        if (Float < 0.0)
            Scr_ParamError(4u, "must set nonnegative rate");
    LABEL_5:
        v6 = Scr_GetFloat(3);
        if (v6 < 0.0)
            Scr_ParamError(3u, "must set nonnegative goal time");
    LABEL_7:
        v5 = Scr_GetFloat(2);
        if (v5 <= 0.0)
            Scr_ParamError(2u, "must set positive weight");
    LABEL_9:
        //v9 = (unsigned int)Scr_GetAnim((scr_anim_s *)1, (unsigned int)EntAnimTree, v8) >> 16;
        v9 = Scr_GetAnim(1, EntAnimTree).index;
        if (!Scr_GetConstString(0))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10930, 0, "%s", "notifyName");
        Anims = XAnimGetAnims(EntAnimTree);
        if (!XAnimHasTime(Anims, v9))
            Scr_ParamError(1u, "blended nonsynchronized animation has no concept of time");
        if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        {
            switch (flags)
            {
            case 1u:
                v11 = "SetFlaggedAnimKnob";
                break;
            case 2u:
                v11 = "SetFlaggedAnimKnobLimitedRestart";
                break;
            case 3u:
                v11 = "SetFlaggedAnimKnobRestart";
                break;
            default:
                if (flags)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10959, 0, "%s", "flags == 0");
                v11 = "SetFlaggedAnimKnobLimited";
                break;
            }
            DumpAnimCommand(v11, EntAnimTree, v9, -1, v5, v6, Float);
        }
        ServerDObj = Com_GetServerDObj(Entity->s.number);
        if (!ServerDObj)
            Scr_ObjectError("No model exists.");
        if ((flags & 1) != 0)
            v15 = XAnimSetCompleteGoalWeightKnob(ServerDObj, v9, v5, v6, Float, v14, v13);
        else
            v15 = XAnimSetGoalWeightKnob(ServerDObj, v9, v5, v6, Float, v14, v13);
        if (v15)
        {
            GScr_HandleAnimError(v15);
        }
        else
        {
            if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                Entity->flags |= FL_REPEAT_ANIM_UPDATE;
        }
        return;
    }
}

void __cdecl GScr_SetFlaggedAnimKnob(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobInternal(entref, 1u);
}

void __cdecl GScr_SetFlaggedAnimKnobLimited(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobInternal(entref, 0);
}

void __cdecl GScr_SetFlaggedAnimKnobRestart(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobInternal(entref, 3u);
}

void __cdecl GScr_SetFlaggedAnimKnobLimitedRestart(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobInternal(entref, 2u);
}

void __cdecl GScr_SetFlaggedAnimKnobAllInternal(scr_entref_t entref, unsigned int flags, const char *usage)
{
    gentity_s *Entity; // r24
    double Float; // fp30
    double v7; // fp31
    double v8; // fp29
    XAnimTree_s *EntAnimTree; // r27
    XAnimTree_s *v10; // r5
    XAnimTree_s *v11; // r5
    const XAnim_s *Anims; // r3
    const char *v13; // r3
    DObj_s *ServerDObj; // r31
    int v15; // r7
    unsigned int v16; // r6
    int v17; // r3
    int v18; // r11
    scr_anim_s rootanim; // [sp+50h] [-70h]
    scr_anim_s anim; // [sp+54h] [-6Ch]

    Entity = GetEntity(entref);
    Float = 1.0;
    v7 = 1.0;
    v8 = 0.2;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    switch (Scr_GetNumParam())
    {
    case 3u:
        goto LABEL_9;
    case 4u:
        goto LABEL_7;
    case 5u:
        goto LABEL_5;
    case 6u:
        goto LABEL_3;
    default:
        Scr_Error(usage);
    LABEL_3:
        Float = Scr_GetFloat(5);
        if (Float < 0.0)
            Scr_ParamError(5u, "must set nonnegative rate");
    LABEL_5:
        v8 = Scr_GetFloat(4);
        if (v8 < 0.0)
            Scr_ParamError(4u, "must set nonnegative goal time");
    LABEL_7:
        v7 = Scr_GetFloat(3);
        if (v7 <= 0.0)
            Scr_ParamError(3u, "must set positive weight");
    LABEL_9:
        anim = Scr_GetAnim(2, EntAnimTree);
        rootanim = Scr_GetAnim(1, EntAnimTree);
        if (!Scr_GetConstString(0))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11122, 0, "%s", "notifyName");
        Anims = XAnimGetAnims(EntAnimTree);
        if (!XAnimHasTime(Anims, rootanim.index))
            Scr_ParamError(1u, "blended nonsynchronized animation has no concept of time");
        if (anim.tree != rootanim.tree)
            Scr_Error("root anim is not in the same anim tree");
        if (anim.index == rootanim.index)
            Scr_Error("root anim is not an ancestor of the anim");
        if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        {
            if (flags == 3)
            {
                v13 = "SetFlaggedAnimKnobAllRestart";
            }
            else
            {
                if (flags != 1)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
                        11151,
                        0,
                        "%s",
                        "flags == ANIM_FLAG_COMPLETE");
                v13 = "SetFlaggedAnimKnobAll";
            }
            DumpAnimCommand(v13, EntAnimTree, rootanim.index, anim.index, v7, v8, Float);
        }
        ServerDObj = Com_GetServerDObj(Entity->s.number);
        if (!ServerDObj)
            Scr_ObjectError("No model exists.");
        if ((flags & 1) != 0)
            v17 = XAnimSetCompleteGoalWeightKnobAll(ServerDObj, rootanim.index, anim.index, v7, v8, Float, v16, v15);
        else
            v17 = XAnimSetGoalWeightKnobAll(ServerDObj, rootanim.index, anim.index, v7, v8, Float, v16, v15);
        if (v17)
        {
            GScr_HandleAnimError(v17);
        }
        else
        {
            if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                Entity->flags |= FL_REPEAT_ANIM_UPDATE;
        }
        return;
    }
}

void __cdecl GScr_SetFlaggedAnimKnobAll(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobAllInternal(entref, 1u, "illegal call to SetFlaggedAnimKnobAll()\n");
}

void __cdecl GScr_SetFlaggedAnimKnobAllRestart(scr_entref_t entref)
{
    GScr_SetFlaggedAnimKnobAllInternal(entref, 3u, "illegal call to SetFlaggedAnimKnobAllRestart()\n");
}

void __cdecl GScr_SetFlaggedAnimInternal(scr_entref_t entref, unsigned int flags)
{
    gentity_s *Entity; // r25
    double Float; // fp30
    double v5; // fp31
    double v6; // fp29
    XAnimTree_s *EntAnimTree; // r27
    XAnimTree_s *v8; // r5
    unsigned int v9; // r29
    const XAnim_s *Anims; // r3
    const char *v11; // r3
    DObj_s *ServerDObj; // r31
    int v13; // r7
    unsigned int v14; // r6
    unsigned int v15; // r5
    int v16; // r3
    int v17; // r11

    Entity = GetEntity(entref);
    Float = 1.0;
    v5 = 1.0;
    v6 = 0.2;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_9;
    case 3u:
        goto LABEL_7;
    case 4u:
        goto LABEL_5;
    case 5u:
        goto LABEL_3;
    default:
        Scr_Error("incorrect number of parameters");
    LABEL_3:
        Float = Scr_GetFloat(4);
        if (Float < 0.0)
            Scr_ParamError(4u, "must set nonnegative rate");
    LABEL_5:
        v6 = Scr_GetFloat(3);
        if (v6 < 0.0)
            Scr_ParamError(3u, "must set nonnegative goal time");
    LABEL_7:
        v5 = Scr_GetFloat(2);
        if (v5 <= 0.0)
            Scr_ParamError(2u, "must set positive weight");
    LABEL_9:
        //v9 = (unsigned int)Scr_GetAnim((scr_anim_s *)1, (unsigned int)EntAnimTree, v8) >> 16;
        v9 = Scr_GetAnim(1, EntAnimTree).index;
        if (!Scr_GetConstString(0))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11283, 0, "%s", "notifyName");
        Anims = XAnimGetAnims(EntAnimTree);
        if (!XAnimHasTime(Anims, v9))
            Scr_ParamError(1u, "blended nonsynchronized animation has no concept of time");
        if (g_dumpAnimsCommands->current.integer == Entity->s.number)
        {
            switch (flags)
            {
            case 1u:
                v11 = "SetFlaggedAnim";
                break;
            case 2u:
                v11 = "SetFlaggedAnimLimitedRestart";
                break;
            case 3u:
                v11 = "SetFlaggedAnimRestart";
                break;
            default:
                if (flags)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11306, 0, "%s", "flags == 0");
                v11 = "SetFlaggedAnimLimited";
                break;
            }
            DumpAnimCommand(v11, EntAnimTree, v9, -1, v5, v6, Float);
        }
        ServerDObj = Com_GetServerDObj(Entity->s.number);
        if (!ServerDObj)
            Scr_ObjectError("No model exists.");
        if ((flags & 1) != 0)
            v16 = XAnimSetCompleteGoalWeight(ServerDObj, v9, v5, v6, Float, v15, v14, v13);
        else
            v16 = XAnimSetGoalWeight(ServerDObj, v9, v5, v6, Float, v15, v14, v13);
        if (v16)
        {
            GScr_HandleAnimError(v16);
        }
        else
        {
            if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                Entity->flags |= FL_REPEAT_ANIM_UPDATE;
        }
        return;
    }
}

void __cdecl GScr_SetFlaggedAnim(scr_entref_t entref)
{
    GScr_SetFlaggedAnimInternal(entref, 1u);
}

void __cdecl GScr_SetFlaggedAnimLimited(scr_entref_t entref)
{
    GScr_SetFlaggedAnimInternal(entref, 0);
}

void __cdecl GScr_SetFlaggedAnimRestart(scr_entref_t entref)
{
    GScr_SetFlaggedAnimInternal(entref, 3u);
}

void __cdecl GScr_SetFlaggedAnimLimitedRestart(scr_entref_t entref)
{
    GScr_SetFlaggedAnimInternal(entref, 2u);
}

void __cdecl GScr_SetAnimTime(scr_entref_t entref)
{
    gentity_s *Entity; // r28
    double v2; // fp31
    XAnimTree_s *EntAnimTree; // r29
    XAnimTree_s *v4; // r5
    unsigned int NumParam; // r3
    double Float; // fp1
    const char *v7; // r4
    unsigned int v8; // r31
    const XAnim_s *Anims; // r30
    int flags; // r11

    Entity = GetEntity(entref);
    v2 = 0.0;
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
            Scr_Error("too many parameters");
        Float = Scr_GetFloat(1);
        v2 = Float;
        if (Float >= 0.0)
        {
            if (Float <= 1.0)
                goto LABEL_9;
            v2 = 1.0;
            v7 = "must be < 1";
        }
        else
        {
            v2 = 0.0;
            v7 = "must be > 0";
        }
        Scr_ParamError(1u, v7);
    }
LABEL_9:
    v8 = Scr_GetAnim(0, EntAnimTree).index;
    Anims = XAnimGetAnims(EntAnimTree);
    if (!XAnimIsLeafNode(Anims, v8))
        Scr_ParamError(0, "not a leaf animation");
    if (v2 == 1.0 && XAnimIsLooped(Anims, v8))
        Scr_ParamError(1u, "cannot set time 1 on looping animation");
    XAnimSetTime(EntAnimTree, v8, v2);
    if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
        Entity->flags |= FL_REPEAT_ANIM_UPDATE;
}

void __cdecl GScr_DumpAnims(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    SV_DObjDisplayAnim(Entity, "");
}

void __cdecl GScr_ShellShock(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r29
    const char *String; // r31
    int v3; // r30
    const char *v4; // r3
    long double v5; // fp2
    __int64 v6; // r10
    long double v7; // fp2
    unsigned int v8; // r31
    double v9; // r4
    const char *v10; // r3
    char v11[1056]; // [sp+60h] [-420h] BYREF

    PlayerEntity = GetPlayerEntity(entref);
    if (Scr_GetNumParam() < 2)
        Scr_Error("USAGE: <player> shellshock(<shellshockname>, <duration>)\n");
    if (!chaplinCheat->current.enabled || Scr_GetNumParam() >= 3 && Scr_GetInt(2))
    {
        String = Scr_GetString(0);
        v3 = 1;
        while (1)
        {
            SV_GetConfigstring(v3 + 2535, v11, 1024);
            if (!I_stricmp(v11, String))
                break;
            if (++v3 >= 16)
            {
                v4 = va("shellshock '%s' was not precached\n", String);
                Scr_Error(v4);
                return;
            }
        }
        *(double *)&v5 = (float)((float)(Scr_GetFloat(1) * (float)1000.0) + (float)0.5);
        v7 = floor(v5);
        v8 = (int)(float)*(double *)&v7;
        if (v8 > 0xEA60)
        {
            LODWORD(v6) = (int)(float)*(double *)&v7;
            v9 = (float)((float)v6 * (float)0.001);
            v10 = va((const char *)HIDWORD(v9), LODWORD(v9));
            Scr_ParamError(1u, v10);
        }
        PlayerEntity->client->ps.shellshockIndex = v3;
        PlayerEntity->client->ps.shellshockTime = level.time;
        PlayerEntity->client->ps.shellshockDuration = v8;
        PlayerEntity->client->ps.pm_flags |= 0x10000u;
    }
}

void __cdecl GScr_StopShellShock(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r31

    PlayerEntity = GetPlayerEntity(entref);
    if (Scr_GetNumParam())
        Scr_Error("USAGE: <player> stopshellshock()\n");
    PlayerEntity->client->ps.shellshockIndex = 0;
    PlayerEntity->client->ps.shellshockTime = 0;
    PlayerEntity->client->ps.shellshockDuration = 0;
    PlayerEntity->client->ps.pm_flags &= ~0x10000u;
}

void __cdecl GScr_SetDepthOfField(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r31
    double Float; // fp28
    double v3; // fp29
    double v4; // fp30
    double v5; // fp27
    double v6; // fp26
    double v7; // fp25
    const char *v8; // r3
    const char *v9; // r3

    PlayerEntity = GetPlayerEntity(entref);
    if (Scr_GetNumParam() != 6)
        Scr_Error("Incorrect number of parameters\n");
    Float = Scr_GetFloat(0);
    v3 = Scr_GetFloat(1);
    v4 = Scr_GetFloat(2);
    v5 = Scr_GetFloat(3);
    v6 = Scr_GetFloat(4);
    v7 = Scr_GetFloat(5);
    if (Float < 0.0)
        Scr_ParamError(0, "near start must be >= 0");
    if (v3 < 0.0)
        Scr_ParamError(1u, "near end must be >= 0");
    if (v4 < 0.0)
        Scr_ParamError(2u, "far start must be >= 0");
    if (v5 < 0.0)
        Scr_ParamError(3u, "far end must be >= 0");
    if (v6 < 4.0 || v6 > 10.0)
    {
        v8 = va((const char *)0x40100000, 0, 0);
        Scr_ParamError(4u, v8);
    }
    if (v7 < 0.0 || v7 > v6)
    {
        v9 = va(0, 0);
        Scr_ParamError(5u, v9);
    }
    if (Float >= v3)
    {
        Float = 0.0;
        v3 = 0.0;
    }
    if (v4 >= v5 || v7 == 0.0)
    {
        v4 = 0.0;
        v5 = 0.0;
    }
    else if (v4 < v3)
    {
        Scr_ParamError(
            2u,
            "far start must be >= near end, or far depth of field should be disabled with far start >= far end or far blur == 0");
    }
    PlayerEntity->client->ps.dofNearStart = Float;
    PlayerEntity->client->ps.dofNearEnd = v3;
    PlayerEntity->client->ps.dofFarStart = v4;
    PlayerEntity->client->ps.dofFarEnd = v5;
    PlayerEntity->client->ps.dofNearBlur = v6;
    PlayerEntity->client->ps.dofFarBlur = v7;
}

void __cdecl GScr_SetViewModelDepthOfField(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r31
    double Float; // fp29
    double v3; // fp31

    PlayerEntity = GetPlayerEntity(entref);
    Float = Scr_GetFloat(0);
    v3 = Scr_GetFloat(1);
    if (Float < 0.0)
        Scr_ParamError(0, "start must be >= 0");
    if (v3 < 0.0)
        Scr_ParamError(1u, "end must be >= 0");
    if (Float >= v3)
    {
        Float = 0.0;
        v3 = 0.0;
    }
    PlayerEntity->client->ps.dofViewmodelStart = Float;
    PlayerEntity->client->ps.dofViewmodelEnd = v3;
}

void __cdecl GScr_ViewKick(scr_entref_t entref)
{
    gentity_s *PlayerEntity; // r31
    const char *v2; // r3
    float *p_commandTime; // r11
    double Float; // [sp+18h] [-58h]
    float v5[4]; // [sp+50h] [-20h] BYREF

    PlayerEntity = GetPlayerEntity(entref);
    if (Scr_GetNumParam() != 2)
        Scr_Error("Incorrect number of parameters\n");
    PlayerEntity->client->damage_blood = (Scr_GetInt(0) * PlayerEntity->maxHealth + 50) / 100;
    if (PlayerEntity->client->damage_blood < 0)
    {
        Float = Scr_GetFloat(0);
        v2 = va("viewkick: damage %g < 0\n", Float);
        Scr_Error(v2);
    }
    Scr_GetVector(1u, v5);
    p_commandTime = (float *)&PlayerEntity->client->ps.commandTime;
    p_commandTime[11490] = p_commandTime[7] - v5[0];
    p_commandTime[11491] = p_commandTime[8] - v5[1];
    p_commandTime[11492] = p_commandTime[9] - v5[2];
}

void __cdecl GScr_GetEntnum(scr_entref_t entref)
{
    Scr_AddInt(entref.entnum);
}

void __cdecl GScr_ValidateLightVis(int eType)
{
    const char *v1; // r3

    if (eType && eType != 5 && !alwaysfails)
    {
        v1 = va("(un)lockLightVis: entity type '%i' is not yet handled, get a coder to fix it\n", eType);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11828, 0, v1);
    }
}

void __cdecl GScr_LockLightVis(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    int eType; // r4
    const char *v3; // r3

    Entity = GetEntity(entref);
    eType = Entity->s.eType;
    Entity->s.lerp.eFlags |= 0x400u;
    if (eType && eType != 5 && !alwaysfails)
    {
        v3 = va("(un)lockLightVis: entity type '%i' is not yet handled, get a coder to fix it\n", eType);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11828, 0, v3);
    }
}

void __cdecl GScr_UnlockLightVis(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    int eType; // r4
    const char *v3; // r3

    Entity = GetEntity(entref);
    eType = Entity->s.eType;
    Entity->s.lerp.eFlags &= ~0x400u;
    if (eType && eType != 5 && !alwaysfails)
    {
        v3 = va("(un)lockLightVis: entity type '%i' is not yet handled, get a coder to fix it\n", eType);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11828, 0, v3);
    }
}

void __cdecl GScr_Launch(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    double v3; // fp1
    double v4; // fp3
    double v5; // fp2
    float v7; // [sp+50h] [-40h]
    float v8; // [sp+50h] [-40h]
    float v9; // [sp+50h] [-40h]
    float v10[3]; // [sp+58h] [-38h] BYREF
    //float v11; // [sp+5Ch] [-34h]
    //float v12; // [sp+60h] [-30h]

    if (Scr_GetNumParam() != 1)
        Scr_Error("Incorrect number of parameters\n");
    Entity = GetEntity(entref);
    Scr_GetVector(0, v10);
    v3 = v10[0];
    v4 = v10[2];
    v5 = v10[1];
    if ((LODWORD(v10[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(v10[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(v10[2]) & 0x7F800000) == 0x7F800000)
    {
        Scr_Error(va(
            "invalid velocity parameter in launch command: %f %f %f",
            v10[0],
            v10[1],
            v10[2]
        ));
        v4 = v10[2];
        v5 = v10[1];
        v3 = v10[0];
    }
    v7 = v3;
    if ((LODWORD(v7) & 0x7F800000) == 0x7F800000
        || (v8 = v5, (LODWORD(v8) & 0x7F800000) == 0x7F800000)
        || (v9 = v4, (LODWORD(v9) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            11902,
            0,
            "%s",
            "!IS_NAN((velocity)[0]) && !IS_NAN((velocity)[1]) && !IS_NAN((velocity)[2])");
    }
    Entity->s.lerp.pos.trType = TR_GRAVITY;
    Entity->s.lerp.pos.trTime = level.time;
    Entity->s.lerp.pos.trDelta[0] = v10[0];
    Entity->s.lerp.pos.trDelta[1] = v10[1];
    Entity->s.lerp.pos.trDelta[2] = v10[2];
    if ((COERCE_UNSIGNED_INT(Entity->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(Entity->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(Entity->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            11908,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])");
    }
    Entity->physicsObject = 1;
    Entity->r.contents = 0;
    SV_LinkEntity(Entity);
}

void __cdecl GScr_SetSoundBlend(scr_entref_t entref)
{
    gentity_s *Entity; // r29
    const char *String; // r3
    unsigned __int16 v3; // r31
    const char *v4; // r3
    unsigned __int16 v5; // r30
    double Float; // fp1
    double v7; // fp31

    Entity = GetEntity(entref);
    if (Entity->s.eType != 6)
        Scr_Error("Entity is not a sound_blend\n");
    String = Scr_GetString(0);
    v3 = G_SoundAliasIndexPermanent(String);
    v4 = Scr_GetString(1);
    v5 = G_SoundAliasIndexPermanent(v4);
    Float = Scr_GetFloat(2);
    v7 = Float;
    if (Float < 0.0 || Float > 1.0)
        Scr_Error("Lerp must be between 0.0f and 1.0f\n");
    G_SetSoundBlend(Entity, v3, v5, v7);
}

void __cdecl GScr_LocalToWorldCoords(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    float v2; // [sp+50h] [-60h] BYREF
    float v3; // [sp+54h] [-5Ch]
    float v4; // [sp+58h] [-58h]
    float v5[4]; // [sp+60h] [-50h] BYREF
    float v6[4][3]; // [sp+70h] [-40h] BYREF

    Entity = GetEntity(entref);
    Scr_GetVector(0, v5);
    AnglesToAxis(Entity->r.currentAngles, v6);
    MatrixTransformVector(v5, (const mat3x3&)v6, &v2);
    v2 = Entity->r.currentOrigin[0] + v2;
    v3 = Entity->r.currentOrigin[1] + v3;
    v4 = Entity->r.currentOrigin[2] + v4;
    Scr_AddVector(&v2);
}

void __cdecl GScr_GetEntityNumber(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    Scr_AddInt(Entity->s.number);
}

void __cdecl GScr_EnableGrenadeTouchDamage(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (Entity->classname != scr_const.trigger_damage)
        Scr_Error("Currently on supported on damage triggers");
    Entity->flags |= FL_GRENADE_TOUCH_DAMAGE;
}

void __cdecl GScr_DisableGrenadeTouchDamage(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (Entity->classname != scr_const.trigger_damage)
        Scr_Error("Currently on supported on damage triggers");
    Entity->flags &= ~(FL_GRENADE_TOUCH_DAMAGE);
}

void __cdecl GScr_MissileSetTarget(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    gentity_s *v2; // r30
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (Scr_GetType(0))
        v2 = Scr_GetEntity(0);
    else
        v2 = 0;
    if (Entity->classname != scr_const.rocket)
    {
        v3 = va("Entity %i is not a rocket\n", Entity->s.number);
        Scr_Error(v3);
    }
    Entity->missileTargetEnt.setEnt(v2);
    if (Scr_GetNumParam() <= 1)
    {
        Entity->mover.apos1[1] = 0.0;
        Entity->mover.apos1[2] = 0.0;
        Entity->mover.apos2[0] = 0.0;
    }
    else
    {
        Scr_GetVector(1u, &Entity->mover.apos1[1]);
    }
}

void __cdecl GScr_EnableAimAssist(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (!Entity->r.bmodel)
        Scr_Error("Currently only supported on entities with brush models");
    Entity->s.lerp.eFlags |= 0x800u;
}

void __cdecl GScr_DisableAimAssist(scr_entref_t entref)
{
    gentity_s *Entity; // r31

    Entity = GetEntity(entref);
    if (!Entity->r.bmodel)
        Scr_Error("Currently only supported on entities with brush models");
    Entity->s.lerp.eFlags &= ~0x800u;
}

void __cdecl GScr_MakeFakeAI(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31

    Entity = GetEntity(entref);
    v2 = Entity;
    if (Entity->s.eType != 5 || (Entity->flags & 0x2000) == 0)
        Scr_Error("makeFakeAI must be applied to a script_model");
    v2->r.svFlags = v2->r.svFlags & 0xF9 | 2;
    v2->r.mins[0] = -15.0;
    v2->r.mins[1] = -15.0;
    v2->r.mins[2] = 0.0;
    v2->r.maxs[0] = 15.0;
    v2->r.maxs[1] = 15.0;
    v2->r.maxs[2] = 72.0;
    v2->r.contents |= 0x8000u;
    SV_LinkEntity(v2);
}

void __cdecl GScr_SetLookatText(scr_entref_t entref)
{
    gentity_s *ent; // r31
    unsigned int ConstString; // r3
    unsigned int ConstIString; // r3

    ent = GetEntity(entref);
    iassert(ent);
    ConstString = Scr_GetConstString(0);
    Scr_SetString(&ent->lookAtText0, ConstString);
    if (Scr_GetNumParam())
    {
        ConstIString = Scr_GetConstIString(1);
        Scr_SetString(&ent->lookAtText1, ConstIString);
    }
}

void __cdecl GScr_SetRightArc(scr_entref_t entref)
{
    TurretInfo *pTurretInfo; // r31
    double Float; // fp1

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    Float = Scr_GetFloat(0);
    pTurretInfo->arcmin[1] = -Float;
    if (-Float > 0.0)
        pTurretInfo->arcmin[1] = 0.0;
}

void __cdecl GScr_SetLeftArc(scr_entref_t entref)
{
    TurretInfo *pTurretInfo; // r31
    double Float; // fp1

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    Float = Scr_GetFloat(0);
    pTurretInfo->arcmax[1] = Float;
    if (Float < 0.0)
        pTurretInfo->arcmax[1] = 0.0;
}

void __cdecl GScr_SetTopArc(scr_entref_t entref)
{
    TurretInfo *pTurretInfo; // r31
    double Float; // fp1

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    Float = Scr_GetFloat(0);
    pTurretInfo->arcmin[0] = -Float;
    if (-Float > 0.0)
        pTurretInfo->arcmin[0] = 0.0;
}

void __cdecl GScr_SetBottomArc(scr_entref_t entref)
{
    TurretInfo *pTurretInfo; // r31
    double Float; // fp1

    pTurretInfo = GetEntity(entref)->pTurretInfo;
    if (!pTurretInfo)
        Scr_Error("entity is not a turret");
    Float = Scr_GetFloat(0);
    pTurretInfo->arcmax[0] = Float;
    if (Float < 0.0)
        pTurretInfo->arcmax[0] = 0.0;
}

void __cdecl GScr_SetDefaultDropPitch(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    double Float; // fp1

    if (Scr_GetNumParam() == 1)
    {
        Entity = GetEntity(entref);
        if (Entity->pTurretInfo)
        {
            Float = Scr_GetFloat(0);
            turret_SetDefaultDropPitch(Entity, Float);
        }
        else
        {
            Scr_Error("entity is not a turret");
        }
    }
    else
    {
        Scr_Error("illegal call to setdefaultdroppitch()\n");
    }
}

void __cdecl GScr_RestoreDefaultDropPitch(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    if (Scr_GetNumParam())
    {
        Scr_Error("illegal call to restoredefaultdroppitch()\n");
    }
    else
    {
        Entity = GetEntity(entref);
        if (Entity->pTurretInfo)
            turret_RestoreDefaultDropPitch(Entity);
        else
            Scr_Error("entity is not a turret");
    }
}

void __cdecl GScr_TurretFireDisable(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    v2->pTurretInfo->flags |= 0x4000u;
}

void __cdecl GScr_TurretFireEnable(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Entity = GetEntity(entref);
    v2 = Entity;
    if (!Entity->pTurretInfo)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("entity type '%s' is not a turret", v3);
        Scr_Error(v4);
    }
    v2->pTurretInfo->flags &= ~0x4000u;
}

void __cdecl GScr_SetSpawnerTeam(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    const char *String; // r31
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (Entity->s.eType != 15)
        Scr_Error("setspawnerteam can only be applied to AI spawners");
    String = Scr_GetString(0);
    if (I_stricmp(String, "axis"))
    {
        if (I_stricmp(String, "allies"))
        {
            if (I_stricmp(String, "neutral"))
            {
                v3 = va("unknown team '%s', should be axis, allies, or neutral", String);
                Scr_ParamError(0, v3);
            }
            else
            {
                Entity->item[0].ammoCount = 3;
            }
        }
        else
        {
            Entity->item[0].ammoCount = 2;
        }
    }
    else
    {
        Entity->item[0].ammoCount = 1;
    }
}

void GScr_PrecacheMenu()
{
    const char *String; // r30
    int i; // r31
    int j; // r31
    const char *v3; // r3
    char v4[1032]; // [sp+50h] [-420h] BYREF

    if (!level.initializing)
        Scr_Error("precacheMenu must be called before any wait statements in the level script\n");
    String = Scr_GetString(0);
    for (i = 0; i < 32; ++i)
    {
        SV_GetConfigstring(i + 2551, v4, 1024);
        if (!I_stricmp(v4, String))
        {
            Com_DPrintf(23, "Script tried to precache the menu '%s' more than once\n", String);
            return;
        }
    }
    for (j = 0; j < 32; ++j)
    {
        SV_GetConfigstring(j + 2551, v4, 1024);
        if (!v4[0])
            break;
    }
    if (j == 32)
    {
        v3 = va("Too many menus precached. Max allowed menus is %i", 32);
        Scr_Error(v3);
    }
    SV_SetConfigstring(j + 2551, String);
}

int __cdecl GScr_GetScriptMenuIndex(const char *pszMenu)
{
    int v2; // r31
    const char *v3; // r3
    char v5[1032]; // [sp+50h] [-420h] BYREF

    v2 = 0;
    while (1)
    {
        SV_GetConfigstring(v2 + 2551, v5, 1024);
        if (!I_stricmp(v5, pszMenu))
            break;
        if (++v2 >= 32)
        {
            v3 = va("Menu '%s' was not precached\n", pszMenu);
            Scr_Error(v3);
            return 0;
        }
    }
    return v2;
}

void GScr_SetDebugOrigin()
{
    float v0[6]; // [sp+50h] [-20h] BYREF

    Scr_GetVector(0, v0);
    CG_SetDebugOrigin(v0);
}

void GScr_SetDebugAngles()
{
    float v0[6]; // [sp+50h] [-20h] BYREF

    Scr_GetVector(0, v0);
    CG_SetDebugAngles(v0);
}

void GScr_OpenFile()
{
    const char *String; // r25
    const char *v1; // r3
    int v2; // r26
    int *openScriptIOFileHandles; // r11
    int v4; // r28
    int *v5; // r31
    const char *v6; // r10
    const char *v7; // r11
    int v8; // r8
    const char *v9; // r31
    int Remote; // r3
    int v11; // r31
    unsigned char *v12; // r3
    void *v13; // r5
    const char *v14; // r10
    const char *v15; // r11
    int v16; // r8
    const char *v17; // r3
    int v18; // r3
    const char *v19; // r10
    const char *v20; // r11
    int v21; // r8
    const char *v22; // r3
    void *v23[20]; // [sp+50h] [-50h] BYREF

    if (Scr_GetNumParam() > 1)
    {
        String = Scr_GetString(0);
        v1 = Scr_GetString(1);
        v2 = 0;
        openScriptIOFileHandles = level.openScriptIOFileHandles;
        while (*openScriptIOFileHandles)
        {
            ++openScriptIOFileHandles;
            ++v2;
            if ((int)openScriptIOFileHandles >= (int)level.openScriptIOFileBuffers)
                goto LABEL_7;
        }
        v4 = v2;
        v5 = &level.openScriptIOFileHandles[v2];
        if (!v5)
        {
        LABEL_7:
            Com_Printf(23, "OpenFile failed.  %i files already open\n", 1);
            Scr_AddInt(-1);
            return;
        }
        v6 = "read";
        v7 = v1;
        do
        {
            v8 = *(unsigned __int8 *)v7 - *(unsigned __int8 *)v6;
            if (!*v7)
                break;
            ++v7;
            ++v6;
        } while (!v8);
        if (!v8)
        {
            // KISAKTODO??
            //v9 = va("%s/%s", "scriptdata", String);
            //if (FS_IsUsingRemotePCSharing())
            //    Remote = FS_FOpenFileReadRemote(v9, 0, v23);
            //else
            //    Remote = FS_FOpenFileByMode(v9, v23, FS_READ);
            //v11 = Remote;
            if (Remote >= 0)
            {
                v12 = (unsigned char *)Z_VirtualAlloc(Remote + 1, "GScr_OpenFile", 10);
                v13 = v23[0];
                level.openScriptIOFileBuffers[v4] = v12;
                FS_Read(v12, v11, (int)v13);
                FS_FCloseFile((int)v23[0]);
                level.openScriptIOFileBuffers[v4][v11] = 0;
                Com_BeginParseSession(String);
                Com_SetCSV(1);
                level.currentScriptIOLineMark[v2].lines = 0;
                Scr_AddInt(v2);
                return;
            }
            goto LABEL_30;
        }
        v14 = "write";
        v15 = v1;
        do
        {
            v16 = *(unsigned __int8 *)v15 - *(unsigned __int8 *)v14;
            if (!*v15)
                break;
            ++v15;
            ++v14;
        } while (!v16);
        if (v16)
        {
            v19 = "append";
            v20 = v1;
            do
            {
                v21 = *(unsigned __int8 *)v20 - *(unsigned __int8 *)v19;
                if (!*v20)
                    break;
                ++v20;
                ++v19;
            } while (!v21);
            if (v21)
            {
                Com_Printf(23, "Valid openfile modes are 'write', 'read', and 'append'\n");
            }
            else
            {
                v22 = va("%s/%s", "scriptdata", String);
                if (FS_FOpenFileByMode((char*)v22, &level.openScriptIOFileHandles[v2], FS_APPEND) >= 0)
                    goto LABEL_22;
            }
        }
        else
        {
            v17 = va("%s/%s", "scriptdata", String);
            v18 = FS_FOpenTextFileWrite(v17);
            *v5 = v18;
            if (v18)
            {
            LABEL_22:
                Scr_AddInt(v2);
                return;
            }
        }
    LABEL_30:
        Scr_AddInt(-1);
    }
}

void GScr_CloseFile()
{
    unsigned int Int; // r3
    unsigned int v1; // r28
    unsigned int v2; // r31
    int v3; // r3

    if (Scr_GetNumParam())
    {
        Int = Scr_GetInt(0);
        v1 = Int;
        if (Int > 1)
        {
            Com_Printf(23, "CloseFile failed, invalid file number %i\n", Int);
        }
        else
        {
            v2 = Int;
            if (level.openScriptIOFileHandles[Int])
            {
                if (level.openScriptIOFileBuffers[v2])
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
                        12697,
                        0,
                        "%s",
                        "!((level.openScriptIOFileHandles[filenum] != 0) && (level.openScriptIOFileBuffers[filenum] != NULL))");
            }
            v3 = level.openScriptIOFileHandles[v2];
            if (v3)
            {
                FS_FCloseFile(v3);
                level.openScriptIOFileHandles[v2] = 0;
                Scr_AddInt(1);
                return;
            }
            if (level.openScriptIOFileBuffers[v2])
            {
                Com_EndParseSession();
                Z_VirtualFree(level.openScriptIOFileBuffers[v2]);
                level.openScriptIOFileBuffers[v2] = 0;
                Scr_AddInt(1);
                return;
            }
            Com_Printf(23, "CloseFile failed, file number %i was not open\n", v1);
        }
        Scr_AddInt(-1);
    }
}

void __cdecl Scr_FPrint_internal(bool commaBetweenFields)
{
    unsigned int Int; // r3
    unsigned int v3; // r31
    unsigned int i; // r28
    const char *String; // r3
    const char *v6; // r11
    unsigned int NumParam; // r3

    if (Scr_GetNumParam() <= 1)
    {
        Com_Printf(23, "fprintln requires at least 2 parameters (file, output)\n");
        Scr_AddInt(-1);
        return;
    }
    Int = Scr_GetInt(0);
    if (Int > 1)
    {
        Com_Printf(23, "FPrintln failed, invalid file number %i\n");
        goto LABEL_14;
    }
    v3 = Int;
    if (!level.openScriptIOFileHandles[Int])
    {
        Com_Printf(23, "FPrintln failed, file number %i was not open for writing\n");
    LABEL_14:
        Scr_AddInt(-1);
        return;
    }
    for (i = 1; i < Scr_GetNumParam(); ++i)
    {
        String = Scr_GetString(i);
        v6 = String;
        while (*(unsigned __int8 *)v6++)
            ;
        FS_Write(String, v6 - String - 1, level.openScriptIOFileHandles[v3]);
        if (commaBetweenFields)
            FS_Write(",", 1, level.openScriptIOFileHandles[v3]);
    }
    FS_Write("\n", 1, level.openScriptIOFileHandles[v3]);
    NumParam = Scr_GetNumParam();
    Scr_AddInt(NumParam - 1);
}

void GScr_FPrintln()
{
    Scr_FPrint_internal(0);
}

void GScr_FPrintFields()
{
    Scr_FPrint_internal(1);
}

void GScr_FReadLn()
{
    unsigned int Int; // r3
    com_parse_mark_t *v1; // r31
    int ArgCountOnLine; // r3
    const char *v3; // r3
    com_parse_mark_t *v4; // r4
    bool v5; // r31
    const char *v6[4]; // [sp+50h] [-20h] BYREF

    if (!Scr_GetNumParam())
    {
        Com_Printf(23, "freadln requires a parameter - the file to read from\n");
    LABEL_11:
        ArgCountOnLine = -1;
        goto LABEL_12;
    }
    Int = Scr_GetInt(0);
    if (Int > 1)
    {
        Com_Printf(23, "freadln failed, invalid file number %i\n");
        goto LABEL_11;
    }
    if (!level.openScriptIOFileBuffers[Int])
    {
        Com_Printf(23, "freadln failed, file number %i was not open for reading\n");
        goto LABEL_11;
    }
    v6[0] = (const char*)level.openScriptIOFileBuffers[Int];
    v1 = &level.currentScriptIOLineMark[Int];
    if (v1->lines)
    {
        Com_ParseReturnToMark(v6, v1);
        Com_SkipRestOfLine(v6);
        Com_ParseSetMark(v6, v1);
        v3 = Com_Parse(v6)->token;
        v4 = v1;
        v5 = *v3 == 0;
        Com_ParseReturnToMark(v6, v4);
        if (v5)
            goto LABEL_11;
        ArgCountOnLine = Com_GetArgCountOnLine(v6);
    }
    else
    {
        Com_ParseSetMark(v6, v1);
        ArgCountOnLine = Com_GetArgCountOnLine(v6);
    }
LABEL_12:
    Scr_AddInt(ArgCountOnLine);
}

void GScr_FGetArg()
{
    unsigned int Int; // r31
    int v1; // r3
    int v2; // r30
    const char *v3; // r29
    int v4; // r31
    const char *v5[12]; // [sp+50h] [-30h] BYREF

    if (Scr_GetNumParam() <= 1)
    {
        Com_Printf(23, "freadline requires at least 2 parameters (file, string)\n");
        Scr_AddString("");
        return;
    }
    Int = Scr_GetInt(0);
    v1 = Scr_GetInt(1);
    v2 = v1;
    if (Int > 1)
    {
        Com_Printf(23, "freadline failed, invalid file number %i\n", Int);
        goto LABEL_14;
    }
    if (v1 < 0)
    {
        Com_Printf(23, "freadline failed, invalid argument number %i\n", v1);
    LABEL_14:
        Scr_AddString("");
        return;
    }
    if (!level.openScriptIOFileBuffers[Int])
    {
        Com_Printf(23, "freadline failed, file number %i was not open for reading\n", Int);
        goto LABEL_14;
    }
    v5[0] = (const char*)level.openScriptIOFileBuffers[Int];
    v3 = 0;
    Com_ParseReturnToMark(v5, &level.currentScriptIOLineMark[Int]);
    v4 = 0;
    if (v2 < 0)
    {
    LABEL_11:
        Scr_AddString(v3);
    }
    else
    {
        while (1)
        {
            v3 = Com_ParseOnLine(v5)->token;
            if (!*v3)
                break;
            if (++v4 > v2)
                goto LABEL_11;
        }
        Com_Printf(
            23,
            "freadline failed, there aren't %i arguments on this line, there are only %i arguments\n",
            v2 + 1,
            v4);
        Scr_AddString("");
    }
}

void __cdecl Scr_PlayRumbleOnPosition_Internal(int event)
{
    const char *String; // r31
    unsigned int v3; // r30
    const char *v4; // r3
    float v5[12]; // [sp+50h] [-30h] BYREF

    String = Scr_GetString(0);
    v3 = G_RumbleIndex(String);
    if (!v3)
    {
        v4 = va("unknown rumble name '%s'", String);
        Scr_ParamError(0, v4);
    }
    Scr_GetVector(1u, v5);
    G_TempEntity(v5, event)->s.eventParm = v3;
}

gentity_s *Scr_PlayRumbleOnPosition()
{
    const char *String; // r31
    int v1; // r30
    const char *v2; // r3
    gentity_s *result; // r3
    float v4[6]; // [sp+50h] [-30h] BYREF

    if (Scr_GetNumParam() != 2)
        Scr_ParamError(0, "PlayRumbleOnPosition [rumble name] [pos]");
    String = Scr_GetString(0);
    v1 = G_RumbleIndex(String);
    if (!v1)
    {
        v2 = va("unknown rumble name '%s'", String);
        Scr_ParamError(0, v2);
    }
    Scr_GetVector(1u, v4);
    result = G_TempEntity(v4, 71);
    result->s.eventParm = v1;
    return result;
}

gentity_s *Scr_PlayRumbleLoopOnPosition()
{
    const char *String; // r31
    int v1; // r30
    const char *v2; // r3
    gentity_s *result; // r3
    float v4[6]; // [sp+50h] [-30h] BYREF

    if (Scr_GetNumParam() != 2)
        Scr_ParamError(0, "PlayRumbleLoopOnPosition [rumble name] [pos]");
    String = Scr_GetString(0);
    v1 = G_RumbleIndex(String);
    if (!v1)
    {
        v2 = va("unknown rumble name '%s'", String);
        Scr_ParamError(0, v2);
    }
    Scr_GetVector(1u, v4);
    result = G_TempEntity(v4, 73);
    result->s.eventParm = v1;
    return result;
}

gentity_s *Scr_StopAllRumbles()
{
    gentity_s *result; // r3
    float v1[6]; // [sp+50h] [-20h] BYREF

    v1[0] = 0.0;
    v1[1] = 0.0;
    v1[2] = 0.0;
    result = G_TempEntity(v1, 75);
    result->s.eventParm = 0;
    return result;
}

void ScrCmd_GiveAchievement()
{
    const char *String; // r31
    int v1; // r3

    if (Scr_GetNumParam() != 1)
        Scr_ParamError(0, "giveachievement [name]");
    String = Scr_GetString(0);
    v1 = CL_ControllerIndexFromClientNum(0);
    //Live_GiveAchievement(v1, String);
}

void ScrCmd_UpdateGamerProfile()
{
    Cbuf_AddText(0, "updategamerprofile");
}

void GScr_SetMiniMap()
{
    double Float; // fp31
    double v1; // fp30
    double v2; // fp29
    double v3; // fp28
    double v4; // fp10
    const char *v5; // r3

    if (Scr_GetNumParam() != 5)
        Scr_Error("Expecting 5 arguments");
    Scr_GetString(0);
    Float = Scr_GetFloat(1);
    v1 = Scr_GetFloat(2);
    v2 = Scr_GetFloat(3);
    v3 = Scr_GetFloat(4);
    v4 = -(float)(level.compassNorth[0] * (float)((float)v2 - (float)Float));
    level.compassMapWorldSize[0] = (float)(level.compassNorth[1] * (float)((float)v2 - (float)Float))
        - (float)(level.compassNorth[0] * (float)((float)v3 - (float)v1));
    level.compassMapWorldSize[1] = -(float)((float)(level.compassNorth[1] * (float)((float)v3 - (float)v1)) - (float)v4);
    if (level.compassMapWorldSize[0] < 0.0
        || (float)-(float)((float)(level.compassNorth[1] * (float)((float)v3 - (float)v1)) - (float)v4) < 0.0)
    {
        Scr_Error("lower-right X and Y coordinates must be southeast of upper-left X and Y coordinates in terms of the northyaw");
    }
    level.compassMapUpperLeft[0] = Float;
    level.compassMapUpperLeft[1] = v1;
    v5 = va("\"%s\" %f %f %f %f", HIDWORD(Float), LODWORD(Float), LODWORD(v1), LODWORD(v2), LODWORD(v3));
    SV_SetConfigstring(1148, v5);
}

void GScr_GetArrayKeys()
{
    const char *TypeName; // r3
    const char *v1; // r3
    unsigned int Object; // r3

    if (Scr_GetPointerType(0) != 21)
    {
        TypeName = Scr_GetTypeName(0);
        v1 = va("Parameter (%s) must be an array", TypeName);
        Scr_ParamError(0, v1);
    }
    Object = Scr_GetObject(0);
    Scr_AddArrayKeys(Object);
}

void GScr_ClearLocalizedStrings()
{
    const char *TypeName; // r3
    const char *v1; // r3
    unsigned int Object; // r3

    if (Scr_GetPointerType(0) != 21)
    {
        TypeName = Scr_GetTypeName(0);
        v1 = va("Parameter (%s) must be an array", TypeName);
        Scr_ParamError(0, v1);
    }
    Object = Scr_GetObject(0);
    Scr_AddArrayKeys(Object);
}

void Scr_TableLookup()
{
    const char *String; // r3
    int Int; // r31
    const char *v2; // r30
    int v3; // r3
    const char *v4; // r3
    StringTable *v5; // [sp+50h] [-20h] BYREF

    if (Scr_GetNumParam() < 3)
        Scr_Error("USAGE: tableLookup( filename, searchColumnNum, searchValue, returnValueColumnNum )\n");
    String = Scr_GetString(0);
    StringTable_GetAsset(String, &v5);
    Int = Scr_GetInt(1);
    v2 = Scr_GetString(2);
    v3 = Scr_GetInt(3);
    v4 = StringTable_Lookup(v5, Int, v2, v3);
    Scr_AddString(v4);
}

void Scr_TableLookupIString()
{
    const char *String; // r3
    int Int; // r31
    const char *v2; // r30
    int v3; // r3
    const char *v4; // r3
    StringTable *v5; // [sp+50h] [-20h] BYREF

    if (Scr_GetNumParam() < 3)
        Scr_Error("USAGE: tableLookupIString( filename, searchColumnNum, searchValue, returnValueColumnNum )\n");
    String = Scr_GetString(0);
    StringTable_GetAsset(String, &v5);
    Int = Scr_GetInt(1);
    v2 = Scr_GetString(2);
    v3 = Scr_GetInt(3);
    v4 = StringTable_Lookup(v5, Int, v2, v3);
    Scr_AddIString(v4);
}

float locs[255][3];
void __cdecl Scr_GetReflectionLocs()
{
    const float *v0; // r30
    unsigned int DebugReflectionProbeLocs; // r31

    v0 = locs[0];
    DebugReflectionProbeLocs = R_GetDebugReflectionProbeLocs(locs, 0xFF);
    Scr_MakeArray();
    for (; DebugReflectionProbeLocs; v0 += 3)
    {
        Scr_AddVector(v0);
        Scr_AddArray();
        --DebugReflectionProbeLocs;
    }
}

void Scr_RefreshHudCompass()
{
    const char *v0; // r3

    v0 = va("menu_show_notify \"%i\"", 2);
    SV_GameSendServerCommand(-1, v0);
}

void Scr_RefreshHudAmmoCounter()
{
    const char *v0; // r3

    v0 = va("menu_show_notify \"%i\"", 1);
    SV_GameSendServerCommand(-1, v0);
}

void Scr_LogString()
{
    const char *String; // r3

    String = Scr_GetString(0);
    //LSP_LogString(cl_controller_in_use, String);
}

void __cdecl ScrCmd_LogString(scr_entref_t entref)
{
    const char *String; // r3

    String = Scr_GetString(0);
    //LSP_LogString(cl_controller_in_use, String);
}

void(__cdecl *__cdecl BuiltIn_GetFunction(const char **pName, int *type))()
{
    int v2; // r6
    unsigned int v3; // r31
    BuiltinFunctionDef *i; // r7
    const char *actionString; // r10
    const char *v6; // r11
    int v7; // r8

    v2 = 0;
    v3 = 0;
    for (i = functions; ; ++i)
    {
        actionString = i->actionString;
        v6 = *pName;
        do
        {
            v7 = (unsigned __int8)*v6 - *(unsigned __int8 *)actionString;
            if (!*v6)
                break;
            ++v6;
            ++actionString;
        } while (!v7);
        if (!v7)
            break;
        v3 += 12;
        ++v2;
        if (v3 >= 0xBB8)
            return 0;
    }
    *pName = functions[v2].actionString;
    *type = functions[v2].type;
    return functions[v2].actionFunc;
}

void(__cdecl *__cdecl Scr_GetFunction(const char **pName, int *type))()
{
    void(__cdecl * result)(); // r3

    *type = 0;
    result = Sentient_GetFunction(pName);
    if (!result)
        return BuiltIn_GetFunction(pName, type);
    return result;
}

void __cdecl ScrCmd_PlayRumbleOnEntity_Internal(scr_entref_t entref, int event)
{
    gentity_s *Entity; // r29
    const char *String; // r31
    unsigned int v5; // r30
    const char *v6; // r3

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    v5 = G_RumbleIndex(String);
    if (!v5)
    {
        v6 = va("unknown rumble name '%s'", String);
        Scr_ParamError(0, v6);
    }
    Entity->r.svFlags &= ~1u;
    if (Scr_GetNumParam() == 1)
        G_AddEvent(Entity, event, v5);
    else
        Scr_Error("Incorrect number of parameters.\n");
}

void __cdecl ScrCmd_PlayRumbleOnEntity(scr_entref_t entref)
{
    ScrCmd_PlayRumbleOnEntity_Internal(entref, 70);
}

void __cdecl ScrCmd_PlayRumbleLoopOnEntity(scr_entref_t entref)
{
    ScrCmd_PlayRumbleOnEntity_Internal(entref, 72);
}

void __cdecl ScrCmd_StopRumble(scr_entref_t entref)
{
    gentity_s *Entity; // r29
    const char *String; // r31
    unsigned int v3; // r30
    const char *v4; // r3

    Entity = GetEntity(entref);
    String = Scr_GetString(0);
    v3 = G_RumbleIndex(String);
    if (!v3)
    {
        v4 = va("unknown rumble name '%s'", String);
        Scr_ParamError(0, v4);
    }
    Entity->r.svFlags &= ~1u;
    if (Scr_GetNumParam() == 1)
        G_AddEvent(Entity, 74, v3);
    else
        Scr_Error("Incorrect number of parameters.\n");
}

void __cdecl ScrCmd_AddAIEventListener(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    unsigned __int16 ConstString; // r3

    Entity = GetEntity(entref);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 13879, 0, "%s", "ent");
    ConstString = Scr_GetConstString(0);
    Actor_EventListener_Add(Entity->s.number, ConstString);
}

void __cdecl ScrCmd_RemoveAIEventListener(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    unsigned __int16 ConstString; // r3

    Entity = GetEntity(entref);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 13901, 0, "%s", "ent");
    if (Scr_GetNumParam())
    {
        ConstString = Scr_GetConstString(0);
        Actor_EventListener_Remove(Entity->s.number, ConstString);
    }
    else
    {
        Actor_EventListener_RemoveEntity(Entity->s.number);
    }
}

gentity_s *__cdecl GScr_SetupLightEntity(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *v2; // r3
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 13917, 0, "%s", "ent");
    if (Entity->s.eType != 9)
    {
        v2 = SL_ConvertToString(Entity->classname);
        v3 = va("Function can only be called on a 'light' entity; actual classname is '%s'\n", v2);
        Scr_Error(v3);
    }
    return Entity;
}

void __cdecl GScr_GetLightColor(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-14h]
    float unpackedColor[4]; // [esp+4h] [ebp-10h] BYREF

    ent = GScr_SetupLightEntity(entref);
    Byte4UnpackRgba((const unsigned __int8 *)&ent->s.lerp.u, unpackedColor);
    Scr_AddVector(unpackedColor);
}

void __cdecl GScr_SetLightColor(scr_entref_t entref)
{
    gentity_s *v1; // r31
    unsigned __int8 v2; // r30
    float v3[3]; // [sp+50h] [-30h] BYREF
    float v4; // [sp+5Ch] [-24h]

    v1 = GScr_SetupLightEntity(entref);
    Scr_GetVector(0, v3);
    v4 = 0.0;
    v2 = v1->s.lerp.u.primaryLight.colorAndExp[3];
    v1->s.lerp.u.primaryLight.colorAndExp[0] = ByteFromFloatColor(v3[0]);
    v1->s.lerp.u.primaryLight.colorAndExp[1] = ByteFromFloatColor(v3[1]);
    v1->s.lerp.u.primaryLight.colorAndExp[2] = ByteFromFloatColor(v3[2]);
    v1->s.lerp.u.primaryLight.colorAndExp[3] = ByteFromFloatColor(v4);
    v1->s.lerp.u.primaryLight.colorAndExp[3] = v2;
}

void __cdecl GScr_GetLightIntensity(scr_entref_t entref)
{
    gentity_s *v1; // r3

    v1 = GScr_SetupLightEntity(entref);
    Scr_AddFloat(v1->s.lerp.u.turret.gunAngles[1]);
}

void __cdecl GScr_SetLightIntensity(scr_entref_t entref)
{
    int v1; // [esp+0h] [ebp-Ch]
    float intensity; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_SetupLightEntity(entref);
    intensity = Scr_GetFloat(0);
    if (intensity < -0.001)
        Scr_ParamError(0, "intensity must be >= 0");
    if ((float)(intensity - 0.0) < 0.0)
        v1 = 0.0f;
    else
        v1 = LODWORD(intensity);
    ent->s.lerp.u.loopFx.period = v1;
}

void __cdecl GScr_GetLightRadius(scr_entref_t entref)
{
    gentity_s *v1; // r3

    v1 = GScr_SetupLightEntity(entref);
    Scr_AddFloat(v1->s.lerp.u.turret.gunAngles[2]);
}

void __cdecl GScr_SetLightRadius(scr_entref_t entref)
{
    const char *v1; // eax
    int v2; // [esp+Ch] [ebp-18h]
    float v3; // [esp+14h] [ebp-10h]
    float radius; // [esp+18h] [ebp-Ch]
    gentity_s *ent; // [esp+1Ch] [ebp-8h]
    const ComPrimaryLight *refLight; // [esp+20h] [ebp-4h]

    ent = GScr_SetupLightEntity(entref);
    refLight = Com_GetPrimaryLight(ent->s.index.brushmodel);
    iassert(refLight);
    radius = Scr_GetFloat(0);
    if (radius >= -0.001)
    {
        if (radius > (float)(refLight->radius + 0.001))
        {
            v1 = va("radius must be less than the bsp radius (%g)", refLight->radius);
            Scr_ParamError(0, v1);
        }
    }
    else
    {
        Scr_ParamError(0, "radius must be >= 0");
    }
    if ((float)(radius - refLight->radius) < 0.0)
        v3 = radius;
    else
        v3 = refLight->radius;
    if ((float)(0.0 - radius) < 0.0)
        v2 = LODWORD(v3);
    else
        v2 = 0.0f;
    //ent->s.lerp.u.actor.team = v2;
    ent->s.lerp.u.primaryLight.radius = v2;
}

void __cdecl GScr_GetLightFovInner(scr_entref_t entref)
{
    long double v1; // fp2
    long double v2; // fp2

    *(double *)&v1 = GScr_SetupLightEntity(entref)->s.lerp.u.primaryLight.cosHalfFovInner;
    v2 = acos(v1);
    Scr_AddFloat((float)((float)*(double *)&v2 * (float)2.0));
}

void __cdecl GScr_GetLightFovOuter(scr_entref_t entref)
{
    long double v1; // fp2
    long double v2; // fp2

    *(double *)&v1 = GScr_SetupLightEntity(entref)->s.lerp.u.primaryLight.cosHalfFovOuter;
    v2 = acos(v1);
    Scr_AddFloat((float)((float)*(double *)&v2 * (float)2.0));
}

// aislop
void GScr_SetLightFovRange(scr_entref_t entref)
{
    gentity_s *ent = GScr_SetupLightEntity(entref);
    const ComPrimaryLight *lightDef = Com_GetPrimaryLight(ent->s.index.primaryLight);

    if (!lightDef)
        MyAssertHandler(__FILE__, 14134, 0, "%s", "refLight");

    float outerFov = Scr_GetFloat(0);
    if (outerFov < 1.0f || outerFov >= 120.0f)
        Scr_ParamError(0, "outer fov must be in the range of 1 to 120");

    float cosOuter = cosf(outerFov * (M_PI / 180.0f * 0.5f));
    if (cosOuter < lightDef->cosHalfFovOuter - 0.001f)
        Scr_ParamError(0, "outer fov cannot be larger than the fov when the map was compiled");

    float clampedCosOuter = fmaxf(cosOuter, lightDef->cosHalfFovOuter - 0.001f);
    float innerCos = lightDef->cosHalfFovInner;

    if (Scr_GetNumParam() == 2)
    {
        float innerFov = Scr_GetFloat(1);
        if (innerFov < 0.0f || innerFov >= outerFov + 0.001f)
            Scr_ParamError(1, "inner fov must be in the range of 0 to outer fov");

        innerCos = cosf(innerFov * (M_PI / 180.0f * 0.5f));
        innerCos = fminf(innerCos, clampedCosOuter + 0.001f);
    }
    else
    {
        innerCos = fminf(innerCos, clampedCosOuter + 0.001f);
    }

    if (clampedCosOuter <= 0.0f || clampedCosOuter >= innerCos || innerCos > 1.0f)
    {
        const char *debugMsg = va(
            "Invalid FOV configuration: cosHalfFovOuter = %.6f, cosHalfFovInner = %.6f",
            clampedCosOuter, innerCos
        );

        MyAssertHandler(__FILE__, 14157, 0,
            "0.0f < cosHalfFovOuter && cosHalfFovOuter < cosHalfFovInner && cosHalfFovInner <= 1.0f\n\t%s",
            debugMsg);
    }

    ent->s.lerp.u.primaryLight.cosHalfFovOuter = clampedCosOuter;
    ent->s.lerp.u.primaryLight.cosHalfFovInner = innerCos;
}


void __cdecl GScr_GetLightExponent(scr_entref_t entref)
{
    gentity_s *v1; // r3

    v1 = GScr_SetupLightEntity(entref);
    Scr_AddInt(v1->s.lerp.u.primaryLight.colorAndExp[3]);
}

void __cdecl GScr_SetLightExponent(scr_entref_t entref)
{
    gentity_s *v1; // r30
    unsigned int Int; // r3
    unsigned __int8 v3; // r31

    v1 = GScr_SetupLightEntity(entref);
    Int = Scr_GetInt(0);
    v3 = Int;
    if (Int > 0x64)
        Scr_ParamError(0, "exponent must be in the range from 0 to 100");
    v1->s.lerp.u.primaryLight.colorAndExp[3] = v3;
}

void __cdecl GScr_StartRagdoll(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    trType_t trType; // r11
    trajectory_t *p_pos; // r3
    trType_t v4; // r11
    trajectory_t *p_apos; // r3

    Entity = GetEntity(entref);
    if (Scr_GetNumParam())
        Scr_GetInt(0);
    trType = Entity->s.lerp.pos.trType;
    p_pos = &Entity->s.lerp.pos;
    if (trType == TR_INTERPOLATE || trType == TR_GRAVITY)
    {
        p_pos->trType = TR_RAGDOLL_GRAVITY;
    }
    else
    {
        p_pos->trType = TR_FIRST_RAGDOLL;
        BG_EvaluateTrajectory(p_pos, level.time, Entity->s.lerp.pos.trBase);
        Entity->s.lerp.pos.trDelta[0] = 0.0;
        Entity->s.lerp.pos.trDelta[1] = 0.0;
        Entity->s.lerp.pos.trDelta[2] = 0.0;
    }
    v4 = Entity->s.lerp.apos.trType;
    p_apos = &Entity->s.lerp.apos;
    if (v4 == TR_INTERPOLATE)
    {
        p_apos->trType = TR_RAGDOLL_INTERPOLATE;
    }
    else if (v4 == TR_GRAVITY)
    {
        p_apos->trType = TR_RAGDOLL_GRAVITY;
    }
    else
    {
        p_apos->trType = TR_FIRST_RAGDOLL;
        BG_EvaluateTrajectory(p_apos, level.time, Entity->s.lerp.apos.trBase);
        Entity->s.lerp.apos.trDelta[0] = 0.0;
        Entity->s.lerp.apos.trDelta[1] = 0.0;
        Entity->s.lerp.apos.trDelta[2] = 0.0;
    }
}

void __cdecl GScr_IsRagdoll(scr_entref_t entref)
{
    gentity_s *Entity; // r3
    unsigned __int8 IsRagdollTrajectory; // r3

    Entity = GetEntity(entref);
    IsRagdollTrajectory = Com_IsRagdollTrajectory(&Entity->s.lerp.pos);
    Scr_AddInt(IsRagdollTrajectory);
}

void(__cdecl *__cdecl BuiltIn_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    int v2; // r6
    unsigned int v3; // r31
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    const char *v6; // r11
    int v7; // r8

    v2 = 0;
    v3 = 0;
    for (i = methods_2; ; ++i)
    {
        actionString = i->actionString;
        v6 = *pName;
        do
        {
            v7 = (unsigned __int8)*v6 - *(unsigned __int8 *)actionString;
            if (!*v6)
                break;
            ++v6;
            ++actionString;
        } while (!v7);
        if (!v7)
            break;
        v3 += 12;
        ++v2;
        if (v3 >= 0x7C8)
            return 0;
    }
    *pName = methods_2[v2].actionString;
    *type = methods_2[v2].type;
    return methods_2[v2].actionFunc;
}

void(__cdecl *__cdecl Scr_GetMethod(const char **pName, int *type))(scr_entref_t __struct_ptr)
{
    void(__cdecl * result)(scr_entref_t  __struct_ptr); // r3

    *type = 0;
    result = Actor_GetMethod(pName);
    if (!result)
    {
        result = Sentient_GetMethod(pName);
        if (!result)
        {
            result = Player_GetMethod(pName);
            if (!result)
            {
                result = ScriptEnt_GetMethod(pName);
                if (!result)
                {
                    result = ScriptVehicle_GetMethod(pName);
                    if (!result)
                    {
                        result = HudElem_GetMethod(pName);
                        if (!result)
                            return BuiltIn_GetMethod(pName, type);
                    }
                }
            }
        }
    }
    return result;
}

void __cdecl Scr_SetOrigin(gentity_s *ent, int offset)
{
    sentient_s *sentient; // r3
    float v4[6]; // [sp+58h] [-28h] BYREF

    if (ent->actor)
        Scr_Error("cannot directly set the origin on AI.  Use the teleport command instead.\n");
    Scr_GetVector(0, v4);
    if ((LODWORD(v4[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(v4[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(v4[2]) & 0x7F800000) == 0x7F800000)
    {
        Scr_Error("origin being set to NAN.");
    }
    G_SetOrigin(ent, v4);
    sentient = ent->sentient;
    if (sentient)
        Sentient_InvalidateNearestNode(sentient);
    if (ent->r.linked)
        SV_LinkEntity(ent);
}

void __cdecl Scr_SetAngles(gentity_s *ent, int offset)
{
    float v3[4]; // [sp+50h] [-20h] BYREF

    if (ent->actor)
        Scr_Error("cannot directly set the angles on AI.  Use the teleport command instead.\n");
    Scr_GetVector(0, v3);
    G_SetAngle(ent, v3);
}

void __cdecl Scr_SetHealth(gentity_s *ent, int offset)
{
    int Int; // r30
    const char *v4; // r6
    const char *v5; // r3
    const char *v6; // r8

    Int = Scr_GetInt(0);
    if (Int <= 0)
    {
        if (ent->targetname)
            v4 = SL_ConvertToString(ent->targetname);
        else
            v4 = "<not set>";
        v5 = va("self.health must be greater than 0 (tried to set %i on ent %i, name %s)", Int, ent->s.number, v4);
        Scr_Error(v5);
    }
    if (ent->health <= 0 && ent->maxHealth)
    {
        if (ent->targetname)
            v6 = SL_ConvertToString(ent->targetname);
        else
            v6 = "<not set>";
        Com_DPrintf(
            23,
            "^2Cannot set health on dead entities (health %i, max %i, ent %i, name %s)\n",
            ent->health,
            ent->maxHealth,
            ent->s.number,
            v6);
    }
    else
    {
        ent->maxHealth = Int;
        ent->health = Int;
    }
}

void __cdecl GScr_Shutdown()
{
    if (level.cachedTagMat.name)
        Scr_SetString(&level.cachedTagMat.name, 0);
}

void __cdecl GScr_SetScriptsAndAnimsForEntities(ScriptFunctions *functions)
{
    int v2; // r15
    int *v3; // r29
    int v4; // r11
    int count; // r11
    int v6; // r31
    int v7; // r11
    int v8; // r11
    int v9; // r31
    int v10; // r11
    int v11; // r11
    int v12; // r31
    const char *classname; // [sp+50h] [-F0h] BYREF
    char filename[224]; // [sp+60h] [-E0h] BYREF

    v2 = 0;
    if (!G_ParseSpawnVars(&level.spawnVar))
        Com_Error(ERR_DROP, "GScr_SetScriptsAndAnimsForEntities: no entities");
    while (G_ParseSpawnVars(&level.spawnVar))
    {
        G_LevelSpawnString("classname", "", &classname);
        if (I_strnicmp(classname, "actor_", 6))
        {
            if (!I_stricmp(classname, "misc_mg42") || !I_stricmp(classname, "misc_turret"))
            {
                SP_turret_XAnimPrecache(functions, classname);
            }
            // LWSS ADD
            else if (!I_stricmp(classname, "node_negotiation_begin"))
            {
                const char *animscript;
                G_LevelSpawnString("animscript", "", &animscript);

                if (animscript[0])
                {
                    if (!Hunk_FindDataForFile(1, animscript))
                    {
                        int func;

                        Com_sprintf(filename, 64, "animscripts/traverse/%s", animscript);
                        if (G_ExitAfterConnectPaths())
                        {
                            func = 0;
                        }
                        else
                        {
                            if (functions->count >= functions->maxSize)
                                Com_Error(ERR_DROP, "CODE ERROR: GScr_SetScriptAndLabel: functions->maxSize exceeded");
                            count = functions->count;
                            func = functions->address[count];
                            functions->count = count + 1;
                            if (!func)
                                Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", "main", filename);
                        }

                        Hunk_SetDataForFile(1, animscript, (void*)func, GScr_AnimscriptAlloc);
                    }
                }
            }
            // LWSS END
        }
        else
        {
            classname += 6;
            if (!Hunk_FindDataForFile(0, classname))
            {
                if (!v2 && I_stristr(classname, "dog"))
                {
                    v2 = 1;
                    GScr_SetDogAnimScripts(functions);
                }
                v3 = (int *)Hunk_AllocLow(12, "GScr_SetScriptsAndAnimsForEntities", 5);
                Com_sprintf(filename, 64, "aitype/%s", classname); // KISAKTODO: some of these sprintf's are only for error printing
                if (G_ExitAfterConnectPaths())
                {
                    v4 = 0;
                }
                else
                {
                    if (functions->count >= functions->maxSize)
                        Com_Error(ERR_DROP, "CODE ERROR: GScr_SetScriptAndLabel: functions->maxSize exceeded");
                    count = functions->count;
                    v6 = functions->address[count];
                    functions->count = count + 1;
                    if (!v6)
                        Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", "main", filename);
                    v4 = v6;
                }
                *v3 = v4;
                if (G_ExitAfterConnectPaths())
                {
                    v7 = 0;
                }
                else
                {
                    if (functions->count >= functions->maxSize)
                        Com_Error(ERR_DROP, "CODE ERROR: GScr_SetScriptAndLabel: functions->maxSize exceeded");
                    v8 = functions->count;
                    v9 = functions->address[v8];
                    functions->count = v8 + 1;
                    if (!v9)
                        Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", "precache", filename);
                    v7 = v9;
                }
                v3[1] = v7;
                if (G_ExitAfterConnectPaths())
                {
                    v10 = 0;
                }
                else
                {
                    if (functions->count >= functions->maxSize)
                        Com_Error(ERR_DROP, "CODE ERROR: GScr_SetScriptAndLabel: functions->maxSize exceeded");
                    v11 = functions->count;
                    v12 = functions->address[v11];
                    functions->count = v11 + 1;
                    if (!v12)
                        Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", "spawner", filename);
                    v10 = v12;
                }
                v3[2] = v10;
                Hunk_SetDataForFile(0, classname, v3, GScr_AnimscriptAlloc);
            }
        }
    }
    G_ResetEntityParsePoint();
}

void __cdecl GScr_SetScripts(ScriptFunctions *functions)
{
    g_scr_data.delete_ = GScr_SetScriptAndLabel(functions, "codescripts/delete", "main", 1);
    g_scr_data.initstructs = GScr_SetScriptAndLabel(functions, "codescripts/struct", "initstructs", 1);
    g_scr_data.createstruct = GScr_SetScriptAndLabel(functions, "codescripts/struct", "createstruct", 1);
    GScr_SetAnimScripts(functions);
    GScr_SetLevelScript(functions);
    GScr_SetScriptsAndAnimsForEntities(functions);
    Path_CallFunctionForNodes(GScr_SetScriptsForPathNode, functions);
    GScr_PostLoadScripts();
    Scr_EndLoadScripts();
    if (!G_ExitAfterConnectPaths())
    {
        Scr_PrecacheAnimTrees(Hunk_AllocXAnimCreate, 1);
        GScr_FindAnimTrees();
    }
    Scr_EndLoadAnimTrees();
}

void __cdecl ScrCmd_GetShootAtPosition(scr_entref_t entref)
{
    gentity_s *Entity; // r30
    const sentient_s *sentient; // r3
    float v3[6]; // [sp+50h] [-30h] BYREF

    Entity = GetEntity(entref);
    sentient = Entity->sentient;
    if (sentient)
    {
        Sentient_GetEyePosition(sentient, v3);
    }
    else if (GScr_UpdateTagInternal(Entity, scr_const.tag_eye, &level.cachedEntTargetTagMat, 0))
    {
        v3[0] = level.cachedEntTargetTagMat.tagMat[3][0];
        v3[1] = level.cachedEntTargetTagMat.tagMat[3][1];
        v3[2] = level.cachedEntTargetTagMat.tagMat[3][2];
    }
    else
    {
        G_EntityCentroid(Entity, v3);
    }
    Scr_AddVector(v3);
}

void __cdecl ScrCmd_animscriptedInternal(scr_entref_t entref, int bDelayForActor)
{
    gentity_s *Entity; // r31
    DObj_s *ServerDObj; // r19
    XAnimTree_s *EntAnimTree; // r28
    __int16 v6; // r20
    unsigned __int16 v7; // r27
    unsigned int v8; // r25
    XAnimTree_s *v9; // r5
    unsigned int ConstString; // r3
    const char *v11; // r3
    const char *v12; // r3
    scr_anim_s v13; // r24
    unsigned int v14; // r3
    actor_s *actor; // r21
    unsigned int v16; // r26
    const char *v17; // r30
    const char *v18; // r17
    double v19; // fp31
    double v20; // fp30
    double v21; // fp29
    const char *v22; // r3
    int v23; // r7
    unsigned int v24; // r6
    animscripted_s *scripted; // r11
    unsigned int v26; // r5
    int flags; // r11
    unsigned __int16 v28; // r3
    tagInfo_s *tagInfo; // r30
    scr_anim_s Anim; // [sp+50h] [-100h]
    float v31[4]; // [sp+58h] [-F8h] BYREF
    float v32[6]; // [sp+68h] [-E8h] BYREF
    float v33[4][3]; // [sp+80h] [-D0h] BYREF

    Entity = GetEntity(entref);
    ServerDObj = Com_GetServerDObj(Entity->s.number);
    if (!ServerDObj)
        Scr_ObjectError("No model exists.");
    EntAnimTree = GScr_GetEntAnimTree(Entity);
    if (!EntAnimTree)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 9930, 0, "%s", "pAnimTree");
    v6 = 0;
    v7 = 0;
    v8 = 0;
    Anim = 0;
    if (Scr_GetNumParam() > 4)
    {
        ConstString = Scr_GetConstString(4);
        v8 = ConstString;
        if (ConstString != scr_const.normal && ConstString != scr_const.deathplant)
        {
            v11 = SL_ConvertToString(ConstString);
            v12 = va("Illegal mode %s for animScripted. Valid modes are normal and deathplant", v11);
            Scr_Error(v12);
        }
        if (Scr_GetNumParam() > 5)
        {
            Anim = Scr_GetAnim(5, EntAnimTree);
            v6 = Anim.tree;
            v7 = Anim.index;
        }
    }
    v13 = Scr_GetAnim(3, EntAnimTree);
    Scr_GetVector(2, v31);
    Scr_GetVector(1, v32);
    v14 = Scr_GetConstString(0);
    actor = Entity->actor;
    v16 = v14;
    if (!actor)
        goto LABEL_32;
    if (!actor->Physics.bIsAlive)
    {
        if (Entity->targetname)
            v17 = SL_ConvertToString(Entity->targetname);
        else
            v17 = "<undefined>";
        v18 = SL_ConvertToString(Entity->classname);
        v19 = Entity->r.currentOrigin[2];
        v20 = Entity->r.currentOrigin[1];
        v21 = Entity->r.currentOrigin[0];
        Sentient_NameForTeam(Entity->sentient->eTeam);
        v22 = va(
            "tried to play a scripted animation on a dead AI; entity %i team %s origin %g %g %g targetname %s classname %s\n",
            Entity->s.number,
            HIDWORD(v21),
            HIDWORD(v20),
            v19,
            v17,
            v18);
        Scr_Error(v22);
    }
    if (bDelayForActor)
    {
        Actor_PushState(actor, AIS_SCRIPTEDANIM);
        Actor_KillAnimScript(actor);
        scripted = Entity->scripted;
        if (scripted)
        {
            v26 = scripted->anim;
            if (scripted->anim)
            {
                if (g_dumpAnimsCommands->current.integer == Entity->s.number)
                    DumpAnimCommand("animscripted", EntAnimTree, v26, -1, 1.0, 0.2, 1.0);
                XAnimSetCompleteGoalWeight(ServerDObj, Entity->scripted->anim, 1.0, 0.2, 1.0, v26, v24, v23);
                if ((Entity->flags & FL_NO_AUTO_ANIM_UPDATE) == 0)
                    Entity->flags |= FL_REPEAT_ANIM_UPDATE;
            }
        }
        if (v6)
            Scr_AddAnim(Anim);
        else
            Scr_AddUndefined();
        if (v8)
            Scr_AddConstString(v8);
        else
            Scr_AddUndefined();
        Scr_AddAnim(v13);
        Scr_AddVector(v31);
        Scr_AddVector(v32);
        Scr_AddConstString(v16);
        v28 = Scr_ExecEntThread(Entity, g_scr_data.scripted_init, 6u);
        Scr_FreeThread(v28);
    }
    else
    {
    LABEL_32:
        if (!v6 && v7)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 9993, 0, "%s", "root.tree || !root.index");
        G_Animscripted(Entity, v32, v31, v13.index, v7, v16, scr_const.deathplant == v8);
    }
    if (!actor || bDelayForActor)
    {
        tagInfo = Entity->tagInfo;
        if (tagInfo)
        {
            G_CalcTagParentAxis(Entity, v33);
            MatrixInverseOrthogonal43(v33, tagInfo->parentInvAxis);
        }
    }
}

void __cdecl G_StopAnimScripted(gentity_s *ent)
{
    actor_s *actor; // r3
    animscripted_s *scripted; // r30
    XAnimTree_s *EntAnimTree; // r29
    int v5; // r7
    unsigned int v6; // r6
    unsigned int v7; // r5
    DObj_s *ServerDObj; // r3

    actor = ent->actor;
    if (actor && actor->eSimulatedState[actor->simulatedStateLevel] == AIS_SCRIPTEDANIM)
        Actor_PopState(actor);
    scripted = ent->scripted;
    if (scripted)
    {
        if (scripted->anim)
        {
            EntAnimTree = GScr_GetEntAnimTree(ent);
            if (!EntAnimTree)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 10036, 0, "%s", "pAnimTree");
            if (g_dumpAnimsCommands->current.integer == ent->s.number)
                DumpAnimCommand("stopanimscripted", EntAnimTree, scripted->anim, -1, 0.0, 0.2, 1.0);
            ServerDObj = Com_GetServerDObj(ent->s.number);
            if (ServerDObj)
                XAnimSetCompleteGoalWeight(ServerDObj, scripted->anim, 0.0, 0.2, 1.0, v7, v6, v5);
        }
        MT_Free((unsigned char*)scripted, 96);
        ent->scripted = 0;
    }
}

void __cdecl ScrCmd_stopanimscripted(scr_entref_t entref)
{
    gentity_s *Entity; // r3

    Entity = GetEntity(entref);
    G_StopAnimScripted(Entity);
}

void __cdecl ScrCmd_animscripted(scr_entref_t entref)
{
    if (Scr_GetNumParam() > 6)
        Scr_Error("too many parameters");
    if (Scr_GetNumParam() > 6 || Scr_GetNumParam() < 4)
        Scr_Error("Incorrect number of parameters for animscripted command");
    ScrCmd_animscriptedInternal(entref, 1);
}

void __cdecl G_SetAnimTree(gentity_s *ent, scr_animtree_t *animtree)
{
    XAnimTree_s *pAnimTree; // r30

    if (G_GetEntAnimTree(ent) != ent->pAnimTree)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp",
            11421,
            0,
            "%s",
            "G_GetEntAnimTree( ent ) == ent->pAnimTree");
    G_StopAnimScripted(ent);
    if (ent->scripted)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_main.cpp", 11424, 0, "%s", "!ent->scripted");
    pAnimTree = ent->pAnimTree;
    if (!animtree)
    {
        if (!pAnimTree)
            return;
        ent->pAnimTree = 0;
        goto LABEL_11;
    }
    if (!pAnimTree || XAnimGetAnims(ent->pAnimTree) != animtree->anims)
    {
        ent->pAnimTree = Com_XAnimCreateSmallTree(animtree->anims);
    LABEL_11:
        G_DObjUpdate(ent);
        if (pAnimTree)
            Com_XAnimFreeSmallTree(pAnimTree);
    }
}

void __cdecl GScr_UseAnimTree(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    unsigned int v2; // r4
    const char *v3; // r3
    const char *v4; // r3
    scr_animtree_t v5; // [sp+50h] [-20h] BYREF

    Entity = GetEntity(entref);
    v5.anims = Scr_GetAnimTree(0);
    if (G_GetEntAnimTree(Entity) != Entity->pAnimTree)
    {
        v3 = SL_ConvertToString(Entity->classname);
        v4 = va("cannot change the animtree of classname '%s'", v3);
        Scr_Error(v4);
    }
    G_SetAnimTree(Entity, &v5);
}

void __cdecl GScr_StopUseAnimTree(scr_entref_t entref)
{
    gentity_s *Entity; // r31
    const char *v2; // r3
    const char *v3; // r3

    Entity = GetEntity(entref);
    if (G_GetEntAnimTree(Entity) != Entity->pAnimTree)
    {
        v2 = SL_ConvertToString(Entity->classname);
        v3 = va("cannot change the animtree of classname '%s'", v2);
        Scr_Error(v3);
    }
    G_SetAnimTree(Entity, 0);
}

void GScr_GetNumVehicles()
{
    int v0; // r3
    int *p_entNum; // r11

    v0 = 0;
    p_entNum = &s_vehicles[1].entNum;
    do
    {
        if (*(p_entNum - 206) != ENTITYNUM_NONE)
            ++v0;
        if (*p_entNum != ENTITYNUM_NONE)
            ++v0;
        if (p_entNum[206] != ENTITYNUM_NONE)
            ++v0;
        if (p_entNum[412] != ENTITYNUM_NONE)
            ++v0;
        p_entNum += 824;
    } while ((uintptr_t)p_entNum < (uintptr_t)&s_vehicles[64]);
    Scr_AddInt(v0);
}

void GScr_PrecacheVehicle()
{
    const char *String; // r31
    int VehicleInfoFromName; // r30
    const char *v2; // r3

    String = Scr_GetString(0);
    if (!level.initializing)
        Scr_Error("precacheVehicle must be called before any wait statements in the level script\n");
    VehicleInfoFromName = VEH_GetVehicleInfoFromName(String);
    if (VehicleInfoFromName < 0)
    {
        v2 = va("Cannot find vehicle info for [%s]\n", String);
        Scr_Error(v2);
    }
    G_GetWeaponIndexForName(s_vehicleInfos[VehicleInfoFromName].turretWeapon);
}