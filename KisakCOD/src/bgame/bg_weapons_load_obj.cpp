#include "bg_local.h"
#include "bg_public.h"
#include <qcommon/mem_track.h>
#include <universal/q_parse.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>

//int surfaceTypeSoundListCount 828010f0     bg_weapons_load_obj.obj
//struct SurfaceTypeSoundList *surfaceTypeSoundLists 828011f8     bg_weapons_load_obj.obj

unsigned int g_playerAnimTypeNamesCount;

const char *szWeapInventoryTypeNames[4] = { "primary", "offhand", "item", "altmode" }; // idb
const char *penetrateTypeNames[4] = { "none", "small", "medium", "large" }; // idb
const char *szWeapOverlayReticleNames[2] = { "none", "crosshair" }; // idb
const char *szWeapStanceNames[3] = { "stand", "duck", "prone" }; // idb
const char *accuracyDirName[3] = { "aivsai", "aivsplayer", NULL }; // idb

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

const char *szWeapTypeNames[4] = { "bullet", "grenade", "projectile", "binoculars" }; // idb
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
        return (WeaponDef *)((int(__cdecl *)(WeaponDef * (__cdecl *)()))BG_LoadDefaultWeaponDef_FastFile)(BG_LoadDefaultWeaponDef_FastFile);
    else
        return (WeaponDef *)((int(__cdecl *)(WeaponDef * (__cdecl *)()))BG_LoadDefaultWeaponDef_LoadObj)(BG_LoadDefaultWeaponDef_LoadObj);
}

WeaponDef *__cdecl BG_LoadDefaultWeaponDef_FastFile()
{
    return DB_FindXAssetHeader(ASSET_TYPE_WEAPON, "none").weapon;
}

