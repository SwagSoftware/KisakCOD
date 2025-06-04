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

unsigned int g_playerAnimTypeNamesCount;

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
  { "displayName", 4, 0 },
  { "AIOverlayDescription", 8, 0 },
  { "modeName", 212, 0 },
  { "playerAnimType", 296, 20 },
  { "gunModel", 12, 9 },
  { "gunModel2", 16, 9 },
  { "gunModel3", 20, 9 },
  { "gunModel4", 24, 9 },
  { "gunModel5", 28, 9 },
  { "gunModel6", 32, 9 },
  { "gunModel7", 36, 9 },
  { "gunModel8", 40, 9 },
  { "gunModel9", 44, 9 },
  { "gunModel10", 48, 9 },
  { "gunModel11", 52, 9 },
  { "gunModel12", 56, 9 },
  { "gunModel13", 60, 9 },
  { "gunModel14", 64, 9 },
  { "gunModel15", 68, 9 },
  { "gunModel16", 72, 9 },
  { "handModel", 76, 9 },
  { "hideTags", 216, 33 },
  { "notetrackSoundMap", 232, 34 },
  { "idleAnim", 84, 0 },
  { "emptyIdleAnim", 88, 0 },
  { "fireAnim", 92, 0 },
  { "holdFireAnim", 96, 0 },
  { "lastShotAnim", 100, 0 },
  { "detonateAnim", 180, 0 },
  { "rechamberAnim", 104, 0 },
  { "meleeAnim", 108, 0 },
  { "meleeChargeAnim", 112, 0 },
  { "reloadAnim", 116, 0 },
  { "reloadEmptyAnim", 120, 0 },
  { "reloadStartAnim", 124, 0 },
  { "reloadEndAnim", 128, 0 },
  { "raiseAnim", 132, 0 },
  { "dropAnim", 140, 0 },
  { "firstRaiseAnim", 136, 0 },
  { "altRaiseAnim", 144, 0 },
  { "altDropAnim", 148, 0 },
  { "quickRaiseAnim", 152, 0 },
  { "quickDropAnim", 156, 0 },
  { "emptyRaiseAnim", 160, 0 },
  { "emptyDropAnim", 164, 0 },
  { "sprintInAnim", 168, 0 },
  { "sprintLoopAnim", 172, 0 },
  { "sprintOutAnim", 176, 0 },
  { "nightVisionWearAnim", 184, 0 },
  { "nightVisionRemoveAnim", 188, 0 },
  { "adsFireAnim", 192, 0 },
  { "adsLastShotAnim", 196, 0 },
  { "adsRechamberAnim", 200, 0 },
  { "adsUpAnim", 204, 0 },
  { "adsDownAnim", 208, 0 },
  { "script", 2036, 0 },
  { "weaponType", 300, 12 },
  { "weaponClass", 304, 13 },
  { "penetrateType", 308, 15 },
  { "impactType", 312, 16 },
  { "inventoryType", 316, 26 },
  { "fireType", 320, 27 },
  { "offhandClass", 324, 19 },
  { "viewFlashEffect", 332, 8 },
  { "worldFlashEffect", 336, 8 },
  { "pickupSound", 340, 11 },
  { "pickupSoundPlayer", 344, 11 },
  { "ammoPickupSound", 348, 11 },
  { "ammoPickupSoundPlayer", 352, 11 },
  { "projectileSound", 356, 11 },
  { "pullbackSound", 360, 11 },
  { "pullbackSoundPlayer", 364, 11 },
  { "fireSound", 368, 11 },
  { "fireSoundPlayer", 372, 11 },
  { "loopFireSound", 376, 11 },
  { "loopFireSoundPlayer", 380, 11 },
  { "stopFireSound", 384, 11 },
  { "stopFireSoundPlayer", 388, 11 },
  { "lastShotSound", 392, 11 },
  { "lastShotSoundPlayer", 396, 11 },
  { "emptyFireSound", 400, 11 },
  { "emptyFireSoundPlayer", 404, 11 },
  { "meleeSwipeSound", 408, 11 },
  { "meleeSwipeSoundPlayer", 412, 11 },
  { "meleeHitSound", 416, 11 },
  { "meleeMissSound", 420, 11 },
  { "rechamberSound", 424, 11 },
  { "rechamberSoundPlayer", 428, 11 },
  { "reloadSound", 432, 11 },
  { "reloadSoundPlayer", 436, 11 },
  { "reloadEmptySound", 440, 11 },
  { "reloadEmptySoundPlayer", 444, 11 },
  { "reloadStartSound", 448, 11 },
  { "reloadStartSoundPlayer", 452, 11 },
  { "reloadEndSound", 456, 11 },
  { "reloadEndSoundPlayer", 460, 11 },
  { "detonateSound", 464, 11 },
  { "detonateSoundPlayer", 468, 11 },
  { "nightVisionWearSound", 472, 11 },
  { "nightVisionWearSoundPlayer", 476, 11 },
  { "nightVisionRemoveSound", 480, 11 },
  { "nightVisionRemoveSoundPlayer", 484, 11 },
  { "raiseSound", 496, 11 },
  { "raiseSoundPlayer", 500, 11 },
  { "firstRaiseSound", 504, 11 },
  { "firstRaiseSoundPlayer", 508, 11 },
  { "altSwitchSound", 488, 11 },
  { "altSwitchSoundPlayer", 492, 11 },
  { "putawaySound", 512, 11 },
  { "putawaySoundPlayer", 516, 11 },
  { "bounceSound", 520, 23 },
  { "viewShellEjectEffect", 524, 8 },
  { "worldShellEjectEffect", 528, 8 },
  { "viewLastShotEjectEffect", 532, 8 },
  { "worldLastShotEjectEffect", 536, 8 },
  { "reticleCenter", 540, 10 },
  { "reticleSide", 544, 10 },
  { "reticleCenterSize", 548, 4 },
  { "reticleSideSize", 552, 4 },
  { "reticleMinOfs", 556, 4 },
  { "activeReticleType", 560, 21 },
  { "standMoveF", 564, 6 },
  { "standMoveR", 568, 6 },
  { "standMoveU", 572, 6 },
  { "standRotP", 576, 6 },
  { "standRotY", 580, 6 },
  { "standRotR", 584, 6 },
  { "duckedOfsF", 588, 6 },
  { "duckedOfsR", 592, 6 },
  { "duckedOfsU", 596, 6 },
  { "duckedMoveF", 600, 6 },
  { "duckedMoveR", 604, 6 },
  { "duckedMoveU", 608, 6 },
  { "duckedRotP", 612, 6 },
  { "duckedRotY", 616, 6 },
  { "duckedRotR", 620, 6 },
  { "proneOfsF", 624, 6 },
  { "proneOfsR", 628, 6 },
  { "proneOfsU", 632, 6 },
  { "proneMoveF", 636, 6 },
  { "proneMoveR", 640, 6 },
  { "proneMoveU", 644, 6 },
  { "proneRotP", 648, 6 },
  { "proneRotY", 652, 6 },
  { "proneRotR", 656, 6 },
  { "posMoveRate", 660, 6 },
  { "posProneMoveRate", 664, 6 },
  { "standMoveMinSpeed", 668, 6 },
  { "duckedMoveMinSpeed", 672, 6 },
  { "proneMoveMinSpeed", 676, 6 },
  { "posRotRate", 680, 6 },
  { "posProneRotRate", 684, 6 },
  { "standRotMinSpeed", 688, 6 },
  { "duckedRotMinSpeed", 692, 6 },
  { "proneRotMinSpeed", 696, 6 },
  { "worldModel", 700, 9 },
  { "worldModel2", 704, 9 },
  { "worldModel3", 708, 9 },
  { "worldModel4", 712, 9 },
  { "worldModel5", 716, 9 },
  { "worldModel6", 720, 9 },
  { "worldModel7", 724, 9 },
  { "worldModel8", 728, 9 },
  { "worldModel9", 732, 9 },
  { "worldModel10", 736, 9 },
  { "worldModel11", 740, 9 },
  { "worldModel12", 744, 9 },
  { "worldModel13", 748, 9 },
  { "worldModel14", 752, 9 },
  { "worldModel15", 756, 9 },
  { "worldModel16", 760, 9 },
  { "worldClipModel", 764, 9 },
  { "rocketModel", 768, 9 },
  { "knifeModel", 772, 9 },
  { "worldKnifeModel", 776, 9 },
  { "hudIcon", 780, 10 },
  { "hudIconRatio", 784, 29 },
  { "ammoCounterIcon", 788, 10 },
  { "ammoCounterIconRatio", 792, 30 },
  { "ammoCounterClip", 796, 28 },
  { "startAmmo", 800, 4 },
  { "ammoName", 804, 0 },
  { "clipName", 812, 0 },
  { "maxAmmo", 820, 4 },
  { "clipSize", 824, 4 },
  { "shotCount", 828, 4 },
  { "sharedAmmoCapName", 832, 0 },
  { "sharedAmmoCap", 840, 4 },
  { "damage", 844, 4 },
  { "playerDamage", 848, 4 },
  { "meleeDamage", 852, 4 },
  { "minDamage", 2048, 4 },
  { "minPlayerDamage", 2052, 4 },
  { "maxDamageRange", 2056, 6 },
  { "minDamageRange", 2060, 6 },
  { "destabilizationRateTime", 2064, 6 },
  { "destabilizationCurvatureMax", 2068, 6 },
  { "destabilizeDistance", 2072, 4 },
  { "fireDelay", 860, 7 },
  { "meleeDelay", 864, 7 },
  { "meleeChargeDelay", 868, 7 },
  { "fireTime", 876, 7 },
  { "rechamberTime", 880, 7 },
  { "rechamberBoltTime", 884, 7 },
  { "holdFireTime", 888, 7 },
  { "detonateTime", 892, 7 },
  { "detonateDelay", 872, 7 },
  { "meleeTime", 896, 7 },
  { "meleeChargeTime", 900, 7 },
  { "reloadTime", 904, 7 },
  { "reloadShowRocketTime", 908, 7 },
  { "reloadEmptyTime", 912, 7 },
  { "reloadAddTime", 916, 7 },
  { "reloadStartTime", 920, 7 },
  { "reloadStartAddTime", 924, 7 },
  { "reloadEndTime", 928, 7 },
  { "dropTime", 932, 7 },
  { "raiseTime", 936, 7 },
  { "altDropTime", 940, 7 },
  { "altRaiseTime", 944, 7 },
  { "quickDropTime", 948, 7 },
  { "quickRaiseTime", 952, 7 },
  { "firstRaiseTime", 956, 7 },
  { "emptyRaiseTime", 960, 7 },
  { "emptyDropTime", 964, 7 },
  { "sprintInTime", 968, 7 },
  { "sprintLoopTime", 972, 7 },
  { "sprintOutTime", 976, 7 },
  { "nightVisionWearTime", 980, 7 },
  { "nightVisionWearTimeFadeOutEnd", 984, 7 },
  { "nightVisionWearTimePowerUp", 988, 7 },
  { "nightVisionRemoveTime", 992, 7 },
  { "nightVisionRemoveTimePowerDown", 996, 7 },
  { "nightVisionRemoveTimeFadeInStart", 1000, 7 },
  { "fuseTime", 1004, 7 },
  { "aifuseTime", 1008, 7 },
  { "requireLockonToFire", 1012, 5 },
  { "noAdsWhenMagEmpty", 1016, 5 },
  { "avoidDropCleanup", 1020, 5 },
  { "autoAimRange", 1024, 6 },
  { "aimAssistRange", 1028, 6 },
  { "aimAssistRangeAds", 1032, 6 },
  { "aimPadding", 1036, 6 },
  { "enemyCrosshairRange", 1040, 6 },
  { "crosshairColorChange", 1044, 5 },
  { "moveSpeedScale", 1048, 6 },
  { "adsMoveSpeedScale", 1052, 6 },
  { "sprintDurationScale", 1056, 6 },
  { "idleCrouchFactor", 1180, 6 },
  { "idleProneFactor", 1184, 6 },
  { "gunMaxPitch", 1188, 6 },
  { "gunMaxYaw", 1192, 6 },
  { "swayMaxAngle", 1196, 6 },
  { "swayLerpSpeed", 1200, 6 },
  { "swayPitchScale", 1204, 6 },
  { "swayYawScale", 1208, 6 },
  { "swayHorizScale", 1212, 6 },
  { "swayVertScale", 1216, 6 },
  { "swayShellShockScale", 1220, 6 },
  { "adsSwayMaxAngle", 1224, 6 },
  { "adsSwayLerpSpeed", 1228, 6 },
  { "adsSwayPitchScale", 1232, 6 },
  { "adsSwayYawScale", 1236, 6 },
  { "adsSwayHorizScale", 1240, 6 },
  { "adsSwayVertScale", 1244, 6 },
  { "rifleBullet", 1248, 5 },
  { "armorPiercing", 1252, 5 },
  { "boltAction", 1256, 5 },
  { "aimDownSight", 1260, 5 },
  { "rechamberWhileAds", 1264, 5 },
  { "adsViewErrorMin", 1268, 6 },
  { "adsViewErrorMax", 1272, 6 },
  { "clipOnly", 1280, 5 },
  { "cookOffHold", 1276, 5 },
  { "adsFire", 1284, 5 },
  { "cancelAutoHolsterWhenEmpty", 1288, 5 },
  { "suppressAmmoReserveDisplay", 1292, 5 },
  { "enhanced", 1296, 5 },
  { "laserSightDuringNightvision", 1300, 5 },
  { "killIcon", 1304, 10 },
  { "killIconRatio", 1308, 31 },
  { "flipKillIcon", 1312, 5 },
  { "dpadIcon", 1316, 10 },
  { "dpadIconRatio", 1320, 32 },
  { "noPartialReload", 1324, 5 },
  { "segmentedReload", 1328, 5 },
  { "reloadAmmoAdd", 1332, 4 },
  { "reloadStartAdd", 1336, 4 },
  { "altWeapon", 1340, 0 },
  { "dropAmmoMin", 1348, 4 },
  { "dropAmmoMax", 1352, 4 },
  { "blocksProne", 1356, 5 },
  { "silenced", 1360, 5 },
  { "explosionRadius", 1364, 4 },
  { "explosionRadiusMin", 1368, 4 },
  { "explosionInnerDamage", 1372, 4 },
  { "explosionOuterDamage", 1376, 4 },
  { "damageConeAngle", 1380, 6 },
  { "projectileSpeed", 1384, 4 },
  { "projectileSpeedUp", 1388, 4 },
  { "projectileSpeedForward", 1392, 4 },
  { "projectileActivateDist", 1396, 4 },
  { "projectileLifetime", 1400, 6 },
  { "timeToAccelerate", 1404, 6 },
  { "projectileCurvature", 1408, 6 },
  { "projectileModel", 1412, 9 },
  { "projExplosionType", 1416, 18 },
  { "projExplosionEffect", 1420, 8 },
  { "projExplosionEffectForceNormalUp", 1424, 5 },
  { "projExplosionSound", 1432, 11 },
  { "projDudEffect", 1428, 8 },
  { "projDudSound", 1436, 11 },
  { "projImpactExplode", 1440, 5 },
  { "stickiness", 1444, 24 },
  { "hasDetonator", 1448, 5 },
  { "timedDetonation", 1452, 5 },
  { "rotate", 1456, 5 },
  { "holdButtonToThrow", 1460, 5 },
  { "freezeMovementWhenFiring", 1464, 5 },
  { "lowAmmoWarningThreshold", 1468, 6 },
  { "parallelDefaultBounce", 1472, 6 },
  { "parallelBarkBounce", 1476, 6 },
  { "parallelBrickBounce", 1480, 6 },
  { "parallelCarpetBounce", 1484, 6 },
  { "parallelClothBounce", 1488, 6 },
  { "parallelConcreteBounce", 1492, 6 },
  { "parallelDirtBounce", 1496, 6 },
  { "parallelFleshBounce", 1500, 6 },
  { "parallelFoliageBounce", 1504, 6 },
  { "parallelGlassBounce", 1508, 6 },
  { "parallelGrassBounce", 1512, 6 },
  { "parallelGravelBounce", 1516, 6 },
  { "parallelIceBounce", 1520, 6 },
  { "parallelMetalBounce", 1524, 6 },
  { "parallelMudBounce", 1528, 6 },
  { "parallelPaperBounce", 1532, 6 },
  { "parallelPlasterBounce", 1536, 6 },
  { "parallelRockBounce", 1540, 6 },
  { "parallelSandBounce", 1544, 6 },
  { "parallelSnowBounce", 1548, 6 },
  { "parallelWaterBounce", 1552, 6 },
  { "parallelWoodBounce", 1556, 6 },
  { "parallelAsphaltBounce", 1560, 6 },
  { "parallelCeramicBounce", 1564, 6 },
  { "parallelPlasticBounce", 1568, 6 },
  { "parallelRubberBounce", 1572, 6 },
  { "parallelCushionBounce", 1576, 6 },
  { "parallelFruitBounce", 1580, 6 },
  { "parallelPaintedMetalBounce", 1584, 6 },
  { "perpendicularDefaultBounce", 1588, 6 },
  { "perpendicularBarkBounce", 1592, 6 },
  { "perpendicularBrickBounce", 1596, 6 },
  { "perpendicularCarpetBounce", 1600, 6 },
  { "perpendicularClothBounce", 1604, 6 },
  { "perpendicularConcreteBounce", 1608, 6 },
  { "perpendicularDirtBounce", 1612, 6 },
  { "perpendicularFleshBounce", 1616, 6 },
  { "perpendicularFoliageBounce", 1620, 6 },
  { "perpendicularGlassBounce", 1624, 6 },
  { "perpendicularGrassBounce", 1628, 6 },
  { "perpendicularGravelBounce", 1632, 6 },
  { "perpendicularIceBounce", 1636, 6 },
  { "perpendicularMetalBounce", 1640, 6 },
  { "perpendicularMudBounce", 1644, 6 },
  { "perpendicularPaperBounce", 1648, 6 },
  { "perpendicularPlasterBounce", 1652, 6 },
  { "perpendicularRockBounce", 1656, 6 },
  { "perpendicularSandBounce", 1660, 6 },
  { "perpendicularSnowBounce", 1664, 6 },
  { "perpendicularWaterBounce", 1668, 6 },
  { "perpendicularWoodBounce", 1672, 6 },
  { "perpendicularAsphaltBounce", 1676, 6 },
  { "perpendicularCeramicBounce", 1564, 6 },
  { "perpendicularPlasticBounce", 1568, 6 },
  { "perpendicularRubberBounce", 1572, 6 },
  { "perpendicularCushionBounce", 1692, 6 },
  { "perpendicularFruitBounce", 1696, 6 },
  { "perpendicularPaintedMetalBounce", 1700, 6 },
  { "projTrailEffect", 1704, 8 },
  { "projectileRed", 1708, 6 },
  { "projectileGreen", 1712, 6 },
  { "projectileBlue", 1716, 6 },
  { "guidedMissileType", 1720, 22 },
  { "maxSteeringAccel", 1724, 6 },
  { "projIgnitionDelay", 1728, 4 },
  { "projIgnitionEffect", 1732, 8 },
  { "projIgnitionSound", 1736, 11 },
  { "adsTransInTime", 1156, 7 },
  { "adsTransOutTime", 1160, 7 },
  { "adsIdleAmount", 1164, 6 },
  { "adsIdleSpeed", 1172, 6 },
  { "adsZoomFov", 1060, 6 },
  { "adsZoomInFrac", 1064, 6 },
  { "adsZoomOutFrac", 1068, 6 },
  { "adsOverlayShader", 1072, 10 },
  { "adsOverlayShaderLowRes", 1076, 10 },
  { "adsOverlayReticle", 1080, 14 },
  { "adsOverlayInterface", 1084, 25 },
  { "adsOverlayWidth", 1088, 6 },
  { "adsOverlayHeight", 1092, 6 },
  { "adsBobFactor", 1096, 6 },
  { "adsViewBobMult", 1100, 6 },
  { "adsAimPitch", 1740, 6 },
  { "adsCrosshairInFrac", 1744, 6 },
  { "adsCrosshairOutFrac", 1748, 6 },
  { "adsReloadTransTime", 1940, 7 },
  { "adsGunKickReducedKickBullets", 1752, 4 },
  { "adsGunKickReducedKickPercent", 1756, 6 },
  { "adsGunKickPitchMin", 1760, 6 },
  { "adsGunKickPitchMax", 1764, 6 },
  { "adsGunKickYawMin", 1768, 6 },
  { "adsGunKickYawMax", 1772, 6 },
  { "adsGunKickAccel", 1776, 6 },
  { "adsGunKickSpeedMax", 1780, 6 },
  { "adsGunKickSpeedDecay", 1784, 6 },
  { "adsGunKickStaticDecay", 1788, 6 },
  { "adsViewKickPitchMin", 1792, 6 },
  { "adsViewKickPitchMax", 1796, 6 },
  { "adsViewKickYawMin", 1800, 6 },
  { "adsViewKickYawMax", 1804, 6 },
  { "adsViewKickCenterSpeed", 1808, 6 },
  { "adsSpread", 1820, 6 },
  { "guidedMissileType", 1720, 22 },
  { "hipSpreadStandMin", 1104, 6 },
  { "hipSpreadDuckedMin", 1108, 6 },
  { "hipSpreadProneMin", 1112, 6 },
  { "hipSpreadMax", 1116, 6 },
  { "hipSpreadDuckedMax", 1120, 6 },
  { "hipSpreadProneMax", 1124, 6 },
  { "hipSpreadDecayRate", 1128, 6 },
  { "hipSpreadFireAdd", 1132, 6 },
  { "hipSpreadTurnAdd", 1136, 6 },
  { "hipSpreadMoveAdd", 1140, 6 },
  { "hipSpreadDuckedDecay", 1144, 6 },
  { "hipSpreadProneDecay", 1148, 6 },
  { "hipReticleSidePos", 1152, 6 },
  { "hipIdleAmount", 1168, 6 },
  { "hipIdleSpeed", 1176, 6 },
  { "hipGunKickReducedKickBullets", 1824, 4 },
  { "hipGunKickReducedKickPercent", 1828, 6 },
  { "hipGunKickPitchMin", 1832, 6 },
  { "hipGunKickPitchMax", 1836, 6 },
  { "hipGunKickYawMin", 1840, 6 },
  { "hipGunKickYawMax", 1844, 6 },
  { "hipGunKickAccel", 1848, 6 },
  { "hipGunKickSpeedMax", 1852, 6 },
  { "hipGunKickSpeedDecay", 1856, 6 },
  { "hipGunKickStaticDecay", 1860, 6 },
  { "hipViewKickPitchMin", 1864, 6 },
  { "hipViewKickPitchMax", 1868, 6 },
  { "hipViewKickYawMin", 1872, 6 },
  { "hipViewKickYawMax", 1876, 6 },
  { "hipViewKickCenterSpeed", 1880, 6 },
  { "leftArc", 1944, 6 },
  { "rightArc", 1948, 6 },
  { "topArc", 1952, 6 },
  { "bottomArc", 1956, 6 },
  { "accuracy", 1960, 6 },
  { "aiSpread", 1964, 6 },
  { "playerSpread", 1968, 6 },
  { "maxVertTurnSpeed", 1980, 6 },
  { "maxHorTurnSpeed", 1984, 6 },
  { "minVertTurnSpeed", 1972, 6 },
  { "minHorTurnSpeed", 1976, 6 },
  { "pitchConvergenceTime", 1988, 6 },
  { "yawConvergenceTime", 1992, 6 },
  { "suppressionTime", 1996, 6 },
  { "maxRange", 2000, 6 },
  { "animHorRotateInc", 2004, 6 },
  { "playerPositionDist", 2008, 6 },
  { "stance", 328, 17 },
  { "useHintString", 2012, 0 },
  { "dropHintString", 2016, 0 },
  { "horizViewJitter", 2028, 6 },
  { "vertViewJitter", 2032, 6 },
  { "fightDist", 1892, 6 },
  { "maxDist", 1896, 6 },
  { "aiVsAiAccuracyGraph", 1900, 0 },
  { "aiVsPlayerAccuracyGraph", 1904, 0 },
  { "locNone", 2076, 6 },
  { "locHelmet", 2080, 6 },
  { "locHead", 2084, 6 },
  { "locNeck", 2088, 6 },
  { "locTorsoUpper", 2092, 6 },
  { "locTorsoLower", 2096, 6 },
  { "locRightArmUpper", 2100, 6 },
  { "locRightArmLower", 2108, 6 },
  { "locRightHand", 2116, 6 },
  { "locLeftArmUpper", 2104, 6 },
  { "locLeftArmLower", 2112, 6 },
  { "locLeftHand", 2120, 6 },
  { "locRightLegUpper", 2124, 6 },
  { "locRightLegLower", 2132, 6 },
  { "locRightFoot", 2140, 6 },
  { "locLeftLegUpper", 2128, 6 },
  { "locLeftLegLower", 2136, 6 },
  { "locLeftFoot", 2144, 6 },
  { "locGun", 2148, 6 },
  { "fireRumble", 2152, 0 },
  { "meleeImpactRumble", 2156, 0 },
  { "adsDofStart", 2160, 6 },
  { "adsDofEnd", 2164, 6 }
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

char *__cdecl BG_GetPlayerAnimTypeName(int index)
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
    if ((unsigned int)type >= WEAPTYPE_NUM)
        MyAssertHandler(
            ".\\bgame\\bg_weapons_load_obj.cpp",
            851,
            0,
            "type doesn't index ARRAY_COUNT( szWeapTypeNames )\n\t%i not in [0, %i)",
            type,
            4);
    return szWeapTypeNames[type];
}

const char *__cdecl BG_GetWeaponClassName(weapClass_t type)
{
    if ((unsigned int)type >= WEAPCLASS_NUM)
        MyAssertHandler(
            ".\\bgame\\bg_weapons_load_obj.cpp",
            859,
            0,
            "type doesn't index ARRAY_COUNT( szWeapClassNames )\n\t%i not in [0, %i)",
            type,
            10);
    return szWeapClassNames[type];
}

const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type)
{
    if ((unsigned int)type >= WEAPINVENTORYCOUNT)
        MyAssertHandler(
            ".\\bgame\\bg_weapons_load_obj.cpp",
            867,
            0,
            "type doesn't index ARRAY_COUNT( szWeapInventoryTypeNames )\n\t%i not in [0, %i)",
            type,
            4);
    return szWeapInventoryTypeNames[type];
}

void __cdecl BG_LoadWeaponStrings()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_playerAnimTypeNamesCount; ++i)
        BG_InitWeaponString(i, g_playerAnimTypeNames[i]);
}

void __cdecl BG_LoadPlayerAnimTypes()
{
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
        if (!pField->iFieldType)
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

    if (!buffer)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 23, 0, "%s", "buffer");
    if (!fileName)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 24, 0, "%s", "fileName");
    if (!knots)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 25, 0, "%s", "knots");
    if (!knotCount)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 26, 0, "%s", "knotCount");
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
            Com_PrintWarning(15, "WARNING: \"%s\" has too many graph knots\n", fileName);
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
            Com_PrintError(15, "ERROR: \"%s\" Range must be 0.0 to 1.0\n", fileName);
            return 0;
        }
    }
    else
    {
        Com_PrintError(15, "ERROR: \"%s\" Error in parsing an ai weapon accuracy file\n", fileName);
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
    if (!weaponDef)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 88, 0, "%s", "weaponDef");
    if (!graphName)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 89, 0, "%s", "graphName");
    if (!knots)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 90, 0, "%s", "knots");
    if (!knotCount)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 91, 0, "%s", "knotCount");
    if (!dirName)
        MyAssertHandler(".\\game\\g_weapon_load_obj.cpp", 92, 0, "%s", "dirName");
    if (weaponDef->weapType && weaponDef->weapType != WEAPTYPE_PROJECTILE)
        return 1;
    if (!*graphName)
        return 1;
    sprintf(string, "accuracy/%s/%s", dirName, graphName);
    v6 = FS_FOpenFileByMode(string, &f, FS_READ);
    if (v6 >= 0)
    {
        FS_Read((unsigned __int8 *)buffer, len, f);
        buffer[len] = 0;
        if (!strncmp(buffer, last, len))
        {
            if (v6 - len < 0x2000)
            {
                memset((unsigned __int8 *)buffer, 0, 0x2000u);
                FS_Read((unsigned __int8 *)buffer, v6 - len, f);
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
                Com_PrintWarning(15, "WARNING: \"%s\" Is too long of an ai weapon accuracy file to parse\n", string);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(15, "WARNING: \"%s\" does not appear to be an ai weapon accuracy file\n", string);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(15, "WARNING: Could not load ai weapon accuracy file '%s'\n", string);
        return 0;
    }
}

char __cdecl G_ParseWeaponAccurayGraphs(WeaponDef *weaponDef)
{
    unsigned int size; // [esp+4h] [ebp-8Ch]
    int weaponType; // [esp+8h] [ebp-88h]
    int accuracyGraphKnotCount; // [esp+Ch] [ebp-84h] BYREF
    float accuracyGraphKnots[16][2]; // [esp+10h] [ebp-80h] BYREF

    for (weaponType = 0; weaponType < 2; ++weaponType)
    {
        memset((unsigned __int8 *)accuracyGraphKnots, 0, sizeof(accuracyGraphKnots));
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
            memcpy((unsigned __int8 *)weaponDef->accuracyGraphKnots[weaponType], (unsigned __int8 *)accuracyGraphKnots, size);
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
    if (useFastFile->current.enabled)
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

    if (!value)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 945, 0, "%s", "value");
    if (!stringArray)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 946, 0, "%s", "stringArray");
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

    if (!surfaceSoundBase)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 965, 0, "%s", "surfaceSoundBase");
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

int __cdecl BG_ParseWeaponDefSpecificFieldType(unsigned __int8 *pStruct, const char *pValue, int iFieldType)
{
    unsigned __int16 prev; // ax
    unsigned __int16 LowercaseString_DONE; // ax
    unsigned __int16 v5; // ax
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

    if (!pStruct)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 1023, 0, "%s", "pStruct");
    if (!pValue)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 1024, 0, "%s", "pValue");
    weapDef = (WeaponDef *)pStruct;
    switch (iFieldType)
    {
    case 12:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)szWeapTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon type %s in %s", pValue, weapDef->szInternalName);
        weapDef->weapType = (weapType_t)arrayIndex;
        goto LABEL_86;
    case 13:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)szWeapClassNames, 10);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon class %s in %s", pValue, weapDef->szInternalName);
        weapDef->weapClass = (weapClass_t)arrayIndex;
        goto LABEL_86;
    case 14:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapOverlayReticleNames, 2);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon reticle %s in %s", pValue, weapDef->szInternalName);
        weapDef->overlayReticle = (weapOverlayReticle_t)arrayIndex;
        goto LABEL_86;
    case 15:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)penetrateTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon penetrate type %s in %s", pValue, weapDef->szInternalName);
        weapDef->penetrateType = (PenetrateType)arrayIndex;
        goto LABEL_86;
    case 16:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)impactTypeNames, 9);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon impact type %s in %s", pValue, weapDef->szInternalName);
        weapDef->impactType = (ImpactType)arrayIndex;
        goto LABEL_86;
    case 17:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapStanceNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon stance %s in %s", pValue, weapDef->szInternalName);
        weapDef->stance = (weapStance_t)arrayIndex;
        goto LABEL_86;
    case 18:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szProjectileExplosionNames, 7);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon projExplosion %s in %s", pValue, weapDef->szInternalName);
        weapDef->projExplosion = (weapProjExposion_t)arrayIndex;
        goto LABEL_86;
    case 19:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)offhandClassNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon offhand class %s in %s", pValue, weapDef->szInternalName);
        weapDef->offhandClass = (OffhandClass)arrayIndex;
        goto LABEL_86;
    case 20:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, g_playerAnimTypeNames, g_playerAnimTypeNamesCount);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon player anim type %s in %s", pValue, weapDef->szInternalName);
        weapDef->playerAnimType = arrayIndex;
        goto LABEL_86;
    case 21:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)activeReticleNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon active reticle type %s in %s", pValue, weapDef->szInternalName);
        weapDef->activeReticleType = (activeReticleType_t)arrayIndex;
        goto LABEL_86;
    case 22:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)guidedMissileNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon guided missile type %s in %s", pValue, weapDef->szInternalName);
        weapDef->guidedMissileType = (guidedMissileType_t)arrayIndex;
        goto LABEL_86;
    case 23:
        weapDef->bounceSound = BG_RegisterSurfaceTypeSounds(pValue);
        goto LABEL_86;
    case 24:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)stickinessNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon stickiness %s in %s", pValue, weapDef->szInternalName);
        weapDef->stickiness = (WeapStickinessType)arrayIndex;
        goto LABEL_86;
    case 25:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)overlayInterfaceNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon overlay interface %s in %s", pValue, weapDef->szInternalName);
        weapDef->overlayInterface = (WeapOverlayInteface_t)arrayIndex;
        goto LABEL_86;
    case 26:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapInventoryTypeNames, 4);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon inventory type %s in %s", pValue, weapDef->szInternalName);
        weapDef->inventoryType = (weapInventoryType_t)arrayIndex;
        goto LABEL_86;
    case 27:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)szWeapFireTypeNames, 5);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon firetype %s in %s", pValue, weapDef->szInternalName);
        weapDef->fireType = (weapFireType_t)arrayIndex;
        goto LABEL_86;
    case 28:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char**)ammoCounterClipNames, 7);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon ammo counter clip %s in %s", pValue, weapDef->szInternalName);
        weapDef->ammoCounterClip = (ammoCounterClipType_t)arrayIndex;
        goto LABEL_86;
    case 29:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon hud icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->hudIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case 30:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon ammo counter icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->ammoCounterIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case 31:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon kill icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->killIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case 32:
        arrayIndex = Weapon_GetStringArrayIndex(pValue, (char **)weapIconRatioNames, 3);
        if (arrayIndex < 0)
            Com_Error(ERR_DROP, "Unknown weapon dpad icon ratio %s in %s", pValue, weapDef->szInternalName);
        weapDef->dpadIconRatio = (weaponIconRatioType_t)arrayIndex;
        goto LABEL_86;
    case 33:
        numHideTags = 0;
        pos = pValue;
        while (1)
        {
            token = (const char *)Com_Parse(&pos);
            if (!pos)
                break;
            if (numHideTags >= 8)
                Com_Error(ERR_DROP, "maximum hide tags (%s) exceeded: %i > %i'", token, numHideTags, 8);
            prev = SL_GetStringOfSize((char *)token, 0, strlen(token) + 1, 10).prev;
            weapDef->hideTags[numHideTags] = prev;
            weapDef->hideTags[numHideTags] = SL_ConvertToLowercase(weapDef->hideTags[numHideTags], 0, 10).prev;
            ++numHideTags;
        }
        goto LABEL_86;
    case 34:
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
                0,
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
    if (weapDef->fMaxDamageRange <= 0.0)
        weapDef->fMaxDamageRange = 999999.0;
    if (weapDef->fMinDamageRange <= 0.0)
        weapDef->fMinDamageRange = 999999.12;
}

void __cdecl BG_CheckProjectileValues(WeaponDef *weaponDef)
{
    if (weaponDef->weapType != WEAPTYPE_PROJECTILE)
        MyAssertHandler(".\\bgame\\bg_weapons_load_obj.cpp", 1277, 0, "%s", "weaponDef->weapType == WEAPTYPE_PROJECTILE");
    if ((double)weaponDef->iProjectileSpeed <= 0.0)
        Com_Error(ERR_DROP, "Projectile speed for WeapType %s must be greater than 0.0", weaponDef->szDisplayName);
    if (weaponDef->destabilizationCurvatureMax >= 1000000000.0 || weaponDef->destabilizationCurvatureMax < 0.0)
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
        FS_Read((unsigned __int8 *)buffer, len, f);
        buffer[len] = 0;
        if (!strncmp(buffer, "WEAPONFILE", len))
        {
            if ((unsigned int)(v11 - len) < 0x2800)
            {
                memset((unsigned __int8 *)buffer, 0, 0x2800u);
                FS_Read((unsigned __int8 *)buffer, v11 - len, f);
                buffer[v11 - len] = 0;
                FS_FCloseFile(f);
                if (Info_Validate(buffer))
                {
                    SetConfigString((char **)weapDef, two);
                    if (ParseConfigStringToStructCustomSize(
                        (unsigned __int8 *)weapDef,
                        weaponDefFields,
                        502,
                        buffer,
                        35,
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
                    Com_PrintWarning(17, "WARNING: \"%s\" is not a valid weapon file\n", dest);
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(
                    17,
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
            Com_PrintWarning(17, "WARNING: \"%s\" does not appear to be a weapon file\n", dest);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(17, "WARNING: Could not load weapon file '%s'\n", dest);
        return 0;
    }
}


WeaponDef *__cdecl BG_LoadWeaponDef_LoadObj(const char *name)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]
    WeaponDef *weapDefa; // [esp+0h] [ebp-4h]

    if (!*name)
        return 0;
    weapDef = BG_LoadWeaponDefInternal("mp", name);
    if (weapDef)
        return weapDef;
    weapDefa = BG_LoadWeaponDefInternal("mp", "defaultweapon_mp");
    if (!weapDefa)
        Com_Error(ERR_DROP, "BG_LoadWeaponDef: Could not find default weapon");
    SetConfigString((char **)weapDefa, name);
    return weapDefa;
}