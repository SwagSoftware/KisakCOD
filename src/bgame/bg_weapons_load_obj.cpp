#include <universal/q_shared.h>
#include "bg_local.h"
#include "bg_public.h"
#include <qcommon/mem_track.h>
#include <database/database.h>
#include <universal/q_parse.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <universal/com_sndalias.h>
#include <universal/surfaceflags.h>

//int surfaceTypeSoundListCount 828010f0     bg_weapons_load_obj.obj
//struct SurfaceTypeSoundList *surfaceTypeSoundLists 828011f8     bg_weapons_load_obj.obj

uint32_t g_playerAnimTypeNamesCount;

SurfaceTypeSoundList surfaceTypeSoundLists[16];

const char *stickinessNames[4] =
{
  "Don't stick",
  "Stick to all",
  "Stick to ground",
  "Stick to ground, maintain yaw"
}; // idb
const char *weapIconRatioNames[3] = { "1:1", "2:1", "4:1" }; // idb
const char *ammoCounterClipNames[7] =
{
  "None",
  "Magazine",
  "ShortMagazine",
  "Shotgun",
  "Rocket",
  "Beltfed",
  "AltWeapon"
}; // idb
const char *overlayInterfaceNames[3] = { "None", "Javelin", "Turret Scope" }; // idb
const char *szWeapFireTypeNames[5] =
{
  "Full Auto",
  "Single Shot",
  "2-Round Burst",
  "3-Round Burst",
  "4-Round Burst"
}; // idb
const char *szWeapInventoryTypeNames[4] = { "primary", "offhand", "item", "altmode" }; // idb
const char *penetrateTypeNames[4] = { "none", "small", "medium", "large" }; // idb
const char *szWeapOverlayReticleNames[2] = { "none", "crosshair" }; // idb
const char *szWeapStanceNames[3] = { "stand", "duck", "prone" }; // idb
const char *accuracyDirName[3] = { "aivsai", "aivsplayer", NULL }; // idb
const char *activeReticleNames[3] = { "None", "Pip-On-A-Stick", "Bouncing diamond" }; // idb
const char *szWeapTypeNames[4] = { "bullet", "grenade", "projectile", "binoculars" }; // idb
const char *guidedMissileNames[4] = { "None", "Sidewinder", "Hellfire", "Javelin" }; // idb
const char *offhandClassNames[4] = { "None", "Frag Grenade", "Smoke Grenade", "Flash Grenade" }; // idb
const char *szProjectileExplosionNames[7] = { "grenade", "rocket", "flashbang", "none", "dud", "smoke", "heavy explosive" }; // idb

const char *impactTypeNames[9] =
{
  "none",
  "bullet_small",
  "bullet_large",
  "bullet_ap",
  "shotgun",
  "grenade_bounce",
  "grenade_explode",
  "rocket_explode",
  "projectile_dud"
}; // idb

cspField_t weaponDefFields[502] =
{
  { "displayName", 4, CSPFT_STRING },
  { "AIOverlayDescription", 8, CSPFT_STRING },
  { "modeName", 212, CSPFT_STRING },
  { "playerAnimType", 296, WFT_ANIMTYPE },
  { "gunModel", 12, CSPFT_XMODEL },
  { "gunModel2", 16, CSPFT_XMODEL },
  { "gunModel3", 20, CSPFT_XMODEL },
  { "gunModel4", 24, CSPFT_XMODEL },
  { "gunModel5", 28, CSPFT_XMODEL },
  { "gunModel6", 32, CSPFT_XMODEL },
  { "gunModel7", 36, CSPFT_XMODEL },
  { "gunModel8", 40, CSPFT_XMODEL },
  { "gunModel9", 44, CSPFT_XMODEL },
  { "gunModel10", 48, CSPFT_XMODEL },
  { "gunModel11", 52, CSPFT_XMODEL },
  { "gunModel12", 56, CSPFT_XMODEL },
  { "gunModel13", 60, CSPFT_XMODEL },
  { "gunModel14", 64, CSPFT_XMODEL },
  { "gunModel15", 68, CSPFT_XMODEL },
  { "gunModel16", 72, CSPFT_XMODEL },
  { "handModel", 76, CSPFT_XMODEL },
  { "hideTags", 216, WFT_HIDETAGS },
  { "notetrackSoundMap", 232, WFT_NOTETRACKSOUNDMAP },
  { "idleAnim", 84, CSPFT_STRING },
  { "emptyIdleAnim", 88, CSPFT_STRING },
  { "fireAnim", 92, CSPFT_STRING },
  { "holdFireAnim", 96, CSPFT_STRING },
  { "lastShotAnim", 100, CSPFT_STRING },
  { "detonateAnim", 180, CSPFT_STRING },
  { "rechamberAnim", 104, CSPFT_STRING },
  { "meleeAnim", 108, CSPFT_STRING },
  { "meleeChargeAnim", 112, CSPFT_STRING },
  { "reloadAnim", 116, CSPFT_STRING },
  { "reloadEmptyAnim", 120, CSPFT_STRING },
  { "reloadStartAnim", 124, CSPFT_STRING },
  { "reloadEndAnim", 128, CSPFT_STRING },
  { "raiseAnim", 132, CSPFT_STRING },
  { "dropAnim", 140, CSPFT_STRING },
  { "firstRaiseAnim", 136, CSPFT_STRING },
  { "altRaiseAnim", 144, CSPFT_STRING },
  { "altDropAnim", 148, CSPFT_STRING },
  { "quickRaiseAnim", 152, CSPFT_STRING },
  { "quickDropAnim", 156, CSPFT_STRING },
  { "emptyRaiseAnim", 160, CSPFT_STRING },
  { "emptyDropAnim", 164, CSPFT_STRING },
  { "sprintInAnim", 168, CSPFT_STRING },
  { "sprintLoopAnim", 172, CSPFT_STRING },
  { "sprintOutAnim", 176, CSPFT_STRING },
  { "nightVisionWearAnim", 184, CSPFT_STRING },
  { "nightVisionRemoveAnim", 188, CSPFT_STRING },
  { "adsFireAnim", 192, CSPFT_STRING },
  { "adsLastShotAnim", 196, CSPFT_STRING },
  { "adsRechamberAnim", 200, CSPFT_STRING },
  { "adsUpAnim", 204, CSPFT_STRING },
  { "adsDownAnim", 208, CSPFT_STRING },
  { "script", 2036, CSPFT_STRING },
  { "weaponType", 300, WFT_WEAPONTYPE },
  { "weaponClass", 304, WFT_WEAPONCLASS },
  { "penetrateType", 308, WFT_PENETRATE_TYPE },
  { "impactType", 312, WFT_IMPACT_TYPE },
  { "inventoryType", 316, WFT_INVENTORYTYPE },
  { "fireType", 320, WFT_FIRETYPE },
  { "offhandClass", 324, WFT_OFFHAND_CLASS },
  { "viewFlashEffect", 332, CSPFT_FX },
  { "worldFlashEffect", 336, CSPFT_FX },
  { "pickupSound", 340, CSPFT_SOUND },
  { "pickupSoundPlayer", 344, CSPFT_SOUND },
  { "ammoPickupSound", 348, CSPFT_SOUND },
  { "ammoPickupSoundPlayer", 352, CSPFT_SOUND },
  { "projectileSound", 356, CSPFT_SOUND },
  { "pullbackSound", 360, CSPFT_SOUND },
  { "pullbackSoundPlayer", 364, CSPFT_SOUND },
  { "fireSound", 368, CSPFT_SOUND },
  { "fireSoundPlayer", 372, CSPFT_SOUND },
  { "loopFireSound", 376, CSPFT_SOUND },
  { "loopFireSoundPlayer", 380, CSPFT_SOUND },
  { "stopFireSound", 384, CSPFT_SOUND },
  { "stopFireSoundPlayer", 388, CSPFT_SOUND },
  { "lastShotSound", 392, CSPFT_SOUND },
  { "lastShotSoundPlayer", 396, CSPFT_SOUND },
  { "emptyFireSound", 400, CSPFT_SOUND },
  { "emptyFireSoundPlayer", 404, CSPFT_SOUND },
  { "meleeSwipeSound", 408, CSPFT_SOUND },
  { "meleeSwipeSoundPlayer", 412, CSPFT_SOUND },
  { "meleeHitSound", 416, CSPFT_SOUND },
  { "meleeMissSound", 420, CSPFT_SOUND },
  { "rechamberSound", 424, CSPFT_SOUND },
  { "rechamberSoundPlayer", 428, CSPFT_SOUND },
  { "reloadSound", 432, CSPFT_SOUND },
  { "reloadSoundPlayer", 436, CSPFT_SOUND },
  { "reloadEmptySound", 440, CSPFT_SOUND },
  { "reloadEmptySoundPlayer", 444, CSPFT_SOUND },
  { "reloadStartSound", 448, CSPFT_SOUND },
  { "reloadStartSoundPlayer", 452, CSPFT_SOUND },
  { "reloadEndSound", 456, CSPFT_SOUND },
  { "reloadEndSoundPlayer", 460, CSPFT_SOUND },
  { "detonateSound", 464, CSPFT_SOUND },
  { "detonateSoundPlayer", 468, CSPFT_SOUND },
  { "nightVisionWearSound", 472, CSPFT_SOUND },
  { "nightVisionWearSoundPlayer", 476, CSPFT_SOUND },
  { "nightVisionRemoveSound", 480, CSPFT_SOUND },
  { "nightVisionRemoveSoundPlayer", 484, CSPFT_SOUND },
  { "raiseSound", 496, CSPFT_SOUND },
  { "raiseSoundPlayer", 500, CSPFT_SOUND },
  { "firstRaiseSound", 504, CSPFT_SOUND },
  { "firstRaiseSoundPlayer", 508, CSPFT_SOUND },
  { "altSwitchSound", 488, CSPFT_SOUND },
  { "altSwitchSoundPlayer", 492, CSPFT_SOUND },
  { "putawaySound", 512, CSPFT_SOUND },
  { "putawaySoundPlayer", 516, CSPFT_SOUND },
  { "bounceSound", 520, WFT_BOUNCE_SOUND },
  { "viewShellEjectEffect", 524, CSPFT_FX },
  { "worldShellEjectEffect", 528, CSPFT_FX },
  { "viewLastShotEjectEffect", 532, CSPFT_FX },
  { "worldLastShotEjectEffect", 536, CSPFT_FX },
  { "reticleCenter", 540, CSPFT_MATERIAL },
  { "reticleSide", 544, CSPFT_MATERIAL },
  { "reticleCenterSize", 548, CSPFT_INT },
  { "reticleSideSize", 552, CSPFT_INT },
  { "reticleMinOfs", 556, CSPFT_INT },
  { "activeReticleType", 560, WFT_ACTIVE_RETICLE_TYPE },
  { "standMoveF", 564, CSPFT_FLOAT },
  { "standMoveR", 568, CSPFT_FLOAT },
  { "standMoveU", 572, CSPFT_FLOAT },
  { "standRotP", 576, CSPFT_FLOAT },
  { "standRotY", 580, CSPFT_FLOAT },
  { "standRotR", 584, CSPFT_FLOAT },
  { "duckedOfsF", 588, CSPFT_FLOAT },
  { "duckedOfsR", 592, CSPFT_FLOAT },
  { "duckedOfsU", 596, CSPFT_FLOAT },
  { "duckedMoveF", 600, CSPFT_FLOAT },
  { "duckedMoveR", 604, CSPFT_FLOAT },
  { "duckedMoveU", 608, CSPFT_FLOAT },
  { "duckedRotP", 612, CSPFT_FLOAT },
  { "duckedRotY", 616, CSPFT_FLOAT },
  { "duckedRotR", 620, CSPFT_FLOAT },
  { "proneOfsF", 624, CSPFT_FLOAT },
  { "proneOfsR", 628, CSPFT_FLOAT },
  { "proneOfsU", 632, CSPFT_FLOAT },
  { "proneMoveF", 636, CSPFT_FLOAT },
  { "proneMoveR", 640, CSPFT_FLOAT },
  { "proneMoveU", 644, CSPFT_FLOAT },
  { "proneRotP", 648, CSPFT_FLOAT },
  { "proneRotY", 652, CSPFT_FLOAT },
  { "proneRotR", 656, CSPFT_FLOAT },
  { "posMoveRate", 660, CSPFT_FLOAT },
  { "posProneMoveRate", 664, CSPFT_FLOAT },
  { "standMoveMinSpeed", 668, CSPFT_FLOAT },
  { "duckedMoveMinSpeed", 672, CSPFT_FLOAT },
  { "proneMoveMinSpeed", 676, CSPFT_FLOAT },
  { "posRotRate", 680, CSPFT_FLOAT },
  { "posProneRotRate", 684, CSPFT_FLOAT },
  { "standRotMinSpeed", 688, CSPFT_FLOAT },
  { "duckedRotMinSpeed", 692, CSPFT_FLOAT },
  { "proneRotMinSpeed", 696, CSPFT_FLOAT },
  { "worldModel", 700, CSPFT_XMODEL },
  { "worldModel2", 704, CSPFT_XMODEL },
  { "worldModel3", 708, CSPFT_XMODEL },
  { "worldModel4", 712, CSPFT_XMODEL },
  { "worldModel5", 716, CSPFT_XMODEL },
  { "worldModel6", 720, CSPFT_XMODEL },
  { "worldModel7", 724, CSPFT_XMODEL },
  { "worldModel8", 728, CSPFT_XMODEL },
  { "worldModel9", 732, CSPFT_XMODEL },
  { "worldModel10", 736, CSPFT_XMODEL },
  { "worldModel11", 740, CSPFT_XMODEL },
  { "worldModel12", 744, CSPFT_XMODEL },
  { "worldModel13", 748, CSPFT_XMODEL },
  { "worldModel14", 752, CSPFT_XMODEL },
  { "worldModel15", 756, CSPFT_XMODEL },
  { "worldModel16", 760, CSPFT_XMODEL },
  { "worldClipModel", 764, CSPFT_XMODEL },
  { "rocketModel", 768, CSPFT_XMODEL },
  { "knifeModel", 772, CSPFT_XMODEL },
  { "worldKnifeModel", 776, CSPFT_XMODEL },
  { "hudIcon", 780, CSPFT_MATERIAL },
  { "hudIconRatio", 784, WFT_ICONRATIO_HUD },
  { "ammoCounterIcon", 788, CSPFT_MATERIAL },
  { "ammoCounterIconRatio", 792, WFT_ICONRATIO_AMMOCOUNTER },
  { "ammoCounterClip", 796, WFT_AMMOCOUNTER_CLIPTYPE },
  { "startAmmo", 800, CSPFT_INT },
  { "ammoName", 804, CSPFT_STRING },
  { "clipName", 812, CSPFT_STRING },
  { "maxAmmo", 820, CSPFT_INT },
  { "clipSize", 824, CSPFT_INT },
  { "shotCount", 828, CSPFT_INT },
  { "sharedAmmoCapName", 832, CSPFT_STRING },
  { "sharedAmmoCap", 840, CSPFT_INT },
  { "damage", 844, CSPFT_INT },
  { "playerDamage", 848, CSPFT_INT },
  { "meleeDamage", 852, CSPFT_INT },
  { "minDamage", 2048, CSPFT_INT },
  { "minPlayerDamage", 2052, CSPFT_INT },
  { "maxDamageRange", 2056, CSPFT_FLOAT },
  { "minDamageRange", 2060, CSPFT_FLOAT },
  { "destabilizationRateTime", 2064, CSPFT_FLOAT },
  { "destabilizationCurvatureMax", 2068, CSPFT_FLOAT },
  { "destabilizeDistance", 2072, CSPFT_INT },
  { "fireDelay", 860, CSPFT_MILLISECONDS },
  { "meleeDelay", 864, CSPFT_MILLISECONDS },
  { "meleeChargeDelay", 868, CSPFT_MILLISECONDS },
  { "fireTime", 876, CSPFT_MILLISECONDS },
  { "rechamberTime", 880, CSPFT_MILLISECONDS },
  { "rechamberBoltTime", 884, CSPFT_MILLISECONDS },
  { "holdFireTime", 888, CSPFT_MILLISECONDS },
  { "detonateTime", 892, CSPFT_MILLISECONDS },
  { "detonateDelay", 872, CSPFT_MILLISECONDS },
  { "meleeTime", 896, CSPFT_MILLISECONDS },
  { "meleeChargeTime", 900, CSPFT_MILLISECONDS },
  { "reloadTime", 904, CSPFT_MILLISECONDS },
  { "reloadShowRocketTime", 908, CSPFT_MILLISECONDS },
  { "reloadEmptyTime", 912, CSPFT_MILLISECONDS },
  { "reloadAddTime", 916, CSPFT_MILLISECONDS },
  { "reloadStartTime", 920, CSPFT_MILLISECONDS },
  { "reloadStartAddTime", 924, CSPFT_MILLISECONDS },
  { "reloadEndTime", 928, CSPFT_MILLISECONDS },
  { "dropTime", 932, CSPFT_MILLISECONDS },
  { "raiseTime", 936, CSPFT_MILLISECONDS },
  { "altDropTime", 940, CSPFT_MILLISECONDS },
  { "altRaiseTime", 944, CSPFT_MILLISECONDS },
  { "quickDropTime", 948, CSPFT_MILLISECONDS },
  { "quickRaiseTime", 952, CSPFT_MILLISECONDS },
  { "firstRaiseTime", 956, CSPFT_MILLISECONDS },
  { "emptyRaiseTime", 960, CSPFT_MILLISECONDS },
  { "emptyDropTime", 964, CSPFT_MILLISECONDS },
  { "sprintInTime", 968, CSPFT_MILLISECONDS },
  { "sprintLoopTime", 972, CSPFT_MILLISECONDS },
  { "sprintOutTime", 976, CSPFT_MILLISECONDS },
  { "nightVisionWearTime", 980, CSPFT_MILLISECONDS },
  { "nightVisionWearTimeFadeOutEnd", 984, CSPFT_MILLISECONDS },
  { "nightVisionWearTimePowerUp", 988, CSPFT_MILLISECONDS },
  { "nightVisionRemoveTime", 992, CSPFT_MILLISECONDS },
  { "nightVisionRemoveTimePowerDown", 996, CSPFT_MILLISECONDS },
  { "nightVisionRemoveTimeFadeInStart", 1000, CSPFT_MILLISECONDS },
  { "fuseTime", 1004, CSPFT_MILLISECONDS },
  { "aifuseTime", 1008, CSPFT_MILLISECONDS },
  { "requireLockonToFire", 1012, CSPFT_QBOOLEAN },
  { "noAdsWhenMagEmpty", 1016, CSPFT_QBOOLEAN },
  { "avoidDropCleanup", 1020, CSPFT_QBOOLEAN },
  { "autoAimRange", 1024, CSPFT_FLOAT },
  { "aimAssistRange", 1028, CSPFT_FLOAT },
  { "aimAssistRangeAds", 1032, CSPFT_FLOAT },
  { "aimPadding", 1036, CSPFT_FLOAT },
  { "enemyCrosshairRange", 1040, CSPFT_FLOAT },
  { "crosshairColorChange", 1044, CSPFT_QBOOLEAN },
  { "moveSpeedScale", 1048, CSPFT_FLOAT },
  { "adsMoveSpeedScale", 1052, CSPFT_FLOAT },
  { "sprintDurationScale", 1056, CSPFT_FLOAT },
  { "idleCrouchFactor", 1180, CSPFT_FLOAT },
  { "idleProneFactor", 1184, CSPFT_FLOAT },
  { "gunMaxPitch", 1188, CSPFT_FLOAT },
  { "gunMaxYaw", 1192, CSPFT_FLOAT },
  { "swayMaxAngle", 1196, CSPFT_FLOAT },
  { "swayLerpSpeed", 1200, CSPFT_FLOAT },
  { "swayPitchScale", 1204, CSPFT_FLOAT },
  { "swayYawScale", 1208, CSPFT_FLOAT },
  { "swayHorizScale", 1212, CSPFT_FLOAT },
  { "swayVertScale", 1216, CSPFT_FLOAT },
  { "swayShellShockScale", 1220, CSPFT_FLOAT },
  { "adsSwayMaxAngle", 1224, CSPFT_FLOAT },
  { "adsSwayLerpSpeed", 1228, CSPFT_FLOAT },
  { "adsSwayPitchScale", 1232, CSPFT_FLOAT },
  { "adsSwayYawScale", 1236, CSPFT_FLOAT },
  { "adsSwayHorizScale", 1240, CSPFT_FLOAT },
  { "adsSwayVertScale", 1244, CSPFT_FLOAT },
  { "rifleBullet", 1248, CSPFT_QBOOLEAN },
  { "armorPiercing", 1252, CSPFT_QBOOLEAN },
  { "boltAction", 1256, CSPFT_QBOOLEAN },
  { "aimDownSight", 1260, CSPFT_QBOOLEAN },
  { "rechamberWhileAds", 1264, CSPFT_QBOOLEAN },
  { "adsViewErrorMin", 1268, CSPFT_FLOAT },
  { "adsViewErrorMax", 1272, CSPFT_FLOAT },
  { "clipOnly", 1280, CSPFT_QBOOLEAN },
  { "cookOffHold", 1276, CSPFT_QBOOLEAN },
  { "adsFire", 1284, CSPFT_QBOOLEAN },
  { "cancelAutoHolsterWhenEmpty", 1288, CSPFT_QBOOLEAN },
  { "suppressAmmoReserveDisplay", 1292, CSPFT_QBOOLEAN },
  { "enhanced", 1296, CSPFT_QBOOLEAN },
  { "laserSightDuringNightvision", 1300, CSPFT_QBOOLEAN },
  { "killIcon", 1304, CSPFT_MATERIAL },
  { "killIconRatio", 1308, WFT_ICONRATIO_KILL },
  { "flipKillIcon", 1312, CSPFT_QBOOLEAN },
  { "dpadIcon", 1316, CSPFT_MATERIAL },
  { "dpadIconRatio", 1320, WFT_ICONRATIO_DPAD },
  { "noPartialReload", 1324, CSPFT_QBOOLEAN },
  { "segmentedReload", 1328, CSPFT_QBOOLEAN },
  { "reloadAmmoAdd", 1332, CSPFT_INT },
  { "reloadStartAdd", 1336, CSPFT_INT },
  { "altWeapon", 1340, CSPFT_STRING },
  { "dropAmmoMin", 1348, CSPFT_INT },
  { "dropAmmoMax", 1352, CSPFT_INT },
  { "blocksProne", 1356, CSPFT_QBOOLEAN },
  { "silenced", 1360, CSPFT_QBOOLEAN },
  { "explosionRadius", 1364, CSPFT_INT },
  { "explosionRadiusMin", 1368, CSPFT_INT },
  { "explosionInnerDamage", 1372, CSPFT_INT },
  { "explosionOuterDamage", 1376, CSPFT_INT },
  { "damageConeAngle", 1380, CSPFT_FLOAT },
  { "projectileSpeed", 1384, CSPFT_INT },
  { "projectileSpeedUp", 1388, CSPFT_INT },
  { "projectileSpeedForward", 1392, CSPFT_INT },
  { "projectileActivateDist", 1396, CSPFT_INT },
  { "projectileLifetime", 1400, CSPFT_FLOAT },
  { "timeToAccelerate", 1404, CSPFT_FLOAT },
  { "projectileCurvature", 1408, CSPFT_FLOAT },
  { "projectileModel", 1412, CSPFT_XMODEL },
  { "projExplosionType", 1416, WFT_PROJ_EXPLOSION },
  { "projExplosionEffect", 1420, CSPFT_FX },
  { "projExplosionEffectForceNormalUp", 1424, CSPFT_QBOOLEAN },
  { "projExplosionSound", 1432, CSPFT_SOUND },
  { "projDudEffect", 1428, CSPFT_FX },
  { "projDudSound", 1436, CSPFT_SOUND },
  { "projImpactExplode", 1440, CSPFT_QBOOLEAN },
  { "stickiness", 1444, WFT_STICKINESS },
  { "hasDetonator", 1448, CSPFT_QBOOLEAN },
  { "timedDetonation", 1452, CSPFT_QBOOLEAN },
  { "rotate", 1456, CSPFT_QBOOLEAN },
  { "holdButtonToThrow", 1460, CSPFT_QBOOLEAN },
  { "freezeMovementWhenFiring", 1464, CSPFT_QBOOLEAN },
  { "lowAmmoWarningThreshold", 1468, CSPFT_FLOAT },
  { "parallelDefaultBounce", 1472, CSPFT_FLOAT },
  { "parallelBarkBounce", 1476, CSPFT_FLOAT },
  { "parallelBrickBounce", 1480, CSPFT_FLOAT },
  { "parallelCarpetBounce", 1484, CSPFT_FLOAT },
  { "parallelClothBounce", 1488, CSPFT_FLOAT },
  { "parallelConcreteBounce", 1492, CSPFT_FLOAT },
  { "parallelDirtBounce", 1496, CSPFT_FLOAT },
  { "parallelFleshBounce", 1500, CSPFT_FLOAT },
  { "parallelFoliageBounce", 1504, CSPFT_FLOAT },
  { "parallelGlassBounce", 1508, CSPFT_FLOAT },
  { "parallelGrassBounce", 1512, CSPFT_FLOAT },
  { "parallelGravelBounce", 1516, CSPFT_FLOAT },
  { "parallelIceBounce", 1520, CSPFT_FLOAT },
  { "parallelMetalBounce", 1524, CSPFT_FLOAT },
  { "parallelMudBounce", 1528, CSPFT_FLOAT },
  { "parallelPaperBounce", 1532, CSPFT_FLOAT },
  { "parallelPlasterBounce", 1536, CSPFT_FLOAT },
  { "parallelRockBounce", 1540, CSPFT_FLOAT },
  { "parallelSandBounce", 1544, CSPFT_FLOAT },
  { "parallelSnowBounce", 1548, CSPFT_FLOAT },
  { "parallelWaterBounce", 1552, CSPFT_FLOAT },
  { "parallelWoodBounce", 1556, CSPFT_FLOAT },
  { "parallelAsphaltBounce", 1560, CSPFT_FLOAT },
  { "parallelCeramicBounce", 1564, CSPFT_FLOAT },
  { "parallelPlasticBounce", 1568, CSPFT_FLOAT },
  { "parallelRubberBounce", 1572, CSPFT_FLOAT },
  { "parallelCushionBounce", 1576, CSPFT_FLOAT },
  { "parallelFruitBounce", 1580, CSPFT_FLOAT },
  { "parallelPaintedMetalBounce", 1584, CSPFT_FLOAT },
  { "perpendicularDefaultBounce", 1588, CSPFT_FLOAT },
  { "perpendicularBarkBounce", 1592, CSPFT_FLOAT },
  { "perpendicularBrickBounce", 1596, CSPFT_FLOAT },
  { "perpendicularCarpetBounce", 1600, CSPFT_FLOAT },
  { "perpendicularClothBounce", 1604, CSPFT_FLOAT },
  { "perpendicularConcreteBounce", 1608, CSPFT_FLOAT },
  { "perpendicularDirtBounce", 1612, CSPFT_FLOAT },
  { "perpendicularFleshBounce", 1616, CSPFT_FLOAT },
  { "perpendicularFoliageBounce", 1620, CSPFT_FLOAT },
  { "perpendicularGlassBounce", 1624, CSPFT_FLOAT },
  { "perpendicularGrassBounce", 1628, CSPFT_FLOAT },
  { "perpendicularGravelBounce", 1632, CSPFT_FLOAT },
  { "perpendicularIceBounce", 1636, CSPFT_FLOAT },
  { "perpendicularMetalBounce", 1640, CSPFT_FLOAT },
  { "perpendicularMudBounce", 1644, CSPFT_FLOAT },
  { "perpendicularPaperBounce", 1648, CSPFT_FLOAT },
  { "perpendicularPlasterBounce", 1652, CSPFT_FLOAT },
  { "perpendicularRockBounce", 1656, CSPFT_FLOAT },
  { "perpendicularSandBounce", 1660, CSPFT_FLOAT },
  { "perpendicularSnowBounce", 1664, CSPFT_FLOAT },
  { "perpendicularWaterBounce", 1668, CSPFT_FLOAT },
  { "perpendicularWoodBounce", 1672, CSPFT_FLOAT },
  { "perpendicularAsphaltBounce", 1676, CSPFT_FLOAT },
  { "perpendicularCeramicBounce", 1564, CSPFT_FLOAT },
  { "perpendicularPlasticBounce", 1568, CSPFT_FLOAT },
  { "perpendicularRubberBounce", 1572, CSPFT_FLOAT },
  { "perpendicularCushionBounce", 1692, CSPFT_FLOAT },
  { "perpendicularFruitBounce", 1696, CSPFT_FLOAT },
  { "perpendicularPaintedMetalBounce", 1700, CSPFT_FLOAT },
  { "projTrailEffect", 1704, CSPFT_FX },
  { "projectileRed", 1708, CSPFT_FLOAT },
  { "projectileGreen", 1712, CSPFT_FLOAT },
  { "projectileBlue", 1716, CSPFT_FLOAT },
  { "guidedMissileType", 1720, WFT_GUIDED_MISSILE_TYPE },
  { "maxSteeringAccel", 1724, CSPFT_FLOAT },
  { "projIgnitionDelay", 1728, CSPFT_INT },
  { "projIgnitionEffect", 1732, CSPFT_FX },
  { "projIgnitionSound", 1736, CSPFT_SOUND },
  { "adsTransInTime", 1156, CSPFT_MILLISECONDS },
  { "adsTransOutTime", 1160, CSPFT_MILLISECONDS },
  { "adsIdleAmount", 1164, CSPFT_FLOAT },
  { "adsIdleSpeed", 1172, CSPFT_FLOAT },
  { "adsZoomFov", 1060, CSPFT_FLOAT },
  { "adsZoomInFrac", 1064, CSPFT_FLOAT },
  { "adsZoomOutFrac", 1068, CSPFT_FLOAT },
  { "adsOverlayShader", 1072, CSPFT_MATERIAL },
  { "adsOverlayShaderLowRes", 1076, CSPFT_MATERIAL },
  { "adsOverlayReticle", 1080, WFT_OVERLAYRETICLE },
  { "adsOverlayInterface", 1084, WFT_OVERLAYINTERFACE },
  { "adsOverlayWidth", 1088, CSPFT_FLOAT },
  { "adsOverlayHeight", 1092, CSPFT_FLOAT },
  { "adsBobFactor", 1096, CSPFT_FLOAT },
  { "adsViewBobMult", 1100, CSPFT_FLOAT },
  { "adsAimPitch", 1740, CSPFT_FLOAT },
  { "adsCrosshairInFrac", 1744, CSPFT_FLOAT },
  { "adsCrosshairOutFrac", 1748, CSPFT_FLOAT },
  { "adsReloadTransTime", 1940, CSPFT_MILLISECONDS },
  { "adsGunKickReducedKickBullets", 1752, CSPFT_INT },
  { "adsGunKickReducedKickPercent", 1756, CSPFT_FLOAT },
  { "adsGunKickPitchMin", 1760, CSPFT_FLOAT },
  { "adsGunKickPitchMax", 1764, CSPFT_FLOAT },
  { "adsGunKickYawMin", 1768, CSPFT_FLOAT },
  { "adsGunKickYawMax", 1772, CSPFT_FLOAT },
  { "adsGunKickAccel", 1776, CSPFT_FLOAT },
  { "adsGunKickSpeedMax", 1780, CSPFT_FLOAT },
  { "adsGunKickSpeedDecay", 1784, CSPFT_FLOAT },
  { "adsGunKickStaticDecay", 1788, CSPFT_FLOAT },
  { "adsViewKickPitchMin", 1792, CSPFT_FLOAT },
  { "adsViewKickPitchMax", 1796, CSPFT_FLOAT },
  { "adsViewKickYawMin", 1800, CSPFT_FLOAT },
  { "adsViewKickYawMax", 1804, CSPFT_FLOAT },
  { "adsViewKickCenterSpeed", 1808, CSPFT_FLOAT },
  { "adsSpread", 1820, CSPFT_FLOAT },
  { "guidedMissileType", 1720, WFT_GUIDED_MISSILE_TYPE },
  { "hipSpreadStandMin", 1104, CSPFT_FLOAT },
  { "hipSpreadDuckedMin", 1108, CSPFT_FLOAT },
  { "hipSpreadProneMin", 1112, CSPFT_FLOAT },
  { "hipSpreadMax", 1116, CSPFT_FLOAT },
  { "hipSpreadDuckedMax", 1120, CSPFT_FLOAT },
  { "hipSpreadProneMax", 1124, CSPFT_FLOAT },
  { "hipSpreadDecayRate", 1128, CSPFT_FLOAT },
  { "hipSpreadFireAdd", 1132, CSPFT_FLOAT },
  { "hipSpreadTurnAdd", 1136, CSPFT_FLOAT },
  { "hipSpreadMoveAdd", 1140, CSPFT_FLOAT },
  { "hipSpreadDuckedDecay", 1144, CSPFT_FLOAT },
  { "hipSpreadProneDecay", 1148, CSPFT_FLOAT },
  { "hipReticleSidePos", 1152, CSPFT_FLOAT },
  { "hipIdleAmount", 1168, CSPFT_FLOAT },
  { "hipIdleSpeed", 1176, CSPFT_FLOAT },
  { "hipGunKickReducedKickBullets", 1824, CSPFT_INT },
  { "hipGunKickReducedKickPercent", 1828, CSPFT_FLOAT },
  { "hipGunKickPitchMin", 1832, CSPFT_FLOAT },
  { "hipGunKickPitchMax", 1836, CSPFT_FLOAT },
  { "hipGunKickYawMin", 1840, CSPFT_FLOAT },
  { "hipGunKickYawMax", 1844, CSPFT_FLOAT },
  { "hipGunKickAccel", 1848, CSPFT_FLOAT },
  { "hipGunKickSpeedMax", 1852, CSPFT_FLOAT },
  { "hipGunKickSpeedDecay", 1856, CSPFT_FLOAT },
  { "hipGunKickStaticDecay", 1860, CSPFT_FLOAT },
  { "hipViewKickPitchMin", 1864, CSPFT_FLOAT },
  { "hipViewKickPitchMax", 1868, CSPFT_FLOAT },
  { "hipViewKickYawMin", 1872, CSPFT_FLOAT },
  { "hipViewKickYawMax", 1876, CSPFT_FLOAT },
  { "hipViewKickCenterSpeed", 1880, CSPFT_FLOAT },
  { "leftArc", 1944, CSPFT_FLOAT },
  { "rightArc", 1948, CSPFT_FLOAT },
  { "topArc", 1952, CSPFT_FLOAT },
  { "bottomArc", 1956, CSPFT_FLOAT },
  { "accuracy", 1960, CSPFT_FLOAT },
  { "aiSpread", 1964, CSPFT_FLOAT },
  { "playerSpread", 1968, CSPFT_FLOAT },
  { "maxVertTurnSpeed", 1980, CSPFT_FLOAT },
  { "maxHorTurnSpeed", 1984, CSPFT_FLOAT },
  { "minVertTurnSpeed", 1972, CSPFT_FLOAT },
  { "minHorTurnSpeed", 1976, CSPFT_FLOAT },
  { "pitchConvergenceTime", 1988, CSPFT_FLOAT },
  { "yawConvergenceTime", 1992, CSPFT_FLOAT },
  { "suppressionTime", 1996, CSPFT_FLOAT },
  { "maxRange", 2000, CSPFT_FLOAT },
  { "animHorRotateInc", 2004, CSPFT_FLOAT },
  { "playerPositionDist", 2008, CSPFT_FLOAT },
  { "stance", 328, WFT_STANCE },
  { "useHintString", 2012, CSPFT_STRING },
  { "dropHintString", 2016, CSPFT_STRING },
  { "horizViewJitter", 2028, CSPFT_FLOAT },
  { "vertViewJitter", 2032, CSPFT_FLOAT },
  { "fightDist", 1892, CSPFT_FLOAT },
  { "maxDist", 1896, CSPFT_FLOAT },
  { "aiVsAiAccuracyGraph", 1900, CSPFT_STRING },
  { "aiVsPlayerAccuracyGraph", 1904, CSPFT_STRING },
  { "locNone", 2076, CSPFT_FLOAT },
  { "locHelmet", 2080, CSPFT_FLOAT },
  { "locHead", 2084, CSPFT_FLOAT },
  { "locNeck", 2088, CSPFT_FLOAT },
  { "locTorsoUpper", 2092, CSPFT_FLOAT },
  { "locTorsoLower", 2096, CSPFT_FLOAT },
  { "locRightArmUpper", 2100, CSPFT_FLOAT },
  { "locRightArmLower", 2108, CSPFT_FLOAT },
  { "locRightHand", 2116, CSPFT_FLOAT },
  { "locLeftArmUpper", 2104, CSPFT_FLOAT },
  { "locLeftArmLower", 2112, CSPFT_FLOAT },
  { "locLeftHand", 2120, CSPFT_FLOAT },
  { "locRightLegUpper", 2124, CSPFT_FLOAT },
  { "locRightLegLower", 2132, CSPFT_FLOAT },
  { "locRightFoot", 2140, CSPFT_FLOAT },
  { "locLeftLegUpper", 2128, CSPFT_FLOAT },
  { "locLeftLegLower", 2136, CSPFT_FLOAT },
  { "locLeftFoot", 2144, CSPFT_FLOAT },
  { "locGun", 2148, CSPFT_FLOAT },
  { "fireRumble", 2152, CSPFT_STRING },
  { "meleeImpactRumble", 2156, CSPFT_STRING },
  { "adsDofStart", 2160, CSPFT_FLOAT },
  { "adsDofEnd", 2164, CSPFT_FLOAT }
}; // idb

// const char *szWeapTypeNames[4] = { "bullet", "grenade", "projectile", "binoculars" }; // idb
const char *szWeapClassNames[10] =
{
  "rifle",
  "mg",
  "smg",
  "spread",
  "pistol",
  "grenade",
  "rocketlauncher",
  "turret",
  "non-player",
  "item"
}; // idb

char *g_playerAnimTypeNames[64];

WeaponDef bg_defaultWeaponDefs;

char *__cdecl BG_GetPlayerAnimTypeName(int32_t index)
{
    return g_playerAnimTypeNames[index];
}

void __cdecl TRACK_bg_weapons_load_obj()
{
    track_static_alloc_internal(szWeapOverlayReticleNames, 8, "szWeapOverlayReticleNames", 9);
    track_static_alloc_internal(szWeapStanceNames, 12, "szWeapStanceNames", 9);
    track_static_alloc_internal(weaponDefFields, 6024, "weaponDefFields", 9);
    track_static_alloc_internal(&bg_defaultWeaponDefs, 2168, "bg_defaultWeaponDefs", 9);
    track_static_alloc_internal(penetrateTypeNames, 16, "penetrateTypeNames", 9);
    track_static_alloc_internal(szWeapTypeNames, 16, "szWeapTypeNames", 9);
    track_static_alloc_internal(szWeapClassNames, 40, "szWeapClassNames", 9);
    track_static_alloc_internal(g_playerAnimTypeNames, 256, "g_playerAnimTypeNames", 9);
    track_static_alloc_internal(szWeapInventoryTypeNames, 16, "szWeapInventoryTypeNames", 9);
}

const char *__cdecl BG_GetWeaponTypeName(weapType_t type)
{
    bcassert(type < WEAPTYPE_NUM, ARRAY_COUNT(szWeapTypeNames));

    return szWeapTypeNames[type];
}

const char *__cdecl BG_GetWeaponClassName(weapClass_t type)
{
    bcassert(type < WEAPCLASS_NUM, ARRAY_COUNT(szWeapClassNames));

    return szWeapClassNames[type];
}

const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type)
{
    bcassert(type < WEAPINVENTORYCOUNT, ARRAY_COUNT(szWeapInventoryTypeNames));

    return szWeapInventoryTypeNames[type];
}

#ifdef KISAK_MP
void __cdecl BG_LoadWeaponStrings()
{
    uint32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_playerAnimTypeNamesCount; ++i)
        BG_InitWeaponString(i, g_playerAnimTypeNames[i]);
}
#endif

void __cdecl BG_LoadPlayerAnimTypes()
{
#ifdef KISAK_MP
    char v0; // [esp+3h] [ebp-29h]
    char *v1; // [esp+8h] [ebp-24h]
    const char *v2; // [esp+Ch] [ebp-20h]
    char *buf; // [esp+20h] [ebp-Ch]
    const char *text_p; // [esp+24h] [ebp-8h] BYREF
    const char *token; // [esp+28h] [ebp-4h]

    g_playerAnimTypeNamesCount = 0;
    buf = Com_LoadRawTextFile("mp/playeranimtypes.txt");
    if (!buf)
        Com_Error(ERR_DROP, "Couldn',27h,'t load file %s", "mp/playeranimtypes.txt");
    text_p = buf;
    Com_BeginParseSession("BG_AnimParseAnimScript");
    while (1)
    {
        token = (const char *)Com_Parse(&text_p);
        if (!token || !*token)
            break;
        if (g_playerAnimTypeNamesCount >= 0x40)
            Com_Error(ERR_DROP, "Player anim type array size exceeded");
        g_playerAnimTypeNames[g_playerAnimTypeNamesCount] = (char *)Hunk_Alloc(
            strlen(token) + 1,
            "BG_LoadPlayerAnimTypes",
            9);
        v2 = token;
        v1 = g_playerAnimTypeNames[g_playerAnimTypeNamesCount];
        do
        {
            v0 = *v2;
            *v1++ = *v2++;
        } while (v0);
        ++g_playerAnimTypeNamesCount;
    }
    Com_EndParseSession();
    Com_UnloadRawTextFile(buf);
#elif KISAK_SP
    g_playerAnimTypeNamesCount = 1;
    g_playerAnimTypeNames[0] = (char*)"none";
#endif
}

void __cdecl InitWeaponDef(WeaponDef *weapDef)
{
    const cspField_t *pField; // [esp+4h] [ebp-8h]
    int iField; // [esp+8h] [ebp-4h]

    weapDef->szInternalName = "";
    iField = 0;
    pField = weaponDefFields;
    while (iField < 502)
    {
        if (pField->iFieldType == CSPFT_STRING)
            *(const char **)((char *)&weapDef->szInternalName + pField->iOffset) = "";
        ++iField;
        ++pField;
    }
}

char __cdecl G_ParseAIWeaponAccurayGraphFile(
    const char *buffer,
    const char *fileName,
    float (*knots)[2],
    int *knotCount)
{
    int v4; // eax
    long double v5; // st7
    long double v6; // st7
    int knotCountIndex; // [esp+0h] [ebp-8h]
    parseInfo_t *tokenb; // [esp+4h] [ebp-4h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]
    parseInfo_t *tokena; // [esp+4h] [ebp-4h]

    iassert(buffer);
    iassert(fileName);
    iassert(knots);
    iassert(knotCount);

    Com_BeginParseSession(fileName);
    tokenb = Com_Parse(&buffer);
    v4 = atoi(tokenb->token);
    *knotCount = v4;
    knotCountIndex = 0;
    while (1)
    {
        token = Com_Parse(&buffer);
        if (!token->token[0])
            break;
        if (token->token[0] == 125)
            break;
        v5 = atof(token->token);
        (*knots)[2 * knotCountIndex] = v5;
        tokena = Com_Parse(&buffer);
        if (!tokena->token[0] || tokena->token[0] == 125)
            break;
        v6 = atof(tokena->token);
        (*knots)[2 * knotCountIndex++ + 1] = v6;
        if (knotCountIndex >= 16)
        {
            Com_PrintWarning(CON_CHANNEL_SERVER, "WARNING: \"%s\" has too many graph knots\n", fileName);
            Com_EndParseSession();
            return 0;
        }
    }
    Com_EndParseSession();
    if (knotCountIndex == *knotCount)
    {
        if ((*knots)[2 * knotCountIndex - 2] == 1.0)
        {
            return 1;
        }
        else
        {
            Com_PrintError(CON_CHANNEL_SERVER, "ERROR: \"%s\" Range must be 0.0 to 1.0\n", fileName);
            return 0;
        }
    }
    else
    {
        Com_PrintError(CON_CHANNEL_SERVER, "ERROR: \"%s\" Error in parsing an ai weapon accuracy file\n", fileName);
        return 0;
    }
}

char __cdecl G_ParseWeaponAccurayGraphInternal(
    WeaponDef *weaponDef,
    const char *dirName,
    const char *graphName,
    float (*knots)[2],
    int *knotCount)
{
    signed int v6; // [esp+10h] [ebp-205Ch]
    char string[64]; // [esp+14h] [ebp-2058h] BYREF
    char buffer[8196]; // [esp+54h] [ebp-2018h] BYREF
    const char *last; // [esp+205Ch] [ebp-10h]
    int knotCounta; // [esp+2060h] [ebp-Ch] BYREF
    int f; // [esp+2064h] [ebp-8h] BYREF
    int len; // [esp+2068h] [ebp-4h]

    last = "WEAPONACCUFILE";
    len = strlen("WEAPONACCUFILE");
    iassert(weaponDef);
    iassert(graphName);
    iassert(knots);
    iassert(knotCount);
    iassert(dirName);

    if (weaponDef->weapType && weaponDef->weapType != WEAPTYPE_PROJECTILE)
        return 1;

    if (!*graphName)
        return 1;

    snprintf(string, ARRAYSIZE(string), "accuracy/%s/%s", dirName, graphName);
    v6 = FS_FOpenFileByMode(string, &f, FS_READ);
    if (v6 >= 0)
    {
        FS_Read((uint8_t *)buffer, len, f);
        buffer[len] = 0;
        if (!strncmp(buffer, last, len))
        {
            if (v6 - len < 0x2000)
            {
                memset((uint8_t *)buffer, 0, 0x2000u);
                FS_Read((uint8_t *)buffer, v6 - len, f);
                buffer[v6 - len] = 0;
                FS_FCloseFile(f);
                knotCounta = 0;
                if (G_ParseAIWeaponAccurayGraphFile(buffer, string, knots, &knotCounta))
                {
                    *knotCount = knotCounta;
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(CON_CHANNEL_SERVER, "WARNING: \"%s\" Is too long of an ai weapon accuracy file to parse\n", string);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(CON_CHANNEL_SERVER, "WARNING: \"%s\" does not appear to be an ai weapon accuracy file\n", string);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(CON_CHANNEL_SERVER, "WARNING: Could not load ai weapon accuracy file '%s'\n", string);
        return 0;
    }
}

char __cdecl G_ParseWeaponAccurayGraphs(WeaponDef *weaponDef)
{
    uint32_t size; // [esp+4h] [ebp-8Ch]
    int weaponType; // [esp+8h] [ebp-88h]
    int accuracyGraphKnotCount; // [esp+Ch] [ebp-84h] BYREF
    float accuracyGraphKnots[16][2]; // [esp+10h] [ebp-80h] BYREF

    for (weaponType = 0; weaponType < 2; ++weaponType)
    {
        memset((uint8_t *)accuracyGraphKnots, 0, sizeof(accuracyGraphKnots));
        accuracyGraphKnotCount = 0;
        if (!G_ParseWeaponAccurayGraphInternal(
            weaponDef,
            accuracyDirName[weaponType],
            weaponDef->accuracyGraphName[weaponType],
            accuracyGraphKnots,
            &accuracyGraphKnotCount))
            return 0;
        if (accuracyGraphKnotCount > 0)
        {
            size = 8 * accuracyGraphKnotCount;
            weaponDef->accuracyGraphKnots[weaponType] = (float (*)[2])Hunk_AllocLowAlign(
                8 * accuracyGraphKnotCount,
                4,
                "G_ParseWeaponAccurayGraphs",
                9);
            weaponDef->originalAccuracyGraphKnots[weaponType] = weaponDef->accuracyGraphKnots[weaponType];
            memcpy((uint8_t *)weaponDef->accuracyGraphKnots[weaponType], (uint8_t *)accuracyGraphKnots, size);
            weaponDef->accuracyGraphKnotCount[weaponType] = accuracyGraphKnotCount;
            weaponDef->originalAccuracyGraphKnotCount[weaponType] = weaponDef->accuracyGraphKnotCount[weaponType];
        }
    }
    return 1;
}

WeaponDef *__cdecl BG_LoadDefaultWeaponDef_LoadObj()
{
    InitWeaponDef(&bg_defaultWeaponDefs);
    bg_defaultWeaponDefs.szInternalName = "none";
    bg_defaultWeaponDefs.accuracyGraphName[0] = "noweapon.accu";
    bg_defaultWeaponDefs.accuracyGraphName[1] = "noweapon.accu";
    bg_defaultWeaponDefs.sprintDurationScale = 1.75;
    G_ParseWeaponAccurayGraphs(&bg_defaultWeaponDefs);
    return &bg_defaultWeaponDefs;
}

WeaponDef *__cdecl BG_LoadDefaultWeaponDef()
{
    if (IsFastFileLoad())
        return BG_LoadDefaultWeaponDef_FastFile();
    else
        return BG_LoadDefaultWeaponDef_LoadObj();
}

WeaponDef *__cdecl BG_LoadDefaultWeaponDef_FastFile()
{
    return DB_FindXAssetHeader(ASSET_TYPE_WEAPON, "none").weapon;
}

int __cdecl Weapon_GetStringArrayIndex(const char *value, char **stringArray, int arraySize)
{
    int arrayIndex; // [esp+0h] [ebp-4h]

    iassert(value);
    iassert(stringArray);

    for (arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
    {
        if (!I_stricmp(value, stringArray[arrayIndex]))
            return arrayIndex;
    }
    return -1;
}

snd_alias_list_t **__cdecl BG_RegisterSurfaceTypeSounds(const char *surfaceSoundBase)
{
    char *v2; // eax
    snd_alias_list_t *SoundAlias; // eax
    char v4; // [esp+3h] [ebp-131h]
    char *v5; // [esp+8h] [ebp-12Ch]
    const char *v6; // [esp+Ch] [ebp-128h]
    snd_alias_list_t **result; // [esp+20h] [ebp-114h]
    char aliasName[260]; // [esp+24h] [ebp-110h] BYREF
    snd_alias_list_t *defaultAliasList; // [esp+12Ch] [ebp-8h]
    int i; // [esp+130h] [ebp-4h]

    iassert(surfaceSoundBase);

    if (!*surfaceSoundBase)
        return 0;

    for (i = 0; i < surfaceTypeSoundListCount; ++i)
    {
        if (!I_strcmp(surfaceTypeSoundLists[i].surfaceSoundBase, surfaceSoundBase))
            return surfaceTypeSoundLists[i].soundAliasList;
    }
    if (surfaceTypeSoundListCount == 16)
        Com_Error(ERR_DROP, "Exceeded MAX_SURFACE_TYPE_SOUND_LISTS (%d)", 16);

    result = (snd_alias_list_t **)Hunk_AllocLow(0x74u, "BG_RegisterSurfaceTypeSounds", 15);
    Com_sprintf(aliasName, 0x100u, "%s_default", surfaceSoundBase);
    defaultAliasList = Com_FindSoundAlias(aliasName);
    for (i = 0; i < 29; ++i)
    {
        v2 = (char*)Com_SurfaceTypeToName(i);
        Com_sprintf(aliasName, 0x100u, "%s_%s", surfaceSoundBase, v2);
        SoundAlias = Com_FindSoundAlias(aliasName);
        result[i] = SoundAlias;
        if (!result[i])
            result[i] = defaultAliasList;
    }
    surfaceTypeSoundLists[surfaceTypeSoundListCount].surfaceSoundBase = (char *)Hunk_AllocLow(
        strlen(surfaceSoundBase) + 1,
        "BG_RegisterSurfaceTypeSounds",
        15);
    v6 = surfaceSoundBase;
    v5 = surfaceTypeSoundLists[surfaceTypeSoundListCount].surfaceSoundBase;
    do
    {
        v4 = *v6;
        *v5++ = *v6++;
    } while (v4);
    surfaceTypeSoundLists[surfaceTypeSoundListCount++].soundAliasList = result;
    return result;
}

int __cdecl BG_ParseWeaponDefSpecificFieldType(uint8_t *pStruct, const char *pValue, int iFieldType)
{
    uint16_t LowercaseString_DONE; // ax
    uint16_t v5; // ax
    int result; // eax
    char v7; // [esp+3h] [ebp-91h]
    char *v8; // [esp+8h] [ebp-8Ch]
    const char *v9; // [esp+Ch] [ebp-88h]
    int v10; // [esp+10h] [ebp-84h]
    const char *pos; // [esp+38h] [ebp-5Ch] BYREF
    int numHideTags; // [esp+3Ch] [ebp-58h]
    int numNoteTrackMappings; // [esp+40h] [ebp-54h]
    char keyName[64]; // [esp+44h] [ebp-50h] BYREF
    int arrayIndex; // [esp+88h] [ebp-Ch]
    const char *token; // [esp+8Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+90h] [ebp-4h]

    iassert(pStruct);
    iassert(pValue);

    weapDef = (WeaponDef *)pStruct;
    switch (iFieldType)
    {
    case WFT_WEAPONTYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)szWeapTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon type %s in %s", pValue, weapDef->szInternalName);
        weapDef->weapType = (weapType_t)arrayIndex;
        goto LABEL_86;
    case WFT_WEAPONCLASS:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)szWeapClassNames, 10);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon class %s in %s", pValue, weapDef->szInternalName);
        weapDef->weapClass = (weapClass_t)arrayIndex;
        goto LABEL_86;
    case WFT_OVERLAYRETICLE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapOverlayReticleNames, 2);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon reticle %s in %s", pValue, weapDef->szInternalName);
        weapDef->overlayReticle = (weapOverlayReticle_t)arrayIndex;
        goto LABEL_86;
    case WFT_PENETRATE_TYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)penetrateTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon penetrate type %s in %s", pValue, weapDef->szInternalName);
        weapDef->penetrateType = (PenetrateType)arrayIndex;
        goto LABEL_86;
    case WFT_IMPACT_TYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)impactTypeNames, 9);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon impact type %s in %s", pValue, weapDef->szInternalName);
        weapDef->impactType = (ImpactType)arrayIndex;
        goto LABEL_86;
    case WFT_STANCE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapStanceNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon stance %s in %s", pValue, weapDef->szInternalName);
        weapDef->stance = (weapStance_t)arrayIndex;
        goto LABEL_86;
    case WFT_PROJ_EXPLOSION:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szProjectileExplosionNames, 7);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon projExplosion %s in %s", pValue, weapDef->szInternalName);
        weapDef->projExplosion = (weapProjExposion_t)arrayIndex;
        goto LABEL_86;
    case WFT_OFFHAND_CLASS:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)offhandClassNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon offhand class %s in %s", pValue, weapDef->szInternalName);
        weapDef->offhandClass = (OffhandClass)arrayIndex;
        goto LABEL_86;
    case WFT_ANIMTYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, g_playerAnimTypeNames, g_playerAnimTypeNamesCount);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon player anim type %s in %s", pValue, weapDef->szInternalName);
        weapDef->playerAnimType = arrayIndex;
        goto LABEL_86;
    case WFT_ACTIVE_RETICLE_TYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)activeReticleNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon active reticle type %s in %s", pValue, weapDef->szInternalName);
        weapDef->activeReticleType = (activeReticleType_t)arrayIndex;
        goto LABEL_86;
    case WFT_GUIDED_MISSILE_TYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)guidedMissileNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon guided missile type %s in %s", pValue, weapDef->szInternalName);
        weapDef->guidedMissileType = (guidedMissileType_t)arrayIndex;
        goto LABEL_86;
    case WFT_BOUNCE_SOUND:
        weapDef->bounceSound = BG_RegisterSurfaceTypeSounds(pValue);
        goto LABEL_86;
    case WFT_STICKINESS:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)stickinessNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon stickiness %s in %s", pValue, weapDef->szInternalName);
        weapDef->stickiness = (WeapStickinessType)arrayIndex;
        goto LABEL_86;
    case WFT_OVERLAYINTERFACE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)overlayInterfaceNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon overlay interface %s in %s", pValue, weapDef->szInternalName);
        weapDef->overlayInterface = (WeapOverlayInteface_t)arrayIndex;
        goto LABEL_86;
    case WFT_INVENTORYTYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapInventoryTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon inventory type %s in %s", pValue, weapDef->szInternalName);
        weapDef->inventoryType = (weapInventoryType_t)arrayIndex;
        goto LABEL_86;
    case WFT_FIRETYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapFireTypeNames, 5);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon firetype %s in %s", pValue, weapDef->szInternalName);
        weapDef->fireType = (weapFireType_t)arrayIndex;
        goto LABEL_86;
    case WFT_AMMOCOUNTER_CLIPTYPE:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)ammoCounterClipNames, 7);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon ammo counter clip %s in %s", pValue, weapDef->szInternalName);
        weapDef->ammoCounterClip = (ammoCounterClipType_t)arrayIndex;
        goto LABEL_86;
    case WFT_ICONRATIO_HUD:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon hud icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->hudIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case WFT_ICONRATIO_AMMOCOUNTER:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon ammo counter icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->ammoCounterIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case WFT_ICONRATIO_KILL:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon kill icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->killIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case WFT_ICONRATIO_DPAD:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon dpad icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->dpadIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case WFT_HIDETAGS:
        numHideTags = 0;
        pos = pValue;
        while (1)
        {
            token = (const char *)Com_Parse(&pos);
            if (!pos)
                break;
            if (numHideTags >= 8)
                Com_Error(ERR_DROP, "maximum hide tags (%s) exceeded: %i > %i'", token, numHideTags, 8);
            weapDef->hideTags[numHideTags] = SL_GetStringOfSize((char *)token, 0, strlen(token) + 1, MT_TYPE_MODEL_PART);
            weapDef->hideTags[numHideTags] = SL_ConvertToLowercase(weapDef->hideTags[numHideTags], 0, MT_TYPE_MODEL_PART);
            ++numHideTags;
        }
        goto LABEL_86;
    case WFT_NOTETRACKSOUNDMAP:
        numNoteTrackMappings = 0;
        pos = pValue;
        keyName[0] = 0;
        while (1)
        {
            token = (const char *)Com_Parse(&pos);
            if (!pos)
                break;
            if (numNoteTrackMappings >= 16)
                Com_Error(ERR_DROP, "Max notetrack-to-sound mappings (%i) exceeded with entry '%s'", 16, token);
            if (keyName[0])
            {
                LowercaseString_DONE = SL_GetLowercaseString(keyName, 0);
                weapDef->notetrackSoundMapKeys[numNoteTrackMappings] = LowercaseString_DONE;
                v5 = SL_GetLowercaseString(token, 0);
                weapDef->notetrackSoundMapValues[numNoteTrackMappings++] = v5;
                keyName[0] = 0;
            }   
            else
            {
                v10 = strlen(token);
                if (v10 >= 63)
                    Com_Error(ERR_DROP, "Notetrack - to - sound: keyname \"%s\" is too long(length % i / % i).", token, v10, 63);
                v9 = token;
                v8 = keyName;
                do
                {
                    v7 = *v9;
                    *v8++ = *v9++;
                } while (v7);
            }
        }
        if (keyName[0])
            Com_PrintWarning(
                CON_CHANNEL_DONT_FILTER,
                "Notetrack-to-Sound: Weapon '%s' has bad entry; notetrack '%s' doesn't have a corresponding sound.\n",
                weapDef->szInternalName,
                keyName);
    LABEL_86:
        result = 1;
        break;
    default:
        Com_Error(ERR_DROP, "Bad field type %i in %s", iFieldType, weapDef->szInternalName);
        result = 0;
        break;
    }
    return result;
}

void __cdecl BG_SetupTransitionTimes(WeaponDef *weapDef)
{
    double v1; // st7
    double v2; // st7

    if (weapDef->iAdsTransInTime <= 0)
        v1 = 1.0 / (float)300.0;
    else
        v1 = 1.0 / (double)weapDef->iAdsTransInTime;
    weapDef->fOOPosAnimLength[0] = v1;
    if (weapDef->iAdsTransOutTime <= 0)
        v2 = 1.0 / (float)500.0;
    else
        v2 = 1.0 / (double)weapDef->iAdsTransOutTime;
    weapDef->fOOPosAnimLength[1] = v2;
}

void __cdecl BG_CheckWeaponDamageRanges(WeaponDef *weapDef)
{
    if (weapDef->fMaxDamageRange <= 0.0f)
        weapDef->fMaxDamageRange = 999999.0f;
    if (weapDef->fMinDamageRange <= 0.0f)
        weapDef->fMinDamageRange = 999999.12f;
}

void __cdecl BG_CheckProjectileValues(WeaponDef *weaponDef)
{
    iassert(weaponDef->weapType == WEAPTYPE_PROJECTILE);

    if ((double)weaponDef->iProjectileSpeed <= 0.0)
        Com_Error(ERR_DROP, "Projectile speed for WeapType %s must be greater than 0.0", weaponDef->szDisplayName);

    if (weaponDef->destabilizationCurvatureMax >= 1000000000.0f || weaponDef->destabilizationCurvatureMax < 0.0)
        Com_Error(
            ERR_DROP,
            "Destabilization angle for for WeapType %s must be between 0 and 45 degrees",
            weaponDef->szDisplayName);

    if (weaponDef->destabilizationRateTime < 0.0)
        Com_Error(ERR_DROP, "Destabilization rate time for for WeapType %s must be non-negative", weaponDef->szDisplayName);
}

WeaponDef *__cdecl BG_LoadWeaponDefInternal(const char *one, const char *two)
{
    snd_alias_list_t *SoundAlias; // eax
    snd_alias_list_t *v4; // eax
    snd_alias_list_t *v5; // eax
    snd_alias_list_t *v6; // eax
    snd_alias_list_t *v7; // eax
    char buffer[10244]; // [esp+1Ch] [ebp-2858h] BYREF
    int f; // [esp+2820h] [ebp-54h] BYREF
    int len; // [esp+2824h] [ebp-50h]
    signed int v11; // [esp+2828h] [ebp-4Ch]
    char dest[64]; // [esp+282Ch] [ebp-48h] BYREF
    WeaponDef *weapDef; // [esp+2870h] [ebp-4h]

    len = strlen("WEAPONFILE");
    weapDef = (WeaponDef *)Hunk_AllocLow(0x878u, "BG_LoadWeaponDefInternal", 9);
    InitWeaponDef(weapDef);
    Com_sprintf(dest, 0x40u, "weapons/%s/%s", one, two);
    v11 = FS_FOpenFileByMode(dest, &f, FS_READ);
    if (v11 >= 0)
    {
        FS_Read((uint8_t *)buffer, len, f);
        buffer[len] = 0;
        if (!strncmp(buffer, "WEAPONFILE", len))
        {
            if ((uint32_t)(v11 - len) < 0x2800)
            {
                memset((uint8_t *)buffer, 0, 0x2800u);
                FS_Read((uint8_t *)buffer, v11 - len, f);
                buffer[v11 - len] = 0;
                FS_FCloseFile(f);
                if (Info_Validate(buffer))
                {
                    SetConfigString((char **)weapDef, two);
                    if (ParseConfigStringToStructCustomSize(
                        (uint8_t *)weapDef,
                        weaponDefFields,
                        502,
                        buffer,
                        WFT_NUM_FIELD_TYPES,
                        BG_ParseWeaponDefSpecificFieldType,
                        SetConfigString2))
                    {
                        if (I_stricmp(two, "defaultweapon_mp"))
                        {
                            if (!weapDef->viewLastShotEjectEffect)
                                weapDef->viewLastShotEjectEffect = weapDef->viewShellEjectEffect;
                            if (!weapDef->worldLastShotEjectEffect)
                                weapDef->worldLastShotEjectEffect = weapDef->worldShellEjectEffect;
                            if (!weapDef->raiseSound)
                            {
                                SoundAlias = Com_FindSoundAlias("weap_raise");
                                weapDef->raiseSound = SoundAlias;
                            }
                            if (!weapDef->putawaySound)
                            {
                                v4 = Com_FindSoundAlias("weap_putaway");
                                weapDef->putawaySound = v4;
                            }
                            if (!weapDef->pickupSound)
                            {
                                v5 = Com_FindSoundAlias("weap_pickup");
                                weapDef->pickupSound = v5;
                            }
                            if (!weapDef->ammoPickupSound)
                            {
                                v6 = Com_FindSoundAlias("weap_ammo_pickup");
                                weapDef->ammoPickupSound = v6;
                            }
                            if (!weapDef->emptyFireSound)
                            {
                                v7 = Com_FindSoundAlias("weap_dryfire_smg_npc");
                                weapDef->emptyFireSound = v7;
                            }
                        }
                        BG_SetupTransitionTimes(weapDef);
                        BG_CheckWeaponDamageRanges(weapDef);
                        if (weapDef->enemyCrosshairRange > 15000.0)
                            Com_Error(ERR_DROP, "Enemy crosshair ranges should be less than %f ", 15000.0);
                        if (weapDef->weapType == WEAPTYPE_PROJECTILE)
                            BG_CheckProjectileValues(weapDef);
                        if (G_ParseWeaponAccurayGraphs(weapDef))
                        {
                            I_strlwr((char *)weapDef->szAmmoName);
                            I_strlwr((char *)weapDef->szClipName);
                            return weapDef;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    Com_PrintWarning(CON_CHANNEL_PLAYERWEAP, "WARNING: \"%s\" is not a valid weapon file\n", dest);
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(
                    CON_CHANNEL_PLAYERWEAP,
                    "WARNING: \"%s\" Is too long of a weapon file to parse (fileLength = %d identifierLength = %d)\n",
                    dest,
                    v11,
                    len);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(CON_CHANNEL_PLAYERWEAP, "WARNING: \"%s\" does not appear to be a weapon file\n", dest);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(CON_CHANNEL_PLAYERWEAP, "WARNING: Could not load weapon file '%s'\n", dest);
        return 0;
    }
}


WeaponDef *__cdecl BG_LoadWeaponDef_LoadObj(const char *name)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!*name)
        return 0;
#ifdef KISAK_MP
    weapDef = BG_LoadWeaponDefInternal("mp", name);
#elif KISAK_SP
    weapDef = BG_LoadWeaponDefInternal("sp", name);
#endif
    if (weapDef)
        return weapDef;

#ifdef KISAK_MP
    weapDef = BG_LoadWeaponDefInternal("mp", "defaultweapon_mp");
#elif KISAK_SP
    weapDef = BG_LoadWeaponDefInternal("sp", "defaultweapon");
#endif

    if (!weapDef)
        Com_Error(ERR_DROP, "BG_LoadWeaponDef: Could not find default weapon");

    SetConfigString((char **)weapDef, name);
    return weapDef;
}
